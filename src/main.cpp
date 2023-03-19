#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include <iostream>
#include "graphics.h"

void graphics()
{
    Graphics graphics(800, 600);

    int init = 0;

    init = graphics.initialize();

    if (init != 0)
        std::cout << "Graphics initialization failed" << std::endl;

    graphics.update();

    graphics.release();
}

int main()
{
    graphics();

    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
    _CrtDumpMemoryLeaks();

    return 0;
}