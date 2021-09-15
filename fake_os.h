#include "fake_process.h"
#include "linked_list.h"
#pragma once

int nuclei;

typedef struct {
  ListItem list;
  int pid;
  ListHead events;
} FakePCB;

struct FakeOS;
typedef void (*ScheduleFn)(struct FakeOS* os, void* args);

//WORK IN PROGRESS
/*typedef struct FakeOS{
  ListHead running;
  ListHead ready;
  ListHead waiting;
  int timer;
  ScheduleFn schedule_fn;
  void* schedule_args;
  ListHead processes;
} FakeOS;
*/
typedef struct FakeOS{ 
  FakePCB* running1;
  FakePCB* running2;
  FakePCB* running3;
  FakePCB* running4;
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
