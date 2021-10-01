#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "fake_os.h"


void FakeOS_init(FakeOS* os) {
  List_init(&os->running);
  os->running.maxsize = nuclei; //SETTO IL NUMERO DI CORE <- tramite variabile globale che prendo da scanf
  List_init(&os->ready);
  List_init(&os->waiting);
  List_init(&os->processes);
  os->timer=0;
  os->schedule_fn=0;
}

void FakeOS_createProcess(FakeOS* os, FakeProcess* p) {
  // sanity check
  assert(p->arrival_time==os->timer && "time mismatch in creation");
  // we check that in the list of PCBs there is no
  // pcb having the same pid
  //controllo che il pid non sia lo stesso di uno dei processi in esecuzione
  ListItem* aux;
  aux=os->running.first;
  while(aux){
    FakePCB* pcb=(FakePCB*)aux;
    assert(pcb->pid!=p->pid && "pid taken");
    aux=aux->next;
  }
  
  //controlla che il pid non sia nella ready e nella wait
  aux=os->ready.first;
  while(aux){
    FakePCB* pcb=(FakePCB*)aux;
    assert(pcb->pid!=p->pid && "pid taken");
    aux=aux->next;
  }

  aux=os->waiting.first;
  while(aux){
    FakePCB* pcb=(FakePCB*)aux;
    assert(pcb->pid!=p->pid && "pid taken");
    aux=aux->next;
  }

  // all fine, no such pcb exists
  FakePCB* new_pcb=(FakePCB*) malloc(sizeof(FakePCB));
  new_pcb->list.next=new_pcb->list.prev=0;
  new_pcb->pid=p->pid;
  new_pcb->prio = p->prio;
  new_pcb->temp_prio = p->temp_prio;
  new_pcb->events=p->events;

  assert(new_pcb->events.first && "process without events");

  // depending on the type of the first event
  // we put the process either in ready or in waiting
  
  ProcessEvent* e=(ProcessEvent*)new_pcb->events.first;
  switch(e->type){
  case CPU:
    List_pushBack(&os->ready, (ListItem*) new_pcb);
    break;
  case IO:
    List_pushBack(&os->waiting, (ListItem*) new_pcb);
    break;
  default:
    assert(0 && "illegal resource");
    ;
  }
}




