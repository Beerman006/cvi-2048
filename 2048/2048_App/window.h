#ifndef __window_H__
#define __window_H__

#ifdef __cplusplus
    extern "C" {
#endif

#include "cvidef.h"
#include "../2048/game.h"
        
UserInterface *WindowCreateUserInterface(Game2048 *game);

#ifdef __cplusplus
    }
#endif

#endif  /* ndef __window_H__ */
