#include "Core/Application.h"
#include "Modules/Tracker/TrackerModule.h"
#include "Modules/Console/ConsoleModule.h"
#include "Modules/FileExplorer/FileExplorerModule.h"
#include <memory>
#include <iostream>


int main() 
{
    Application app(1280, 720, "DesktopHub");

    app.AddModule<TrackerModule>();
    app.AddModule<FileExplorerModule>();
    app.AddModule<ConsoleModule>();
    app.Run();
    return 0;

}