void FakeOS_simStep(FakeOS* os){
  
  printf("************** TIME: %08d **************\n", os->timer);

  //scan process waiting to be started
  //and create all processes starting now
  ListItem* aux=os->processes.first;
  while (aux){
    FakeProcess* proc=(FakeProcess*)aux;
    FakeProcess* new_process=0;
    if (proc->arrival_time==os->timer){
      new_process=proc;
    }
    aux=aux->next;
    if (new_process) {
      printf("\tcreate pid:%d\n", new_process->pid);
      new_process=(FakeProcess*)List_detach(&os->processes, (ListItem*)new_process);
      FakeOS_createProcess(os, new_process);
      free(new_process);
    }
  }

  

  // scan waiting list, and put in ready all items whose event terminates
  aux=os->waiting.first;
  while(aux) {
    FakePCB* pcb=(FakePCB*)aux;
    aux=aux->next;
    ProcessEvent* e=(ProcessEvent*) pcb->events.first;
    printf("\twaiting pid: %d\n", pcb->pid);
    assert(e->type==IO);
    e->duration--;
    printf("\t\tremaining time:%d\n",e->duration);
    if (e->duration==0){
      printf("\t\tend burst\n");
      List_popFront(&pcb->events);
      free(e);
      List_detach(&os->waiting, (ListItem*)pcb);
      if (! pcb->events.first) {
        // kill process
        printf("\t\tend process\n");
        free(pcb);
      } else {
        //handle next event
        e=(ProcessEvent*) pcb->events.first;
        switch (e->type){
        case CPU:
          printf("\t\tmove to ready\n"); 
          List_pushBack(&os->ready, (ListItem*) pcb);
          break;
        case IO:
          printf("\t\tmove to waiting\n");
          List_pushBack(&os->waiting, (ListItem*) pcb);
          break;
        }
      }
    }
    
  }

  

  // decrement the duration of running
  // if event over, destroy event
  // and reschedule process
  // if last event, destroy running

  
  aux=os->running.first;
  
  int  i =  1;
  while(aux){
    FakePCB* running = (FakePCB*) aux;
    int num;
    aux=aux->next;
    printf("\trunning pid on core %d: %d\n",i,running?running->pid:-1);
    i++;
    if (running) {
      ProcessEvent* e=(ProcessEvent*) running->events.first;
      assert(e->type==CPU);
      e->duration--;
      printf("\t\tremaining time:%d\n",e->duration);
      if (e->duration==0){
        running->counter = 1; //anche il contatore viene resettato a 1 ogni volta che un processo riesce a completare un burst
        printf("\t\tend burst\n");
        List_popFront(&running->events);
        free(e);
        List_detach(&os->running, (ListItem*)running);
        if (! running->events.first) {
          printf("\t\tend process\n");
          free(running); // kill process
        } else {
          e=(ProcessEvent*) running->events.first;
          switch (e->type){
          case CPU:
            printf("\t\tmove to ready\n");
            List_pushBack(&os->ready, (ListItem*)running);
            running->temp_prio = running->prio; //QUANDO TERMINA IL CPU BURST LA PRIORITÀ DEL PROCESSO VIENE RESETTATA A QUELLA INIZIALE
            break;
          case IO:
            printf("\t\tmove to waiting\n");
            List_pushBack(&os->waiting, (ListItem*) running);
            break;
          }
        }
      }
      //controllo se nella ready ci sono processi con durata minore, in tal caso preemption
      else if(scheduler==2){
        ListItem* ausilio = os->ready.first;
        ProcessEvent* e = (ProcessEvent*) running->events.first;
        int min = e->duration;
        int  concurrent;
        FakePCB* pcb;
        while(ausilio){
          pcb = (FakePCB*) ausilio;
          ProcessEvent* evento = (ProcessEvent*) pcb->events.first;
          concurrent = evento->duration;
          if(concurrent<min) {
            //PREEMPTION!!
            printf("\t\tprocess with shorter duration was found\n");
            printf("\t\tpreempting, move to ready\n");
            List_detach(&os->running, (ListItem*)running);
            List_pushBack(&os->ready, (ListItem*)running);
            break;
          }
          else ausilio = ausilio->next; 
        }
      }
      //controllo se nella ready ci sono processi con priorità maggiore, in tal caso preemption
      else if(scheduler==3){
        ListItem* ausilio = os->ready.first;
        int min = running->temp_prio;
        FakePCB* pcb;
        num = 0;
        int  concurrent;
        while(ausilio){
          pcb = (FakePCB*) ausilio;
          concurrent = pcb->temp_prio;
          if (pcb->counter==20*nuclei) {
            if(pcb->temp_prio>1) {
              pcb->temp_prio--;
              printf("\t\tprocess %d waited for %d time slots, his priority is now increased\n",pcb->pid,quanto-1);
              pcb->counter=1;
            }
          }
          else pcb->counter++;
          if(concurrent<min) {
            num+=1;
          }
          ausilio = ausilio->next; 
        }
        //printf("\n numero processi con maggior prio: %d\n",num);
        if (num>0){
          //PREEMPTION!!
          printf("\t\t%d processes with higher priority were found\n",num);
          printf("\t\tpreempting, move to ready\n");
          List_detach(&os->running, (ListItem*)running);
          List_pushBack(&os->ready, (ListItem*)running);
        }
      }
    }
    
  }

  // call schedule, if defined
  // Controllo che ci sia almeno un core libero verificando che la lista dei running sia piena, se piena allora tutti i core sono occupati -> SKIP
  while(os->schedule_fn && (List_isFull(&os->running)==0) && os->ready.first){    
    (*os->schedule_fn)(os, os->schedule_args); 
  }


  // if running not defined and ready queue not empty
  // put the first in ready to run

  
  while (os->ready.first && (List_isFull(&os->running)==0)) {
    List_pushBack(&os->running, List_popFront(&os->ready));
  } 

  ++os->timer;
}


void FakeOS_destroy(FakeOS* os) {
}
