#ifdef __APPLE__
#import <AppKit/AppKit.h>
#include "Modules/Tracker/Platform/IPlatformTracker.h"

class MacTracker : public IPlatformTracker
{
public:
	WindowInfo GetActiveWindow() override
	{
		WindowInfo info;
		NSDictionary* activeApp = [[NSWorkspace sharedWorkspace]frontmostApplication];
		info.title = [[activeApp objectForKey : @"NSApplicationName"] UTF8String];
		info.executable = [[activeApp objectForKey : @"NSApplicationPath"]UTF8String];
		return info;
	}
};

#endif