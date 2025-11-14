#include "VirtualMemoryMapModule.h"

#include <sstream>
#include <iostream>
#include <chrono>

//-------------------------
// Lifecycle
//-------------------------
void VirtualMemoryMapModule::Init()
{
	m_LastRefreshTime = ImGui::GetTime();
	BuildMemoryMap();
}

void VirtualMemoryMapModule::Update(float dt)
{
	double now = ImGui::GetTime();
	if ((now - m_LastRefreshTime) >= m_AutoRefreshSeconds)
	{
		BuildMemoryMap();
		m_LastRefreshTime = now;
	}
}

void VirtualMemoryMapModule::Render()
{
	ImGui::Begin("Memory Map", nullptr, m_Flags);

	if (ImGui::Button("Refresh")) {
		BuildMemoryMap();
		m_LastRefreshTime = ImGui::GetTime();
	}
	ImGui::SameLine();
	ImGui::Text("Regions: %zu", m_MemRegions.size());
	ImGui::Separator();

	//Table with scroll if many rows
	if (ImGui::BeginTable("##memtable", 5,
		ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_ScrollX))
	{
		ImGui::TableSetupColumn("Base", ImGuiTableColumnFlags_WidthFixed, 160.0f);
		ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, 100.0f);
		ImGui::TableSetupColumn("State", ImGuiTableColumnFlags_WidthStretch, 100.0f);
		ImGui::TableSetupColumn("Protect", ImGuiTableColumnFlags_WidthStretch, 100.0f);
		ImGui::TableSetupColumn("Sample (hex)", ImGuiTableColumnFlags_WidthStretch, 200.0f);
		ImGui::TableHeadersRow();

		//Iterate cached vector
		for (const auto& region : m_MemRegions)
		{
			ImGui::TableNextRow();

			ImVec4 rowColor;
			switch (region.ms_State)
			{
			case MEM_COMMIT:	rowColor = ImVec4(0.3f, 1.0f, 0.3f, 0.3f); break; //Green
			case MEM_RESERVE:	rowColor = ImVec4(1.0f, 1.0f, 0.3f, 0.3f); break; //Yellow
			case MEM_FREE:;		rowColor = ImVec4(0.6f, 0.6f, 0.6f, 0.3f); break; //Gray
			default:;			rowColor = ImVec4(1.0f, 0.0f, 0.0f, 0.3f); break; //Red for unknown
			}

			ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(rowColor));

			ImGui::TableSetColumnIndex(0);
			ImGui::Text("%p", region.ms_Base);

			ImGui::TableSetColumnIndex(1);
			ImGui::TextUnformatted(FormatSize(region.ms_Size).c_str());

			ImGui::TableSetColumnIndex(2);
			ImGui::TextUnformatted(MemStateToString(region.ms_State));

			ImGui::TableSetColumnIndex(3);
			ImGui::TextUnformatted(ProtectToString(region.ms_Protect).c_str());

			ImGui::TableSetColumnIndex(4);
			ImGui::TextUnformatted(region.ms_SampleHex.c_str());
		}

		ImGui::EndTable();
	}

	ImGui::End();
}

void VirtualMemoryMapModule::Shutdown()
{
	m_MemRegions.clear();
}


//-------------------------
// Helpers
//-------------------------
void VirtualMemoryMapModule::BuildMemoryMap()
{
	m_MemRegions.clear();

	SYSTEM_INFO systemInfo;
	::GetSystemInfo(&systemInfo);

	unsigned char* addr = reinterpret_cast<unsigned char*>(systemInfo.lpMinimumApplicationAddress);
	unsigned char* maxAddr = reinterpret_cast<unsigned char*>(systemInfo.lpMaximumApplicationAddress);

	while (addr < maxAddr)
	{
		MEMORY_BASIC_INFORMATION memBasicInfo;
		SIZE_T ret = VirtualQuery(static_cast<LPCVOID>(addr), &memBasicInfo, sizeof(memBasicInfo));
		if (ret == 0) 
			break;

		ms_MemRegion reg;
		reg.ms_Base = memBasicInfo.BaseAddress;
		reg.ms_Size = memBasicInfo.RegionSize;
		reg.ms_State = memBasicInfo.State;
		reg.ms_Protect = memBasicInfo.Protect;

		//Sample only committed readable pages and skip guard/noaccess
		bool canRead = (memBasicInfo.State == MEM_COMMIT) &&
						!(memBasicInfo.Protect & PAGE_NOACCESS) &&
						!(memBasicInfo.Protect & PAGE_GUARD);

		if (canRead)
		{
			reg.ms_SampleHex = SampleBytesAsHex(memBasicInfo.BaseAddress, memBasicInfo.RegionSize, (SIZE_T)m_MaxSampleBytes);
		}
		else
		{
			reg.ms_SampleHex.clear();
		}

		m_MemRegions.push_back(std::move(reg));

		//Advance
		addr += memBasicInfo.RegionSize;
	}
}

std::string VirtualMemoryMapModule::SampleBytesAsHex(void* addr, SIZE_T regionSize, SIZE_T maxSample)
{
	if (addr == nullptr || regionSize == 0 || maxSample == 0) return std::string();

	SIZE_T toRead = (regionSize < maxSample) ? regionSize : maxSample;
	unsigned char* p = reinterpret_cast<unsigned char*>(addr);

	std::ostringstream ss;
	ss << std::hex << std::setfill('0');

	//Read byte-by-byte — be prepared for exceptions (but reading own process generally safe)
	for (SIZE_T i = 0; i < toRead; ++i)
	{
		unsigned int byteVal = static_cast<unsigned int>(p[i]);
		ss << std::setw(2) << (byteVal & 0xFF);
		if (i + 1 < toRead) ss << " ";
	}
	return ss.str();
}

std::string VirtualMemoryMapModule::ProtectToString(DWORD p)
{
	if (p == 0) return "None";
	std::string s;
	if (p & PAGE_EXECUTE)          s += "EXEC ";
	if (p & PAGE_EXECUTE_READ)     s += "R/EXEC ";
	if (p & PAGE_EXECUTE_READWRITE)s += "RW/EXEC ";
	if (p & PAGE_EXECUTE_WRITECOPY)s += "WC/EXEC ";
	if (p & PAGE_READONLY)         s += "R ";
	if (p & PAGE_READWRITE)        s += "RW ";
	if (p & PAGE_WRITECOPY)        s += "WC ";
	if (p & PAGE_NOACCESS)         s += "NO ";
	if (p & PAGE_GUARD)            s += "GUARD ";
	if (p & PAGE_NOCACHE)          s += "NOCACHE ";
	if (s.empty()) s = "Other";
	return s;
}

const char* VirtualMemoryMapModule::MemStateToString(DWORD state)
{
	switch (state)
	{
	case MEM_COMMIT:  return "Committed";
	case MEM_FREE:    return "Free";
	case MEM_RESERVE: return "Reserved";
	default:          return "Unknown";
	}
}

std::string VirtualMemoryMapModule::FormatSize(SIZE_T bytes)
{
	const char* units[] = { "B", "KB", "MB", "GB" };
	double v = static_cast<double>(bytes);
	int u = 0;
	while (v > 1024.0 && u < 3) 
	{ v /= 1024.0; ++u; }
	char buf[64];
	snprintf(buf, sizeof(buf), "%.2f %s", v, units[u]);
	return std::string(buf);
}
