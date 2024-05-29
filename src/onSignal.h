#pragma once
#define _POSIX_C_SOURCE 199309L
#include <signal.h>
#include <stddef.h>

static void onSignal(int signum, void (*handler)(int)) {
  struct sigaction act = {.sa_flags = 0, .sa_handler = handler};
  sigaction(signum, &act, NULL);
}
