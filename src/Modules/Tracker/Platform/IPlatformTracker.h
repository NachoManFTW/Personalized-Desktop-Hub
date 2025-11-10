#pragma once
#include <string>

struct WindowInfo
{
	std::string title;
	std::string executable;
};

class IPlatformTracker
{
public:
	virtual ~IPlatformTracker() = default;
	virtual WindowInfo GetActiveWindow() = 0;
};
