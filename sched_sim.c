#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "fake_os.h"

FakeOS os;



typedef struct {
  int quantum;
} SchedRRArgs;

void schedRR(FakeOS* os, void* args_){
  SchedRRArgs* args=(SchedRRArgs*)args_;
  
  // look for the first process in ready
  // if none, return
  if (!os->ready.first)
    return;

  FakePCB* pcb=(FakePCB*) List_popFront(&os->ready);
  printf("\nPID DELL'ESTRATTO DALLA READY %d\n",pcb->pid);
  //Se c'è spazio nei core inserisco in running il pcb appena preso dai ready
  if (List_isFull(&os->running)==0){
    List_pushBack(&os->running,(ListItem*)pcb);  
  }
  else return;

  /*if(os->running1 == 0){os->running1=pcb;}
  else if(nuclei>=2 && os->running2 == 0){os->running2=pcb;}
  else if(nuclei>=3 && os->running3 == 0){os->running3=pcb;}
  else if(nuclei>=4 && os->running4 == 0){os->running4=pcb;}
  else if(nuclei>=5 && os->running5 == 0){os->running5=pcb;}
  else if(nuclei>=6 && os->running6 == 0){os->running6=pcb;}
  else return;*/
  
  assert(pcb->events.first);
  ProcessEvent* e = (ProcessEvent*)pcb->events.first;
  assert(e->type==CPU);

  // look at the first event
  // if duration>quantum
  // push front in the list of event a CPU event of duration quantum
  // alter the duration of the old event subtracting quantum
  if (e->duration>args->quantum) {
    ProcessEvent* qe=(ProcessEvent*)malloc(sizeof(ProcessEvent));
    qe->list.prev=qe->list.next=0;
    qe->type=CPU;
    qe->duration=args->quantum;
    e->duration-=args->quantum;
    List_pushFront(&pcb->events, (ListItem*)qe);
  }
  
  
};

int main(int argc, char** argv) {

  //Prendo il numero di parametri...massimo 4 per ora
/*
  while(nuclei<1 || nuclei >6){
    printf("Inserisci il numero di core voluti, minimo 1 massimo 6: ");
    scanf("%d",&nuclei);
    if(nuclei==0) printf("Sul serio? Che ci fai con una CPU inutile?\n");
  }*/
  

  FakeOS_init(&os);
  SchedRRArgs srr_args;
  srr_args.quantum=5;
  os.schedule_args=&srr_args;
  os.schedule_fn=schedRR;

  for (int i=1; i<argc; ++i){
    FakeProcess new_process;
    int num_events=FakeProcess_load(&new_process, argv[i]);
    printf("loading [%s], pid: %d, events:%d",
           argv[i], new_process.pid, num_events);
    if (num_events) {
      FakeProcess* new_process_ptr=(FakeProcess*)malloc(sizeof(FakeProcess));
      *new_process_ptr=new_process;
      List_pushBack(&os.processes, (ListItem*)new_process_ptr);
    }
  }
  printf("num processes in queue %d\n", os.processes.size);
  // qua forse sarà os.running.first, oppure un os.running1 .. os.runningN
  // OCCHIO qua c'era os.running
  //Nella mia prima versione è os.running.first
  while(os.running.first //os.running1 || os.running2 || os.running3 || os.running4 || os.running5 || os.running6
        || os.ready.first
        || os.waiting.first
        || os.processes.first){
    FakeOS_simStep(&os);
  }
  printf(" ALL PROCESSES ENDED THEIR WORK ..... BYE BYE\n");
}
