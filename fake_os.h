#include "fake_process.h"
#include "linked_list.h"
#pragma once

int nuclei;

int scheduler;

int quanto;

typedef struct {
  ListItem list;
  int pid;
  ListHead events;
  int prio; //campo priorità
  int temp_prio; // campo necessario per implementare l'aging
  int counter; // la priorità verrà aumentata (quindi numericamente decrementato il campio temp_prio di 1) ogni volta che il counter arriva a 20 circa
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
