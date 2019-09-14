#define _GNU_SOURCE

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include <ucontext.h>
#include <sys/ucontext.h>

#include "init.h"

const int baseValue = 100000;
int baseFrame = 0;

void os_sigHandle(int sig, siginfo_t *info, void *ctx) {
	ucontext_t *uc = (ucontext_t *) ctx;
	static int callNumber = 0;
	int result = baseValue + (++callNumber);
	int index = 0;
	int stepRIP = 2;

	uint16_t cmd = *(uint16_t *) uc->uc_mcontext.gregs[REG_RIP];

	if (cmd == 0x0b8b || cmd == 0x138b) {
	} else if (cmd == 0x538b) {
		index = ((*(uint32_t *) uc->uc_mcontext.gregs[REG_RIP]) & 0x00ff0000) >> 18;
		stepRIP = 3;
	} else if (cmd == 0x4d8b || cmd == 0x558b) {
		index = (((*(uint32_t *) uc->uc_mcontext.gregs[REG_RIP]) & 0x00ff0000) >> 18) \
			+ (((int)uc->uc_mcontext.gregs[REG_RBP] - baseFrame) >> 2);
		stepRIP = 3;
	} 
	
	if (cmd == 0x0b8b || cmd == 0x54d8b) {
		uc->uc_mcontext.gregs[REG_RCX] = result + 1000 * index;
		uc->uc_mcontext.gregs[REG_RIP] += stepRIP;
	} else if (cmd == 0x138b || cmd == 0x538b || cmd == 0x558b) {
		uc->uc_mcontext.gregs[REG_RDX] = result + 1000 * index;
		uc->uc_mcontext.gregs[REG_RIP] += stepRIP;
	}
}

void init(void *base) {
	struct sigaction act = {
		.sa_sigaction = os_sigHandle,
		.sa_flags = SA_RESTART,
	};
	sigemptyset(&act.sa_mask);
	baseFrame = base;

	sigaction(SIGSEGV, &act, NULL);
}
