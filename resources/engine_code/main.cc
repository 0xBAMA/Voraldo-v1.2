/*
 * =====================================================================================
 *
 *       Filename:  main.cc
 *
 *    Description:  instatiates an engine object, which falls through and quits on exit
 *
 *        Version:  1.0
 *        Created:  07/12/2020 11:04:36 AM
 *       Compiler:  gcc
 *
 *         Author:  Jon Baker 
 *
 * =====================================================================================
 */


#include "engine.h"

int main(int argc, char **argv)
{
    SDL_SetMainReady();

    engine e;

    if(!e.init())
        return 1;

    while(e.run())      // main loop
        e.main_loop();

    return 0;
}
