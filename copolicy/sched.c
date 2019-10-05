#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "sched.h"

enum Policy{FIFO, PRIORITY, DEADLINE} policy;

struct Task {
	void (*entrypoint)(void *aspace);
	void *aspace;
	unsigned id;
	unsigned ord;
	unsigned priority;
	unsigned deadline;
	unsigned endlockTime;
	struct Task *nextTask;
};

struct Task *nextTask = NULL;
struct Task tasks[1000];
unsigned time = 0;
unsigned currentId = -1;
unsigned currentOrd = 0;
unsigned newId = 0;

int cmp(struct Task *a, struct Task *b) {
	switch(policy) {
		case FIFO:
			return a->ord <= b->ord;
		case PRIORITY:
			return (a->priority < b->priority) || ((a->priority == b->priority) && (a->ord <= b->ord));
		case DEADLINE:
			if (a->deadline == -1) {
				if (b->deadline == -1) {
					return 1;
				} else {
					return 0;
				}
			} 
			
			if (b->deadline == -1) {
				return 1;
			}

			if (a->deadline == b->deadline) {
				if (a->endlockTime == b->endlockTime) {
					return (a->priority < b->priority) || ((a->priority == b->priority) && (a->ord <= b->ord));
				}

				return a->endlockTime < b->endlockTime;
			}

			return a->deadline <= b->deadline;
	}
}

void queue(struct Task *newTask) { 
	if (nextTask == NULL || !cmp(nextTask, newTask)) {
		newTask->nextTask = nextTask;
		nextTask = newTask;
		return;
	}

	struct Task *parent = nextTask;

	while (parent->nextTask != NULL && cmp(parent->nextTask, newTask)) {
		parent = parent->nextTask;
	}
	
	newTask->nextTask = parent->nextTask;
	parent->nextTask = newTask;
}

void sched_new(void (*entrypoint)(void *aspace),
		void *aspace,
	       	int priority,
		int deadline) {
	unsigned id = newId++;
	tasks[id].ord = currentOrd++;
	tasks[id].entrypoint = entrypoint;
	tasks[id].id = id;
	tasks[id].aspace = aspace;
	tasks[id].priority = priority;
	tasks[id].deadline = deadline;
	tasks[id].endlockTime = 0;

	queue(&tasks[id]);
}

void sched_cont(void (*entrypoint)(void *aspace),
		void *aspace,
		int timeout) {
	if (currentId == -1) {
		printf("Illegal sched_cont!");
		return;
	}
	else {
		tasks[currentId].endlockTime = time + timeout;
		tasks[currentId].ord = currentOrd++;
		queue(&tasks[currentId]);
		currentId = -1;
	}
}

void sched_time_elapsed(unsigned amount) {
	time += amount;
}

void sched_set_policy(const char *name) {
	if (!strcmp(name, "fifo"))
		policy = FIFO;
	else if (!strcmp(name, "priority"))
		policy = PRIORITY;
	else if (!strcmp(name, "deadline"))
		policy = DEADLINE;
	else {
		printf("Wrong policy\n");
		exit(1);
	}
}

void sched_run(void) {
	while(nextTask != NULL) {
		if (nextTask->endlockTime < time) {
			struct Task *toExecute = nextTask;
			nextTask = nextTask->nextTask;
			currentId = toExecute->id;
			void (*run_app)(void *) = toExecute->entrypoint;
        		run_app(toExecute->aspace);
		} else {
			sched_time_elapsed(1);
		}
	}
}
