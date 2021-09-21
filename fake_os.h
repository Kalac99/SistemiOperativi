#include "fake_process.h"
#include "linked_list.h"
#pragma once

int nuclei;

int scheduler;

typedef struct {
  ListItem list;
  int pid;
  ListHead events;
} FakePCB;

struct FakeOS;
typedef void (*ScheduleFn)(struct FakeOS* os, void* args);

//WORK IN PROGRESS
typedef struct FakeOS{
  ListHead running;
  ListHead ready;
  ListHead waiting;
  int timer;
  ScheduleFn schedule_fn;
  void* schedule_args;
  ListHead processes;
  //int maxsize;
} FakeOS;

void FakeOS_init(FakeOS* os);
void FakeOS_simStep(FakeOS* os);
void FakeOS_destroy(FakeOS* os);
