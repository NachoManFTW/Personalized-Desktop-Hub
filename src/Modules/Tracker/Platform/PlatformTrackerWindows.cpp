#ifdef _Win32
#include <Windows.h>
#include <Psapi.h>
#include <Modules/Tracker/Platform/IPlatformTracker.h>

WindowInfo GetWindowDetails(HWND hwnd)
{
	WindowInfo info;
	char title[256];
	GetWindowTextA(hwnd, title, sizeof(title));
	info.title = title;
	
	DWORD pid;
	GetWindowThreadProcessId(hwnd, &pid);
	HANDLE handle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);

	char path[MAX_PATH];
	DWORD size = MAX_PATH;
	if (QueryFullProcessImageNameA(handle, 0, path, &size))
		info.executable = path;
	else
		info.executable = "Unknown";

	CloseHandle(handle);
	return info;
}

class WindowsTracker : public IPlatformTracker
{
public:
	WindowInfo GetActiveWindow() override
	{
		HWND hwnd = GetForegroundWindow();
		if (!hwnd) return{};
		return GetWindowDetails(hwnd);
	}
};
#endif