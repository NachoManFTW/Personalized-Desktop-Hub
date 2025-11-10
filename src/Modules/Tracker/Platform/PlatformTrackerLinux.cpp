#ifdef __linux__
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <unistd.h>
#include <cstring>
#include "Modules/Tracker/Platform/IPlatformTracker.h"

class LinuxTracker : public IPlatformTracker
{
public:
	WindowInfo GetActiveWindow() override
	{
		WindowInfo info;
		Display* display = XOpenDisplay(nullptr);
		if (!display) return info;

		Atom activeWindowAtom = XInternAtom(display, "_NET_ACTIVE_WINDOW", True);
		Atom actualType;
		int format;
		unsigned long nItems, bytesAfter;
		unsigned char* prop = nullptr;

		XGetWindowProperty(display, DefaulRootWindow(display), activeWindowAtom, 0, ~0L, False, AnyPropertyType, &actualType, &format, &nItems, &bytesAfter, &prop);
	
		if (prop)
		{
			Window active = *(Window*)prop;
			XFree(prop);

			char* name = nullptr;
			XFetchName(display, active, &name);
			if (name)
			{
				info.title = name;
				XFree(name);
			}
		}
		
		XCloseDisplay(display);
		return info;
	}
};
#endif