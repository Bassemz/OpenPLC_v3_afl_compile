// Minimal OpenPLC DNP3/TCP outstation entrypoint for fuzzing.
//
// Goal: initialize the generated PLC program + I/O tables, then run only the
// DNP3 server loop (no interactive server, no hardware init, no realtime).
//
// This is intentionally small so it can be used as an AFLNet target.

#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "ladder.h"

// -----------------------------------------------------------------------------
// Globals expected across the core (normally defined in `main.cpp`)
// -----------------------------------------------------------------------------
IEC_BOOL __DEBUG;
unsigned long __tick = 0;
pthread_mutex_t bufferLock;
uint8_t run_openplc = 1;

static void on_signal(int /*signum*/)
{
    run_dnp3 = 0;
    run_openplc = 0;
}

int main(int argc, char **argv)
{
    uint16_t port = 20000;
    if (argc >= 2)
    {
        long parsed = strtol(argv[1], nullptr, 10);
        if (parsed > 0 && parsed < 65536)
        {
            port = static_cast<uint16_t>(parsed);
        }
        else
        {
            fprintf(stderr, "Invalid port '%s' (expected 1-65535)\n", argv[1]);
            return 2;
        }
    }

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

    run_dnp3 = 1;
    dnp3StartServer(port);

    run_dnp3 = 0;
    run_openplc = 0;
    return 0;
}
