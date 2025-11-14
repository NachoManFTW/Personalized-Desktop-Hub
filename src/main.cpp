#include "Core/Application.h"

//Modules
#include "Modules/Tracker/TrackerModule.h"
#include "Modules/Console/ConsoleModule.h"
#include "Modules/FileExplorer/FileExplorerModule.h"
#include "Modules/VirtualMemoryMap/VirtualMemoryMapModule.h"

int main() 
{
    Application app(1280, 720, "DesktopHub");

    app.AddModule<TrackerModule>();
    app.AddModule<FileExplorerModule>();
    app.AddModule<ConsoleModule>();
    app.AddModule<VirtualMemoryMapModule>();

    app.Run();
    app.Shutdown();
}
