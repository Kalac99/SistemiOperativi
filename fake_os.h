#include "fake_process.h"
#include "linked_list.h"
#pragma once


typedef struct {
  ListItem list;
  int pid;
  ListHead events;
} FakePCB;

struct FakeOS;
typedef void (*ScheduleFn)(struct FakeOS* os, void* args);

typedef struct FakeOS{ //io aggiungerei na bella linked list di fakepcb con size il numero di core oppure  runningI con i = 1..N numero di core 
  FakePCB* running;
  ListHead ready;
  ListHead waiting;
  int timer;
  ScheduleFn schedule_fn;
  void* schedule_args;

  ListHead processes;
} FakeOS;

void FakeOS_init(FakeOS* os);
void FakeOS_simStep(FakeOS* os);
void FakeOS_destroy(FakeOS* os);
