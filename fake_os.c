#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "fake_os.h"

//WORK IN PROGRESS
void FakeOS_init(FakeOS* os) {
  List_init(&os->running);
  os->running.maxsize = 4; //SETTO IL NUMERO DI CORE, DA RIVEDERE COME PASSARGLI QUESTO VALORE -< tramite variabile globale che prendo da scanf
  List_init(&os->ready);
  List_init(&os->waiting);
  List_init(&os->processes);
  os->timer=0;
  os->schedule_fn=0;
}

/*void FakeOS_init(FakeOS* os) {
  os->running1=0;
  os->running2=0;
  os->running3=0;
  os->running4=0;
  os->running5=0;
  os->running6=0;
  List_init(&os->ready);
  List_init(&os->waiting);
  List_init(&os->processes);
  os->timer=0;
  os->schedule_fn=0;
}*/

void FakeOS_createProcess(FakeOS* os, FakeProcess* p) {
  // sanity check
  assert(p->arrival_time==os->timer && "time mismatch in creation");
  // we check that in the list of PCBs there is no
  // pcb having the same pid
  // se implemento la linked list mi conviene fare l'aux per vedere i pcb di ogni core
  ListItem* aux;
  //WORK IN PROGRESS
  aux=os->running.first;
  while(aux){
    FakePCB* pcb=(FakePCB*)aux;
    assert(pcb->pid!=p->pid && "pid taken");
    aux=aux->next;
  }

  /*assert( (!os->running1 || os->running1->pid!=p->pid) && "pid taken");
  if (nuclei>=2) {assert( (!os->running2 || os->running2->pid!=p->pid) && "pid taken");
  if (nuclei>=3) {assert( (!os->running3 || os->running3->pid!=p->pid) && "pid taken");
  if (nuclei>=4) {assert( (!os->running4 || os->running4->pid!=p->pid) && "pid taken");
  if (nuclei>=5) {assert( (!os->running5 || os->running5->pid!=p->pid) && "pid taken");
  if (nuclei>=5) {assert( (!os->running6 || os->running6->pid!=p->pid) && "pid taken");}}}}}*/
  
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
  new_pcb->events=p->events;

  assert(new_pcb->events.first && "process without events");

  // depending on the type of the first event
  // we put the process either in ready or in waiting
  //dovrebbe rimanere invariata nel multicore
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

  //qua probabilmente aux=os->running.first e poi il while, altrimenti un blocco tipo il seguente per ogni core
  
  
  aux=os->running.first;
  //-------------------------------PROBLEMA PRINCIPALE!-------------------------------------------------------------------
  int  i =  0;
  int dimensione = os->running.size;
  while(i<dimensione){
    FakePCB* running = (FakePCB*) aux;
    printf("\n PID del processo preso dal running: %d \n ", running->pid);
    aux=aux->next;
    printf("\trunning pid on core %d: %d\n", i+1,running?running->pid:-1);
    i++;
    if (running) {
      ProcessEvent* e=(ProcessEvent*) running->events.first;
      assert(e->type==CPU);
      e->duration--;
      printf("\t\tremaining time:%d\n",e->duration);
      if (e->duration==0){
        printf("\t\tend burst\n");
        List_popFront(&running->events);
        free(e);
        if (! running->events.first) {
          printf("\t\tend process\n");
          free(running); // kill process
        } else {
          e=(ProcessEvent*) running->events.first;
          switch (e->type){
            //PROBLEMA----VIENE PUSHATO SOLO L'ULTIMO PROCESSO NELLA READY ipotizzo avvenga lo stesso nella waiting - GLI ALTRI PROCESSI SONO SPARITI 
            // quindi penso che in qualche modo ogni push vada a "sovrascrivere il precedente"
          case CPU:
            printf("\t\tmove to ready\n");
            List_pushBack(&os->ready, (ListItem*) running);
            break;
          case IO:
            printf("\t\tmove to waiting\n");
            List_pushBack(&os->waiting, (ListItem*) running);
            break;
          }
        }
        //os->running = 0; //CREATA ALT
        //questa detach dovrebbe eliminare il pcb del processo che ha terminato la sua durata (burst o quanto?)
        List_detach(&os->running,(ListItem*)running);
      }
      
    }
  }
  

   /* 
    FakePCB* running=os->running1;  
    printf("\trunning pid on core 1: %d\n", running?running->pid:-1);
    if (running) {
      ProcessEvent* e=(ProcessEvent*) running->events.first;
      assert(e->type==CPU);
      e->duration--;
      printf("\t\tremaining time:%d\n",e->duration);
      if (e->duration==0){
        printf("\t\tend burst\n");
        List_popFront(&running->events);
        free(e);
        if (! running->events.first) {
          printf("\t\tend process\n");
          free(running); // kill process
        } else {
          e=(ProcessEvent*) running->events.first;
          switch (e->type){
          case CPU:
            printf("\t\tmove to ready\n");
            List_pushBack(&os->ready, (ListItem*) running);
            break;
          case IO:
            printf("\t\tmove to waiting\n");
            List_pushBack(&os->waiting, (ListItem*) running);
            break;
          }
        }
        os->running1 = 0;  
      }
    }

  if(nuclei>=2){
    running=os->running2;  
    printf("\trunning pid on core 2: %d\n", running?running->pid:-1);
    if (running) {
      ProcessEvent* e=(ProcessEvent*) running->events.first;
      assert(e->type==CPU);
      e->duration--;
      printf("\t\tremaining time:%d\n",e->duration);
      if (e->duration==0){
        printf("\t\tend burst\n");
        List_popFront(&running->events);
        free(e);
        if (! running->events.first) {
          printf("\t\tend process\n");
          free(running); // kill process
        } else {
          e=(ProcessEvent*) running->events.first;
          switch (e->type){
          case CPU:
            printf("\t\tmove to ready\n");
            List_pushBack(&os->ready, (ListItem*) running);
            break;
          case IO:
            printf("\t\tmove to waiting\n");
            List_pushBack(&os->waiting, (ListItem*) running);
            break;
          }
        }
        os->running2 = 0; 
      }
    }
  

    if(nuclei>=3){
      running=os->running3;  
      printf("\trunning pid on core 3: %d\n", running?running->pid:-1);
      if (running) {
        ProcessEvent* e=(ProcessEvent*) running->events.first;
        assert(e->type==CPU);
        e->duration--;
        printf("\t\tremaining time:%d\n",e->duration);
        if (e->duration==0){
          printf("\t\tend burst\n");
          List_popFront(&running->events);
          free(e);
          if (! running->events.first) {
            printf("\t\tend process\n");
            free(running); // kill process
          } else {
            e=(ProcessEvent*) running->events.first;
            switch (e->type){
            case CPU:
              printf("\t\tmove to ready\n");
              List_pushBack(&os->ready, (ListItem*) running);
              break;
            case IO:
              printf("\t\tmove to waiting\n");
              List_pushBack(&os->waiting, (ListItem*) running);
              break;
            }
          }
          os->running3 = 0;  
        }
      }
      if(nuclei>=4){
        running=os->running4;  
        printf("\trunning pid on core 4: %d\n", running?running->pid:-1);
        if (running) {
          ProcessEvent* e=(ProcessEvent*) running->events.first;
          assert(e->type==CPU);
          e->duration--;
          printf("\t\tremaining time:%d\n",e->duration);
          if (e->duration==0){
            printf("\t\tend burst\n");
            List_popFront(&running->events);
            free(e);
            if (! running->events.first) {
              printf("\t\tend process\n");
              free(running); // kill process
            } else {
              e=(ProcessEvent*) running->events.first;
              switch (e->type){
              case CPU:
                printf("\t\tmove to ready\n");
                List_pushBack(&os->ready, (ListItem*) running);
                break;
              case IO:
                printf("\t\tmove to waiting\n");
                List_pushBack(&os->waiting, (ListItem*) running);
                break;
              }
            }
            os->running4 = 0;  
          }
        }
        if(nuclei>=5){
          running=os->running5;  
          printf("\trunning pid on core 5: %d\n", running?running->pid:-1);
          if (running) {
            ProcessEvent* e=(ProcessEvent*) running->events.first;
            assert(e->type==CPU);
            e->duration--;
            printf("\t\tremaining time:%d\n",e->duration);
            if (e->duration==0){
              printf("\t\tend burst\n");
              List_popFront(&running->events);
              free(e);
              if (! running->events.first) {
                printf("\t\tend process\n");
                free(running); // kill process
              } else {
                e=(ProcessEvent*) running->events.first;
                switch (e->type){
                case CPU:
                  printf("\t\tmove to ready\n");
                  List_pushBack(&os->ready, (ListItem*) running);
                  break;
                case IO:
                  printf("\t\tmove to waiting\n");
                  List_pushBack(&os->waiting, (ListItem*) running);
                  break;
                }
              }
              os->running5 = 0;  
            }
          }
          if(nuclei>=6){
            running=os->running6;  
            printf("\trunning pid on core 6: %d\n", running?running->pid:-1);
            if (running) {
              ProcessEvent* e=(ProcessEvent*) running->events.first;
              assert(e->type==CPU);
              e->duration--;
              printf("\t\tremaining time:%d\n",e->duration);
              if (e->duration==0){
                printf("\t\tend burst\n");
                List_popFront(&running->events);
                free(e);
                if (! running->events.first) {
                  printf("\t\tend process\n");
                  free(running); // kill process
                } else {
                  e=(ProcessEvent*) running->events.first;
                  switch (e->type){
                  case CPU:
                    printf("\t\tmove to ready\n");
                    List_pushBack(&os->ready, (ListItem*) running);
                    break;
                  case IO:
                    printf("\t\tmove to waiting\n");
                    List_pushBack(&os->waiting, (ListItem*) running);
                    break;
                  }
                }
                os->running6 = 0;  
              }
            }
          }
        }
      }
    }
  }*/

  aux = os->ready.first;
  while(aux){
    FakePCB* pcb = (FakePCB*) aux;
    printf("\nPID DELLA READY: %d\n",pcb->pid);
    aux = aux->next;
  }

    


  // call schedule, if defined
  // Controllo che ci sia almeno un core libero verificando che la lista dei running sia piena, se piena allora tutti i core sono occupati -> SKIP
  //if (os->schedule_fn && ! List_isFull(os->running))
  
  //CICLA QUA DENTRO-- non più I guess -- ora dovrei avert risolto con os->ready.first che mi faceva ciclare
  
  while(os->schedule_fn && (List_isFull(&os->running)==0) && os->ready.first){    
    printf("\nDEBUGGO\n");
    (*os->schedule_fn)(os, os->schedule_args); 
  }

  
  
 /*
  if (os->schedule_fn && ! os->running1){ 
    (*os->schedule_fn)(os, os->schedule_args); 
  }
  if (nuclei>=2 && os->schedule_fn && ! os->running2){ 
    (*os->schedule_fn)(os, os->schedule_args); 
  }
  if (nuclei>=3 && os->schedule_fn && ! os->running3){ 
    (*os->schedule_fn)(os, os->schedule_args); 
  }
  if (nuclei>=4 && os->schedule_fn && ! os->running4){ 
    (*os->schedule_fn)(os, os->schedule_args); 
  }
  if (nuclei>=5 && os->schedule_fn && ! os->running5){ 
    (*os->schedule_fn)(os, os->schedule_args); 
  }
  if (nuclei>=6 && os->schedule_fn && ! os->running6){ 
    (*os->schedule_fn)(os, os->schedule_args); 
  }*/

  // if running not defined and ready queue not empty
  // put the first in ready to run

  
  while (os->ready.first && (List_isFull(&os->running)==0)) {
    List_pushBack(&os->running, List_popFront(&os->ready));
  }
  
/*
  if (! os->running1 && os->ready.first) {
    os->running1=(FakePCB*) List_popFront(&os->ready);}
  if (nuclei>=2 && ! os->running2 && os->ready.first) {
    os->running2=(FakePCB*) List_popFront(&os->ready);}
  if (nuclei>=3 && ! os->running3 && os->ready.first) {
    os->running3=(FakePCB*) List_popFront(&os->ready);}
  if (nuclei>=4 && ! os->running4 && os->ready.first) {
    os->running4=(FakePCB*) List_popFront(&os->ready);}
  if (nuclei>=5 && ! os->running5 && os->ready.first) {
    os->running5=(FakePCB*) List_popFront(&os->ready);}
  if (nuclei>=6 && ! os->running6 && os->ready.first) {
    os->running6=(FakePCB*) List_popFront(&os->ready);}    
  */
  ++os->timer;

}

//non implementata?
void FakeOS_destroy(FakeOS* os) {
}
