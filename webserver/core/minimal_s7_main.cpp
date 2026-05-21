// Minimal OpenPLC Siemens S7 (Snap7) server entrypoint for fuzzing.
//
// Goal: initialize the generated PLC program + I/O tables, then run only the
// Snap7 server (no interactive server, no hardware init, no realtime).
//
// The S7 protocol listens on TCP port 102 by default (ISO-on-TCP).

#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "ladder.h"
#include "oplc_snap7.h"

// -----------------------------------------------------------------------------
// Globals expected across the core (normally defined in `main.cpp`)
// -----------------------------------------------------------------------------
IEC_BOOL __DEBUG;
unsigned long __tick = 0;
pthread_mutex_t bufferLock;
uint8_t run_openplc = 1;

static void on_signal(int /*signum*/)
{
    run_openplc = 0;
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    signal(SIGINT, on_signal);
    signal(SIGTERM, on_signal);

    if (pthread_mutex_init(&bufferLock, NULL) != 0)
    {
        fprintf(stderr, "Mutex init failed\n");
        return 1;
    }

    config_init__();
#ifdef __AFL_HAVE_MANUAL_CONTROL
    __AFL_INIT();
#endif
    glueVars();

    mapUnusedIO();

    initializeSnap7();
    startSnap7();

    while (run_openplc)
    {
        sleepms(100);
    }

    stopSnap7();
    finalizeSnap7();

    return 0;
}
