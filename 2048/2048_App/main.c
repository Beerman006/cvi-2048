#include <userint.h>
#include <cvirte.h>
#include "../2048/game.h"
#include "window.h"

int main (int argc, char *argv[])
{
    if (InitCVIRTE (0, argv, 0) == 0)
        return -1;    /* out of memory */
    
    Game2048 *game = Game2048Create(4, 4, WindowCreateUserInterface);
    Game2048Run(game);
    Game2048Dispose(game);
    return 0;
}
