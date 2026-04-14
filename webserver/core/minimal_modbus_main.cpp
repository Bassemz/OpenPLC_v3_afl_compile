// Minimal OpenPLC Modbus/TCP server entrypoint for fuzzing.
//
// Goal: initialize the generated PLC program + I/O tables, then run only the
// Modbus server loop (no interactive server, no hardware init, no realtime).
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
    run_modbus = 0;
    run_openplc = 0;
}

int main(int argc, char **argv)
{
    uint16_t port = 502;
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

    // Initialize the generated PLC program and connect the variable pointers.
    config_init__();
        #ifdef __AFL_HAVE_MANUAL_CONTROL
  __AFL_INIT();
#endif
    glueVars();

    // Ensure all unmapped I/O points still point to valid storage, so Modbus
    // accesses never hit NULL pointers.
    mapUnusedIO();

    // Start only the Modbus/TCP server.
    run_modbus = 1;
    startServer(port, MODBUS_PROTOCOL);



    run_modbus = 0;
    run_openplc = 0;
    return 0;
}
