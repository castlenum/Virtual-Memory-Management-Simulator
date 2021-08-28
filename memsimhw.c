// V3irual Memory Simulator Homework
// One-level page table system with FIFO and LRU
// Two-level page table system with LRU
// Inverted page table with a hashing system 
// Submission Year:2019/10
// Student Name: Jeong Seong Soo
// Student Number:B411187
//
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define PAGESIZEBITS 12         // page size = 4Kbytes
#define VIRTUALADDRBITS 32      // virtual address space size = 4Gbytes
#define TRUE 1
#define FALSE 0
int firstLevelBits, phyMemSizeBits, numProcess,Symtype,TI;
int S_Flag=0;
int S_PTE_SIZE=0;
typedef struct pageTableEntry {
   int level;            // page table level (1 or 2)
   char valid;
   struct pageTableEntry *secondLevelPageTable;   // valid if this entry is for the first level page table (level = 1)
   int frameNumber;                        // valid if this entry is for the second level page table (level = 2)
}PTE;
typedef struct framePage {
   int number;         // frame number
   int pid;         // Process id that owns the frame
   int virtualPageNumber;         // virtual page number using the frame
}FP;
//FIFO를 위한 List_Based_Queue
typedef FP* DataF;
typedef struct F_node {
    DataF data;
   struct F_node* next;
}Node1;
typedef struct L_Queue {
   Node1* front;
   Node1* rear;
}Queue;
void Q_Init(Queue* q) { q->front = q->rear = NULL; }
int Q_IsEmpty(Queue* q) { 
   if (q->front == NULL)return TRUE;
   else return FALSE;
}
void Enqueue(Queue* q, DataF d) {
   Node1* newNode = (Node1*)malloc(sizeof(Node1));
   newNode->data = d;
   newNode->next = NULL;
   if (Q_IsEmpty(q))
      q->front = newNode;
   else
      q->rear->next = newNode;
   q->rear = newNode;
}
DataF Deque(Queue* q) {
   if (Q_IsEmpty(q))exit(-1);
   Node1* rpos = q->front;
   DataF rdata = rpos->data;
   q->front = q->front->next;
   free(rpos);
   return rdata;
}
//LRU를 위한 Doubly Linked List
typedef struct L_Node{
   DataF data;
   struct L_Node* prev;
   struct L_Node* next;
}Node2;
typedef struct LinkedList{
   Node2* tail;
   Node2* head;
   Node2* cur;
   int N_O_D;
}List;

//List ADT
void L_Init(List* l) {
   l->head = (Node2*)malloc(sizeof(Node2));
   l->head->data = (FP*)malloc(sizeof(FP));
   l->tail = (Node2*)malloc(sizeof(Node2));
   l->tail->data = (FP*)malloc(sizeof(FP));
   l->head->prev = NULL;
   l->tail->next = NULL;
   l->head->next = l->tail;
   //l->head->next->data = l->tail->data;
   l->tail->prev = l->head;
   //l->tail->prev->data = l->head->data;
   l->cur = NULL;
   l->N_O_D = 0;
}
void L_Insert(List* l, DataF data) {
   Node2* newNode = (Node2*)malloc(sizeof(Node2));
   newNode->data = data;
   l->head->next->prev = newNode;
   newNode->next = l->head->next;
   if (l->head->next != l->tail) {
      l->head->next->prev->data = newNode->data;
      newNode->next->data = l->head->next->data;
   }
   newNode->prev = l->head;
   newNode->prev->data = l->head->data;
   l->head->next = newNode;
   l->head->next->data = newNode->data;
   l->N_O_D++;
   //printf("Input Data:pid:%d Vpn:%d FN:%d\n", newNode->data->pid, newNode->data->virtualPageNumber, newNode->data->number);
}
//searching it&seperating it
Node2* L_Search(List* l, int key) {
   l->cur = l->head->next;
   if (l->cur->data->number == key) {
	//puts("Key");
	   l->cur->prev->next = l->cur->next;
	   l->cur->next->prev = l->cur->prev;
	   return l->cur;
   }
   //printf("key: %d\n",key);
   //problem
   while (l->cur->next != l->tail) {
      if (l->cur->data->number == key) {
         //printf("I find a key in FN:%d\n",l->cur->data->number);
         (l->cur->prev)->next = l->cur->next;
         (l->cur->next)->prev = l->cur->prev;
         return l->cur;
      }
      else{
      //printf("fiding\n");
      //printf("key in cur:%d\n",l->cur->data->number);
      l->cur=l->cur->next;
      }
   }
   if(l->cur->data->number==key){
      //puts("Key!");
      l->cur->prev->next=l->cur->next;
      l->cur->next->prev=l->cur->prev;
      return l->cur;
   }
   //printf("I can't find a key\n");
   return NULL;
}
typedef struct procEntry {
   PTE* firstLevelPageTable;
   char* traceName;         // the memory trace name
   int pid;               // process (trace) id
   int ntraces;            // the number of memory traces
   int numPageFault;         // The number of page faults
   int numPageHit;            // The number of page hits
   //for 2 Level
   int num2ndLevelPageTable;   // The 2nd level page created(allocated);
   int numIHTConflictAccess;    // The number of Inverted Hash Table Conflict Accesses
   int numIHTNULLAccess;      // The number of Empty Inverted Hash Table Accesses
   int numIHTNonNULLAcess;      // The number of Non Empty Inverted Hash Table Accesses*/
   FILE* tracefp;
}PE;
int L_SearchIHT(List* l,DataF key,PE* pe,int pid) {
   l->cur = l->head->next;
   if (l->cur->data->pid == key->pid && l->cur->data->virtualPageNumber == key->virtualPageNumber) {
	   return l->cur->data->number;
   }
   //printf("key: %d\n",key);
   //problem
   while (l->cur->next != l->tail) {
      if (l->cur->data->pid == key->pid&&l->cur->data->virtualPageNumber==key->virtualPageNumber) {
         return l->cur->data->number;
      }
      else {
         //printf("fiding\n");
        // printf("key in cur:%d\n",l->cur->data->number);
         l->cur = l->cur->next;
      }
   }
   if (l->cur->data->pid == key->pid && l->cur->data->virtualPageNumber == key->virtualPageNumber) {
      return l->cur->data->number;
   }
   //printf("I can't find a key\n");
   return -1;
}
int L_get(List* l, DataF key) {
   l->cur = l->head->next;
   if (l->cur->data->pid == key->pid && l->cur->data->virtualPageNumber == key->virtualPageNumber) {
	   return l->cur->data->number;
   }
   //printf("key: %d\n",key);
   //problem
   while (l->cur->next != l->tail) {
      if (l->cur->data->pid == key->pid && l->cur->data->virtualPageNumber == key->virtualPageNumber) {
         return l->cur->data->number;
      }
      else {
         //printf("fiding\n");
         //printf("key in cur:%d\n",l->cur->data->number);
         l->cur = l->cur->next;
      }
   }
   if (l->cur->data->pid == key->pid && l->cur->data->virtualPageNumber == key->virtualPageNumber) {
      return l->cur->data->number;
   }
   //printf("I can't find a key\n");
   return -1;
}
//l->tail->prev delete
DataF L_Remove(List* l) {
   Node2* rpos = l->tail->prev;
   DataF rdata = rpos->data;
   rpos->prev->next = rpos->next;
   rpos->next->prev = rpos->prev;
   l->tail->prev = rpos->prev;
   free(rpos);
   l->N_O_D--;
   return rdata;
}
//Remove For IHT
//update needed
void L_SR(List* l, int vpn, int frame) {
	l->cur = l->head->next;
	//printf("Finfing Data:pid:%d Vpn:%d FN:%d\n", l->cur->data->pid, l->cur->data->virtualPageNumber, l->cur->data->number);
	//("Target Data: Vpn:%d FN:%d\n", vpn, frame);
	if (l->cur == l->tail) {
		return;
	}
	else 
	{
		if (l->cur->next == l->tail) {
			if (l->cur->data->number == frame && l->cur->data->virtualPageNumber == vpn) {
				//puts("Key!");
				l->cur->prev->next->data = NULL;
				//puts("11");
				l->cur->prev->next = l->cur->next;
				//puts("22");
				l->cur->next->prev = l->cur->prev;
				//puts("complete");
				return;
			}
		}
		while (l->cur->next != l->tail) {
			// puts("no ");
		  // printf("Finfing Data:pid:%d Vpn:%d FN:%d\n", l->cur->data->pid, l->cur->data->virtualPageNumber, l->cur->data->number);
			// puts("yes");
			if (l->cur->data->number == frame && l->cur->data->virtualPageNumber == vpn) {
				 //printf("I find a key in FN:%d\n",l->cur->data->number);
				l->cur->prev->next->data = l->cur->next->data;
				l->cur->prev->next = l->cur->next;
				l->cur->next->prev->data = l->cur->prev->data;
				l->cur->next->prev = l->cur->prev;
				return;
			}
			else {
				//  printf("");
				  //printf("fiding\n");
				  //printf("key in cur:%d\n",l->cur->data->number);
				l->cur = l->cur->next;
			}
		}
		if (l->cur->next == l->tail) {
			if (l->cur->data->number == frame && l->cur->data->virtualPageNumber == vpn) {
				//puts("Key!");
				l->cur->prev->next->data = NULL;
				l->cur->prev->next = l->cur->next;
				l->cur->next->prev = l->cur->prev;
			}
		}
	}
	
   //printf("I can't find a key\n");
}
void L_Concatenate(List* l, Node2** node) {
   (*node)->next = l->head->next;
   l->head->next->prev = (*node);
   (*node)->prev = l->head;
   l->head->next = (*node);
}

int L_IsEmpty(List* l) {
   if (l->head->next == l->tail){
  // puts("empty");
   return TRUE;
   }
   else{   
    //puts("Non empty");
    return FALSE;
   }
}
int L_Idx(int Vpn, int i, int nFrame) { return ((Vpn + i) % nFrame); }

/*void L_Print(List* l) {
   l->cur = l->head->next;
   if (l->head->next == l->tail)
	   puts("Empty entry");
   while (l->cur != l->tail) {
	printf("element==>:pid:%d  VN:%d  FN:%d\n", (*l->cur).data->pid, (*l->cur).data->virtualPageNumber, (*l->cur).data->number);
      l->cur = l->cur->next;
   }
}*/
FP* key;FP* Container;FP* oldestFrame; // the oldest frame pointer
void oneLevelVMSim(PE *procTable, FP *phyMemFrames,Queue* Fq,List* Dl,char FIFOorLRU,int nFrame) {
   int i,j,PFF,End;
   i = j = PFF;
   Node2* O_Node;
   unsigned int Vaddr,Paddr,Vpn,Offset;
   char rw='a';
   if (FIFOorLRU == 'F')
   {
      while (1) 
      {   
         Vaddr=Paddr=Vpn=Offset=0;
         End = 1;
         for (i = 0; i < numProcess; i++) {
            //unfinished process left.
            if (feof(procTable[i].tracefp) == 0)
               End = 0;
         }
         if (End == 1)break;
         for (i = 0; i < numProcess; i++)
         {
            fscanf(procTable[i].tracefp, "%x %c", &Vaddr, &rw);
            if(feof(procTable[i].tracefp)!=0)
               continue;
            Vpn = Vaddr >> PAGESIZEBITS;
            Offset = Vaddr - (Vpn << PAGESIZEBITS);
            PFF = 0;
            //page hit
            if (procTable[i].firstLevelPageTable[Vpn].valid == 1) 
            {
               Paddr = ((procTable[i].firstLevelPageTable[Vpn].frameNumber) << (PAGESIZEBITS))+Offset;
            //   printf("page hit\n");
               procTable[i].numPageHit++;
            }
            //page fault
            else if (procTable[i].firstLevelPageTable[Vpn].valid == 0) {
               procTable[i].numPageFault++;
               for (j = 0; j < nFrame; j++) {
                  if (phyMemFrames[j].pid == -1) {
                     //printf("Empty Frame allocated\n");
                     Enqueue(Fq, &phyMemFrames[j]);
                     phyMemFrames[j].pid = i;
                     phyMemFrames[j].virtualPageNumber = Vpn;
                     //VPE Update
                     procTable[i].firstLevelPageTable[Vpn].frameNumber = j;
                     procTable[i].firstLevelPageTable[Vpn].valid = 1;
                     Paddr = (j << PAGESIZEBITS) + Offset;
                     PFF = 1;
                     break;
                  }
               }
               switch (PFF) {
               case 1:
                  break;
               case 0:   
                  //printf("All Frame Allocated\n");
                  //#2.No Empty Frame
                  oldestFrame = Deque(Fq);
                  //old VPE Update
                  procTable[oldestFrame->pid].firstLevelPageTable[oldestFrame->virtualPageNumber].valid= 0;
                  //oldestFrame update
                  oldestFrame->pid = i;
                  oldestFrame->virtualPageNumber = Vpn;
                  //Enqueue updated Frame
                  Enqueue(Fq,oldestFrame);
                  //new VPE Update
                  procTable[i].firstLevelPageTable[Vpn].frameNumber = oldestFrame->number;
                  procTable[i].firstLevelPageTable[Vpn].valid = 1;
                  Paddr = ((procTable[i].firstLevelPageTable[Vpn].frameNumber) << (PAGESIZEBITS)) + Offset;
                  break;
               }
               
            }
            procTable[i].ntraces++;
            if(S_Flag==1)   
            printf("One-Level procID %d traceNumber %d virtual addr %x physical addr %x\n", i, procTable[i].ntraces, Vaddr, Paddr);
         }
         
      }//while(1) ends.
      for (i = 0; i < numProcess; i++) {
         printf("**** %s *****\n", procTable[i].traceName);
         printf("Proc %d Num of traces %d\n", i, procTable[i].ntraces);
         printf("Proc %d Num of Page Faults %d\n", i, procTable[i].numPageFault);
         printf("Proc %d Num of Page Hit %d\n", i, procTable[i].numPageHit);
         assert(procTable[i].numPageHit + procTable[i].numPageFault == procTable[i].ntraces);
      }
   }
   else if (FIFOorLRU == 'L') {
      while (1)
      {
         Vaddr = Paddr = Vpn = Offset = 0;
         End = 1;
         for (i = 0; i < numProcess; i++) {
            //unfinished process left.
            if (feof(procTable[i].tracefp) == 0)
               End = 0;
         }
         if (End == 1)break;
         for (i = 0; i < numProcess; i++)
         {
            fscanf(procTable[i].tracefp, "%x %c", &Vaddr, &rw);
            if (feof(procTable[i].tracefp) != 0)
               continue;
            Vpn = Vaddr >> PAGESIZEBITS;
            Offset = Vaddr - (Vpn << PAGESIZEBITS);
            PFF = 0;
            //page fault
            if (procTable[i].firstLevelPageTable[Vpn].valid == 0) {
               procTable[i].numPageFault++;
               //printf("pagefault\n");
               //searching for empty frame
               for (j = 0; j < nFrame; j++) {
                  if (phyMemFrames[j].pid == -1) {
                     //printf("Empty Frame allocated\n");
                     phyMemFrames[j].pid = i;
                     phyMemFrames[j].virtualPageNumber = Vpn;
                     //printf("Frame No:%d to Pid:%d VPN: %d\n",j,i,Vpn);
                     //VPE Update
                     procTable[i].firstLevelPageTable[Vpn].frameNumber = j;
                     procTable[i].firstLevelPageTable[Vpn].valid = 1;
                     //printf("Pid:%d %dthVPN Update:FN: %d\n",i,Vpn,j);
                     //List_Insert
                     L_Insert(Dl,&phyMemFrames[j]);
                     //printf("Insert ok\n");
                     Paddr = (j << PAGESIZEBITS) + Offset;
                     PFF = 1;
                     break;
                  }
               }
               switch (PFF) {
               case 1:
                  break;
               //No Empty Frame
               case 0:
                  //printf("All Frame Allocated\n");
                  oldestFrame =L_Remove(Dl);
                  //old VPE Update
                  procTable[oldestFrame->pid].firstLevelPageTable[oldestFrame->virtualPageNumber].valid = 0;
                  //oldestFrame update
                  oldestFrame->pid = i;
                  oldestFrame->virtualPageNumber = Vpn;
                  //Insert updated Frame
                  L_Insert(Dl, oldestFrame);
                  //new VPE Update
                  procTable[i].firstLevelPageTable[Vpn].frameNumber = oldestFrame->number;
                  procTable[i].firstLevelPageTable[Vpn].valid = 1;
                  Paddr = ((procTable[i].firstLevelPageTable[Vpn].frameNumber) << (PAGESIZEBITS)) + Offset;
                  break;
               }

            }
            //page hit
            else if (procTable[i].firstLevelPageTable[Vpn].valid == 1)
            {   //printf("pageHit\n");
               O_Node=L_Search(Dl, procTable[i].firstLevelPageTable[Vpn].frameNumber);
               //printf("Instructioncomplete\n");
               if (O_Node != NULL) {
                  L_Concatenate(Dl,&O_Node);
                  Paddr = ((procTable[i].firstLevelPageTable[Vpn].frameNumber) << (PAGESIZEBITS)) + Offset;
               }
               procTable[i].numPageHit++;
            }
            procTable[i].ntraces++;
            if(S_Flag==1)
            printf("One-Level procID %d traceNumber %d virtual addr %x physical addr %x\n", i, procTable[i].ntraces, Vaddr, Paddr);
         }

      }//while(1) ends.
      for (i = 0; i < numProcess; i++) {
         printf("**** %s *****\n", procTable[i].traceName);
         printf("Proc %d Num of traces %d\n", i, procTable[i].ntraces);
         printf("Proc %d Num of Page Faults %d\n", i, procTable[i].numPageFault);
         printf("Proc %d Num of Page Hit %d\n", i, procTable[i].numPageHit);
         assert(procTable[i].numPageHit + procTable[i].numPageFault == procTable[i].ntraces);
      }
   }
   
}
void twoLevelVMSim(struct procEntry* procTable, struct framePage* phyMemFrames, List* Dl, int nFrame) {
   int i, j, PFF, End;
   Node2* O_Node;
   int S_MP, S_SP, S_OF;
   S_OF = PAGESIZEBITS ;
   S_MP = firstLevelBits;
   S_SP = VIRTUALADDRBITS - PAGESIZEBITS - firstLevelBits;
   unsigned int Vaddr, Paddr, Vpn, Offset, M_Page, S_page;
   char rw = 'a';
   while (1)
   {
      Vaddr = Paddr = Vpn = Offset = 0;
      End = 1;
      for (i = 0; i < numProcess; i++) {
         //unfinished process left.
         if (feof(procTable[i].tracefp) == 0)
            End = 0;
      }
      if (End == 1)break;
      for (i = 0; i < numProcess; i++)
      {
         fscanf(procTable[i].tracefp, "%x %c", &Vaddr, &rw);
         if (feof(procTable[i].tracefp) != 0)
            continue;
         Vpn = Vaddr >> (PAGESIZEBITS);
         Offset = Vaddr - (Vpn << PAGESIZEBITS);
         M_Page = Vaddr >> (VIRTUALADDRBITS- firstLevelBits);
         S_page = ((Vpn<< PAGESIZEBITS)-(M_Page<<(VIRTUALADDRBITS- firstLevelBits)))>> PAGESIZEBITS;
         S_PTE_SIZE = (1 << (VIRTUALADDRBITS - PAGESIZEBITS - firstLevelBits));
         PFF = 0;
         //printf("Vaddr:%d    VPN:%d      M_Page:%d   S_Page:%d   Offset:%d   S_PTE_SIZE:%d\n",Vaddr, Vpn, M_Page, S_page, Offset,S_PTE_SIZE);
         
         //page fault
         //case1:Master Table entry가 invalid 한 경우.
         if (procTable[i].firstLevelPageTable[M_Page].valid == 0) {
            procTable[i].numPageFault++;
            procTable[i].num2ndLevelPageTable++;
            //printf("pagefault\n");
            procTable[i].firstLevelPageTable[M_Page].valid = 1;
            procTable[i].firstLevelPageTable[M_Page].secondLevelPageTable = (PTE*)malloc(sizeof(PTE)*S_PTE_SIZE);
            for (j = 0; j < S_PTE_SIZE; j++) {
               procTable[i].firstLevelPageTable[M_Page].secondLevelPageTable[j].level = 2;
               procTable[i].firstLevelPageTable[M_Page].secondLevelPageTable[j].frameNumber = -1;
               procTable[i].firstLevelPageTable[M_Page].secondLevelPageTable[j].valid =0;
            }
            //searching for empty frame
            for (j = 0; j < nFrame; j++) {
               if (phyMemFrames[j].pid == -1) {
                  //printf("Empty Frame allocated\n");
                  phyMemFrames[j].pid = i;
                  phyMemFrames[j].virtualPageNumber =Vpn;
                  //printf("Frame No:%d to Pid:%d VPN: %d\n",j,i,Vpn);
                  //VPE Update
                  procTable[i].firstLevelPageTable[M_Page].secondLevelPageTable[S_page].frameNumber = j;
                  procTable[i].firstLevelPageTable[M_Page].secondLevelPageTable[S_page].valid = 1;
                  //printf("Pid:%d %dthVPN Update:FN: %d\n",i,Vpn,j);
                  //List_Insert
                  L_Insert(Dl, &phyMemFrames[j]);
                  //printf("Insert ok\n");
                  Paddr = (j << PAGESIZEBITS) + Offset;
                  PFF = 1;
                  break;
               }
            }
            switch (PFF) {
            case 1:
               break;
               //No Empty Frame
            case 0:
               //printf("All Frame Allocated\n");
               oldestFrame = L_Remove(Dl);
               //old VPE Update
               procTable[oldestFrame->pid].firstLevelPageTable[((oldestFrame->virtualPageNumber)>>S_SP)].secondLevelPageTable[(oldestFrame->virtualPageNumber)-(((oldestFrame->virtualPageNumber) >> S_SP)<<S_SP)].valid = 0;
               //oldestFrame update
               oldestFrame->pid = i;
               oldestFrame->virtualPageNumber = Vpn;
               //Insert updated Frame
               L_Insert(Dl, oldestFrame);
               //new VPE Update
               procTable[i].firstLevelPageTable[M_Page].secondLevelPageTable[S_page].frameNumber= oldestFrame->number;
               procTable[i].firstLevelPageTable[M_Page].secondLevelPageTable[S_page].valid= 1;
               Paddr = ((oldestFrame->number) << (PAGESIZEBITS))+ Offset;
               break;
            }

         }
         else if (procTable[i].firstLevelPageTable[M_Page].valid == 1) {
            //pageFault
            if (procTable[i].firstLevelPageTable[M_Page].secondLevelPageTable[S_page].valid == 0) {
               procTable[i].numPageFault++;
               procTable[i].firstLevelPageTable[M_Page].secondLevelPageTable[S_page].valid = 1;
               //searching for empty frame
               for (j = 0; j < nFrame; j++) {
                  if (phyMemFrames[j].pid == -1) {
                     //printf("Empty Frame allocated\n");
                     phyMemFrames[j].pid = i;
                     phyMemFrames[j].virtualPageNumber = Vpn;
                     //printf("Frame No:%d to Pid:%d VPN: %d\n",j,i,Vpn);
                     //VPE Update
                     procTable[i].firstLevelPageTable[M_Page].secondLevelPageTable[S_page].frameNumber = j;
                     procTable[i].firstLevelPageTable[M_Page].secondLevelPageTable[S_page].valid = 1;
                     //printf("Pid:%d %dthVPN Update:FN: %d\n",i,Vpn,j);
                     //List_Insert
                     L_Insert(Dl, &phyMemFrames[j]);
                     //printf("Insert ok\n");
                     Paddr = (j << PAGESIZEBITS) + Offset;
                     PFF = 1;
                     break;
                  }
               }
               switch (PFF) {
               case 1:
                  break;
                  //No Empty Frame
               case 0:
                  //printf("All Frame Allocated\n");
                  oldestFrame = L_Remove(Dl);
                  //old VPE Update
                  procTable[oldestFrame->pid].firstLevelPageTable[((oldestFrame->virtualPageNumber) >> S_SP)].secondLevelPageTable[(oldestFrame->virtualPageNumber) - (((oldestFrame->virtualPageNumber) >> S_SP) << S_SP)].valid = 0;
                  //oldestFrame update
                  oldestFrame->pid = i;
                  oldestFrame->virtualPageNumber = Vpn;
                  //Insert updated Frame
                  L_Insert(Dl, oldestFrame);
                  //new VPE Update
                  procTable[i].firstLevelPageTable[M_Page].secondLevelPageTable[S_page].frameNumber = oldestFrame->number;
                  procTable[i].firstLevelPageTable[M_Page].secondLevelPageTable[S_page].valid = 1;
                  Paddr = ((oldestFrame->number) << (PAGESIZEBITS))+Offset;
                  break;
               }
            }
            //page hit
            else if (procTable[i].firstLevelPageTable[M_Page].secondLevelPageTable[S_page].valid == 1) {
               //printf("pageHit\n");
               O_Node = L_Search(Dl, procTable[i].firstLevelPageTable[M_Page].secondLevelPageTable[S_page].frameNumber);
               //printf("Instructioncomplete\n");
               if (O_Node != NULL) {
                  L_Concatenate(Dl, &O_Node);
                  Paddr = ((procTable[i].firstLevelPageTable[M_Page].secondLevelPageTable[S_page].frameNumber) << (PAGESIZEBITS)) + Offset;
               }
               procTable[i].numPageHit++;
            }

         }
         procTable[i].ntraces++;
         if (S_Flag == 1)
            printf("Two-Level procID %d traceNumber %d virtual addr %x physical addr %x\n", i, procTable[i].ntraces, Vaddr, Paddr);
      }

   }

      for (i = 0; i < numProcess; i++) {
         printf("**** %s *****\n", procTable[i].traceName);
         printf("Proc %d Num of traces %d\n", i, procTable[i].ntraces);
         printf("Proc %d Num of second level page tables allocated %d\n", i, procTable[i].num2ndLevelPageTable);
         printf("Proc %d Num of Page Faults %d\n", i, procTable[i].numPageFault);
         printf("Proc %d Num of Page Hit %d\n", i, procTable[i].numPageHit);
         assert(procTable[i].numPageHit + procTable[i].numPageFault == procTable[i].ntraces);
      }
      
}
void invertedPageVMSim(PE* procTable, FP* phyMemFrames, List* Dl,List* IHT, int nFrame) {
   int i, j, PFF, End, idx, IsIn;
   Node2* O_Node; Node2* Flag;
   unsigned int Vaddr, Paddr, Vpn, Offset;
   unsigned int old_idx, old_pid, old_Vpn, old_FN;
   char rw = 'a';
   while (1)
   {
      Vaddr = Paddr = Vpn = Offset = 0;
      End = 1;
      for (i = 0; i < numProcess; i++) {
         //unfinished process left.
         if (feof(procTable[i].tracefp) == 0)
            End = 0;
      }
      if (End == 1)break;
      for (i = 0; i < numProcess; i++)
      {
      // printf("---------------------------------------------\n");
         fscanf(procTable[i].tracefp, "%x %c", &Vaddr, &rw);
         if (feof(procTable[i].tracefp) != 0)
            continue;
         Vpn = Vaddr >> (PAGESIZEBITS);
         Offset = Vaddr - (Vpn << PAGESIZEBITS);
         PFF = 0;
         idx = (Vpn+i)%nFrame;
		//printf("pid %d Vpn:%d idx:%d\n",i,Vpn,idx);
		 if (L_IsEmpty(&IHT[idx]))
		 {
			 //puts("Empty IHTE");
			 procTable[i].numPageFault++;;
			 procTable[i].numIHTNULLAccess++;
			 for (j = 0; j < nFrame; j++) {
				 if (phyMemFrames[j].pid == -1) {
					 //List Init
					// puts("LRU LIST");
					// L_Print(Dl);
					 phyMemFrames[j].pid = i;
					 phyMemFrames[j].number = j;
					 phyMemFrames[j].virtualPageNumber = Vpn;
					 L_Insert(Dl,&phyMemFrames[j]);
					 //puts("to");
					 //L_Print(Dl);
					 //Entry Init
					// puts("IHTE");
					// L_Print(&IHT[idx]);
					 Container[j].number = j;
					 Container[j].pid = i;
					 Container[j].virtualPageNumber = Vpn;
					 L_Insert(&IHT[idx],&Container[j]);
					// puts("to");
					// L_Print(&IHT[idx]);
					 Paddr = (j << (PAGESIZEBITS)) + Offset;
					 PFF = 1;
					 break;
				 }
			 }
			 switch (PFF) {
			 case 1:
				 break;
			 case 0:
				 //puts("LRU LIST");
				// L_Print(Dl);
				 oldestFrame = L_Remove(Dl);
				// L_Print(Dl);
				 old_pid = oldestFrame->pid;
				 old_FN = oldestFrame->number;
				 old_Vpn = oldestFrame->virtualPageNumber;
				 old_idx = (old_pid + old_Vpn) % nFrame;
				// puts("IHTE");
				// L_Print(&IHT[old_idx]);
				 //puts("to");
				 L_SR(&IHT[old_idx],old_Vpn,old_FN);
				// L_Print(&IHT[old_idx]);
				 phyMemFrames[old_FN].pid = i;
				 phyMemFrames[old_FN].virtualPageNumber = Vpn;
				 phyMemFrames[old_FN].number=old_FN;
				// puts("LRU LIST");
				// L_Print(Dl);
				 L_Insert(Dl, &phyMemFrames[old_FN]);
				 //L_Print(Dl);
				// L_Print(&IHT[idx]);
				 Container[old_FN].virtualPageNumber = Vpn;
				 Container[old_FN].pid = i;
				 Container[old_FN].number = old_FN;
				 //puts("IHTE");
				 L_Insert(&IHT[idx],&Container[old_FN]);
				// L_Print(&IHT[idx]);
				 Paddr = (old_FN << PAGESIZEBITS) + Offset;
			 }
		 }
		 else if (!L_IsEmpty(&IHT[idx])) {
			 IsIn = 0;
			//puts("Non Empty IHTE");
			 procTable[i].numIHTNonNULLAcess++;
			 IHT[idx].cur = IHT[idx].head->next;
			 do{
				
				 if (IHT[idx].cur == IHT[idx].tail)
					 break;
				 if (IHT[idx].cur->data->virtualPageNumber != Vpn || IHT[idx].cur->data->pid != i) {
					// puts("2154");
					 procTable[i].numIHTConflictAccess++;
					 IHT[idx].cur = IHT[idx].cur->next;
				 }
				 else if (IHT[idx].cur->data->virtualPageNumber == Vpn && IHT[idx].cur->data->pid == i) {
					 IsIn = 1;
					 procTable[i].numIHTConflictAccess++;
					 old_FN = IHT[idx].cur->data->number;
					 break;

				 }
			 } while (IHT[idx].cur!=IHT[idx].tail);
			 switch (IsIn)
			 {	
				//page hit
			 case 1:
				//puts("page hit");
				 procTable[i].numPageHit++;
				// puts("LRU LIST");
				// L_Print(Dl);
				 O_Node = L_Search(Dl,old_FN);
				 L_Concatenate(Dl,&O_Node);
				// L_Print(Dl);
				 Paddr = ((old_FN) << (PAGESIZEBITS)) + Offset;
				 break;
			 //page fault
			 case 0:
				 PFF = 0;
				// puts("page fault");
				 procTable[i].numPageFault++;;
				 for (j = 0; j < nFrame; j++) {
					 if (phyMemFrames[j].pid == -1) {
						 //List Init
						// puts("LRU LIST");
						// L_Print(Dl);
						 phyMemFrames[j].pid = i;
						 phyMemFrames[j].number = j;
						 phyMemFrames[j].virtualPageNumber = Vpn;
						 L_Insert(Dl, &phyMemFrames[j]);
						// puts("to");
						// L_Print(Dl);
						 //Entry Init
						// puts("IHTE");
						// L_Print(&IHT[idx]);
						// puts("to");
						 Container[j].number = j;
						 Container[j].pid = i;
						 Container[j].virtualPageNumber = Vpn;
						 L_Insert(&IHT[idx], &Container[j]);
						// L_Print(&IHT[idx]);
						 Paddr = (j << (PAGESIZEBITS)) + Offset;
						 PFF = 1;
						 break;
					 }
				 }
				 switch (PFF) {
				 case 1:
					 break;
				 case 0:
					// puts("LRU LIST");
					 ///L_Print(Dl);
					 oldestFrame = L_Remove(Dl);
					// L_Print(Dl);
					 old_pid = oldestFrame->pid;
					 old_FN = oldestFrame->number;
					 old_Vpn = oldestFrame->virtualPageNumber;
					 old_idx = (old_pid + old_Vpn) % nFrame;
					// puts("IHTE");
					// L_Print(&IHT[old_idx]);
					// puts("to");
					 L_SR(&IHT[old_idx], old_Vpn, old_FN);
					// L_Print(&IHT[old_idx]);
					 phyMemFrames[old_FN].pid = i;
					 phyMemFrames[old_FN].virtualPageNumber = Vpn;
					 phyMemFrames[old_FN].number = old_FN;
					 //puts("LRU LIST");
					// L_Print(Dl);
					 L_Insert(Dl, &phyMemFrames[old_FN]);
					// L_Print(Dl);
					// puts("IHTE");
					// L_Print(&IHT[idx]);
					 Container[old_FN].virtualPageNumber = Vpn;
					 Container[old_FN].pid = i;
					 Container[old_FN].number = old_FN;
					 L_Insert(&IHT[idx], &Container[old_FN]);
					 //L_Print(&IHT[idx]);
					 Paddr = (old_FN << PAGESIZEBITS) + Offset;
				 }
			 }


		 }
         procTable[i].ntraces++;
         if (S_Flag == 1)
            printf("IHT procID %d traceNumber %d virtual addr %x physical addr %x\n", i, procTable[i].ntraces, Vaddr, Paddr);
      }
   
   }
   
   for (i = 0; i < numProcess; i++) {
      printf("**** %s *****\n", procTable[i].traceName);
      printf("Proc %d Num of traces %d\n", i, procTable[i].ntraces);
      printf("Proc %d Num of Inverted Hash Table Access Conflicts %d\n", i, procTable[i].numIHTConflictAccess);
      printf("Proc %d Num of Empty Inverted Hash Table Access %d\n", i, procTable[i].numIHTNULLAccess);
      printf("Proc %d Num of Non-Empty Inverted Hash Table Access %d\n", i, procTable[i].numIHTNonNULLAcess);
      printf("Proc %d Num of Page Faults %d\n", i, procTable[i].numPageFault);
      printf("Proc %d Num of Page Hit %d\n", i, procTable[i].numPageHit);
      assert(procTable[i].numPageHit + procTable[i].numPageFault == procTable[i].ntraces);
      assert(procTable[i].numIHTNULLAccess + procTable[i].numIHTNonNULLAcess == procTable[i].ntraces);
   }
}
   
   
int main(int argc, char* argv[]) {
   int i, j;
   int N_PTE; int D_PTE;
   PE* procTable;
   PE* procTable2;
   PE* procTable3;
   FP* MainMemory;
   Queue Fq;
   List Dl;
   List Dl2;
   List Dl3;
   List* IHT;
   L_Init(&Dl);
   L_Init(&Dl2);
   L_Init(&Dl3);
   Q_Init(&Fq);
   if (argc == 1) {
      printf("Usage : %s [-s] firstLevelBits PhysicalMemorySizeBits TraceFileNames\n", argv[0]); exit(1);
   }
   if (strcmp(argv[1], "-s") == 0) {
      S_Flag = 1;
      Symtype = atoi(argv[2]);
      firstLevelBits = atoi(argv[3]);
      phyMemSizeBits = atoi(argv[4]);
      numProcess = argc - 5;
      TI = 5;
   }
   else if (strcmp(argv[1], "-s") != 0) {
      Symtype = atoi(argv[1]);
      firstLevelBits = atoi(argv[2]);
      phyMemSizeBits = atoi(argv[3]);
      numProcess = argc - 4;
      TI = 4;
   }
   if (Symtype == 0)
      N_PTE = (1 << (VIRTUALADDRBITS - PAGESIZEBITS));
   else if (Symtype == 1)
      N_PTE = (1 << firstLevelBits);
   else if (Symtype == 3) {
	   N_PTE = (1 << (VIRTUALADDRBITS - PAGESIZEBITS));
	   D_PTE = (1 << firstLevelBits);
   }
   if (phyMemSizeBits < PAGESIZEBITS) {
      printf("PhysicalMemorySizeBits %d should be larger than PageSizeBits %d\n", phyMemSizeBits, PAGESIZEBITS); exit(1);
   }
   if (VIRTUALADDRBITS - PAGESIZEBITS - firstLevelBits <= 0) {
      printf("firstLevelBits %d is too Big for the 2nd level page system\n", firstLevelBits); exit(1);
   }
   // initialize procTable for memory simulations
   for (i = 0; i < numProcess; i++) {
      // opening a tracefile for the process
      printf("process %d opening %s\n", i, argv[i + TI]);
   }
   int nFrame = (1 << (phyMemSizeBits - PAGESIZEBITS)); assert(nFrame > 0);
   MainMemory = (FP*)malloc(sizeof(FP) * nFrame);
   procTable = (PE*)malloc(sizeof(PE) * numProcess);
   procTable2 = (PE*)malloc(sizeof(PE) * numProcess);
   procTable3 = (PE*)malloc(sizeof(PE) * numProcess);
   for (i = 0; i < nFrame; i++) {
      MainMemory[i].number = i;
      MainMemory[i].pid = -1;
      MainMemory[i].virtualPageNumber = -1;
   }
   printf("\nNum of Frames %d Physical Memory Size %ld bytes\n", nFrame, (1L << phyMemSizeBits));
   //initialize procTable for the simulation
   switch (Symtype) {
   case 0:
      printf("=============================================================\n");
      printf("The One-Level Page Table with FIFO Memory Simulation Starts .....\n");
      printf("=============================================================\n");
      // call oneLevelVMSim() with FIFO
	  for (i = 0; i < numProcess; i++) {
		  // initialize procTable fields
		  procTable[i].traceName = argv[i + TI];
		  procTable[i].pid = i;
		  procTable[i].ntraces = 0;
		  procTable[i].numPageFault = 0;
		  procTable[i].numPageHit = 0;
		  procTable[i].tracefp = fopen((const char*)procTable[i].traceName, "r");
		  procTable[i].firstLevelPageTable = (PTE*)malloc(sizeof(PTE) * N_PTE);
		  for (j = 0; j < N_PTE; j++) {
			  procTable[i].firstLevelPageTable[j].valid = 0;
			  procTable[i].firstLevelPageTable[j].frameNumber = -1;
		  }
		  // rewind tracefiles
		  rewind(procTable[i].tracefp);
	  }
	  oneLevelVMSim(procTable, MainMemory, &Fq, &Dl, 'F', nFrame);
      // initialize procTable for the simulation
      printf("=============================================================\n");
      printf("The One-Level Page Table with LRU Memory Simulation Starts .....\n");
      printf("=============================================================\n");
      // call oneLevelVMSim() with LRU
      for (i = 0; i < nFrame; i++) {
         MainMemory[i].number = i;
         MainMemory[i].pid = -1;
         MainMemory[i].virtualPageNumber = -1;
      }
      for (i = 0; i < numProcess; i++) {
         // initialize procTable fields
         procTable[i].traceName = argv[i + TI];
         procTable[i].pid = i;
         procTable[i].ntraces = 0;
         procTable[i].numPageFault = 0;
         procTable[i].numPageHit = 0;
         procTable[i].tracefp = fopen((const char*)procTable[i].traceName, "r");
		 for (j = 0; j < N_PTE; j++) {
			 procTable[i].firstLevelPageTable[j].valid = 0;
			 procTable[i].firstLevelPageTable[j].frameNumber = -1;
		 }
         rewind(procTable[i].tracefp);
      }
	  oneLevelVMSim(procTable, MainMemory, &Fq, &Dl, 'L', nFrame);
      break;
   case 1:
      // initialize procTable for the simulation
      printf("=============================================================\n");
      printf("The Two-Level Page Table Memory Simulation Starts .....\n");
      printf("=============================================================\n");
      // call twoLevelVMSim()
	  for (i = 0; i < numProcess; i++) {
		  // initialize procTable fields
		  procTable[i].traceName = argv[i + TI];
		  procTable[i].pid = i;
		  procTable[i].ntraces = 0;
		  procTable[i].numPageFault = 0;
		  procTable[i].numPageHit = 0;
		  procTable[i].tracefp = fopen((const char*)procTable[i].traceName, "r");
		  procTable[i].num2ndLevelPageTable = 0;
		  procTable[i].firstLevelPageTable = (PTE*)malloc(sizeof(PTE) * N_PTE);
	      for (j = 0; j < N_PTE; j++) {
			 procTable[i].firstLevelPageTable[j].valid = 0;
			procTable[i].firstLevelPageTable[j].level = 1;
			procTable[i].firstLevelPageTable[j].frameNumber = -1;
			procTable[i].firstLevelPageTable[j].secondLevelPageTable = NULL;
	      }
		  // rewind tracefiles
		  rewind(procTable[i].tracefp);
      }
      twoLevelVMSim(procTable, MainMemory, &Dl, nFrame);
      break;
   case 2:
	   for (i = 0; i < numProcess; i++) {
		   // initialize procTable fields
		   procTable[i].traceName = argv[i + TI];
		   procTable[i].pid = i;
		   procTable[i].ntraces = 0;
		   procTable[i].numPageFault = 0;
		   procTable[i].numPageHit = 0;
		   procTable[i].tracefp = fopen((const char*)procTable[i].traceName, "r");
		   procTable[i].numIHTConflictAccess = 0;
		   procTable[i].numIHTNULLAccess = 0;
		   procTable[i].numIHTNonNULLAcess = 0;
		   // rewind tracefiles
		   rewind(procTable[i].tracefp);
	   }
      Container=(FP*)malloc(sizeof(FP)*nFrame);
      IHT = (List*)malloc(sizeof(List) * nFrame);
      for (j = 0; j < nFrame; j++) {
      	 Container[j].number=j;
      	 Container[j].pid=-1;
      	 Container[j].virtualPageNumber=-1;
         L_Init(&IHT[j]);
      }
      // initialize procTable for the simulation
      printf("=============================================================\n");
      printf("The Inverted Page Table Memory Simulation Starts .....\n");
      printf("=============================================================\n");
      invertedPageVMSim(procTable, MainMemory, &Dl,IHT, nFrame);
      // call invertedPageVMsim()
      break; 
	case 3:
		printf("=============================================================\n");
		printf("The One-Level Page Table with FIFO Memory Simulation Starts .....\n");
		printf("=============================================================\n");
		// call oneLevelVMSim() with FIFO
		for (i = 0; i < numProcess; i++) {
			// initialize procTable fields
			procTable[i].traceName = argv[i + TI];
			procTable[i].pid = i;
			procTable[i].ntraces = 0;
			procTable[i].numPageFault = 0;
			procTable[i].numPageHit = 0;
			procTable[i].tracefp = fopen((const char*)procTable[i].traceName, "r");
			procTable[i].firstLevelPageTable = (PTE*)malloc(sizeof(PTE) * N_PTE);
			for (j = 0; j < N_PTE; j++) {
				procTable[i].firstLevelPageTable[j].valid = 0;
				procTable[i].firstLevelPageTable[j].frameNumber = -1;
			}
			// rewind tracefiles
			rewind(procTable[i].tracefp);
		}
		oneLevelVMSim(procTable, MainMemory, &Fq, &Dl, 'F', nFrame);
		// initialize procTable for the simulation
		printf("=============================================================\n");
		printf("The One-Level Page Table with LRU Memory Simulation Starts .....\n");
		printf("=============================================================\n");
		// call oneLevelVMSim() with LRU
		for (i = 0; i < nFrame; i++) {
			MainMemory[i].number = i;
			MainMemory[i].pid = -1;
			MainMemory[i].virtualPageNumber = -1;
		}
		for (i = 0; i < numProcess; i++) {
			// initialize procTable fields
			procTable[i].traceName = argv[i + TI];
			procTable[i].pid = i;
			procTable[i].ntraces = 0;
			procTable[i].numPageFault = 0;
			procTable[i].numPageHit = 0;
			procTable[i].tracefp = fopen((const char*)procTable[i].traceName, "r");
			for (j = 0; j < N_PTE; j++) {
				procTable[i].firstLevelPageTable[j].valid = 0;
				procTable[i].firstLevelPageTable[j].frameNumber = -1;
			}
			rewind(procTable[i].tracefp);
		}
		oneLevelVMSim(procTable, MainMemory, &Fq, &Dl, 'L', nFrame);
		free(procTable);
		//case1
		 for (i = 0; i < nFrame; i++) {
			 MainMemory[i].number = i;
			 MainMemory[i].pid = -1;
			 MainMemory[i].virtualPageNumber = -1;
		 }
		 for (i = 0; i < numProcess; i++) {
			 // initialize procTable fields
			 procTable2[i].traceName = argv[i + TI];
			 procTable2[i].pid = i;
			 procTable2[i].ntraces = 0;
			 procTable2[i].numPageFault = 0;
			 procTable2[i].numPageHit = 0;
			 procTable2[i].tracefp = fopen((const char*)procTable2[i].traceName, "r");
			 procTable2[i].num2ndLevelPageTable = 0;
			 procTable2[i].firstLevelPageTable = (PTE*)malloc(sizeof(PTE) * D_PTE);
			 for (j = 0; j < D_PTE; j++) {
				 procTable2[i].firstLevelPageTable[j].valid = 0;
				 procTable2[i].firstLevelPageTable[j].level = 1;
				 procTable2[i].firstLevelPageTable[j].frameNumber = -1;
				 procTable2[i].firstLevelPageTable[j].secondLevelPageTable = NULL;
			 }
			 // rewind tracefiles
			 rewind(procTable2[i].tracefp);
		 }
        printf("=============================================================\n");
        printf("The Two-Level Page Table Memory Simulation Starts .....\n");
        printf("=============================================================\n");
		twoLevelVMSim(procTable2, MainMemory, &Dl2, nFrame);
		free(procTable2);
        printf("=============================================================\n");
        printf("The Inverted Page Table Memory Simulation Starts .....\n");
        printf("=============================================================\n");
	//case2
	   for (i = 0; i < nFrame; i++) {
			 MainMemory[i].number = i;
			 MainMemory[i].pid = -1;
			 MainMemory[i].virtualPageNumber = -1;
		}
	   for (i = 0; i < numProcess; i++) {
		   // initialize procTable fields
		   procTable3[i].traceName = argv[i + TI];
		   procTable3[i].pid = i;
		   procTable3[i].ntraces = 0;
		   procTable3[i].numPageFault = 0;
		   procTable3[i].numPageHit = 0;
		   procTable3[i].tracefp = fopen((const char*)procTable3[i].traceName, "r");
		   procTable3[i].num2ndLevelPageTable = 0;
		   procTable3[i].numIHTConflictAccess = 0;
		   procTable3[i].numIHTNULLAccess = 0;
		   procTable3[i].numIHTNonNULLAcess = 0;
		   // rewind tracefiles
		   rewind(procTable3[i].tracefp);

	   }
	   Container = (FP*)malloc(sizeof(FP) * nFrame);
	   IHT = (List*)malloc(sizeof(List) * nFrame);
	   for (j = 0; j < nFrame; j++) {
		   Container[j].number = j;
		   Container[j].pid = -1;
		   Container[j].virtualPageNumber = -1;
		   L_Init(&IHT[j]);
	   }
	   // initialize procTable for the simulation
	   invertedPageVMSim(procTable3, MainMemory, &Dl3, IHT, nFrame);
	   // call invertedPageVMsim()

	   break;
   }
   return 0;
}
