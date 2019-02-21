#include "paging.h"


unsigned int page_directory[PD_SIZE] __attribute__((aligned(FOURKB)));
unsigned int page_table[PD_SIZE] __attribute__((aligned(FOURKB)));
unsigned int page_vid[PD_SIZE] __attribute__((aligned(FOURKB)));
static unsigned int free_page[2];


/*
 * void paging_init()
 * input: none
 * output: none
 * result : create a page directory and a page table entries for initialization
 *          
 */

void paging_init(){
  int p = 0;
  int i;

  for(i=0; i<PD_SIZE; i++){
    page_directory[i]=PRESENT; //Initialize page directoy
    page_table[i] = p | PRESENT; 
    page_vid[i] = p | USER_FLAG;
    p = p + FOURKB; //add 4kB to the address each loop
  }
  free_page[0] = FREE_INIT;
  free_page[1] = 0;
  /*the page directory is initialized to user level here*/
  page_directory[0] = ((unsigned int)page_table) | 1;  
  page_directory[1] = (PD_SIZE*FOURKB) | SIZE_FLAG;
  page_table[VID_MEM] = (FOURKB*VID_MEM) | RWFLAG;
  page_table[VID_MEM+1] = (FOURKB*VID_MEM) | RWFLAG;
  page_table[VID_MEM+2] = (FOURKB*VID_MEM+FOURKB*2) | RWFLAG;
  page_table[VID_MEM+3] = (FOURKB*VID_MEM+FOURKB*3) | RWFLAG;
  page_enable(); // load PDE


}

void page_enable(){ //load the page directoy by changing cr values to assembly language
  asm volatile("movl %0, %%eax \n"
               "movl %%eax, %%cr3 \n"
               "movl %%cr4, %%eax \n"
               "orl  $0x00000010, %%eax \n"
               "movl %%eax, %%cr4 \n"
               "movl %%cr0, %%eax \n"
               "orl  $0x80000001, %%eax \n"
               "movl %%eax, %%cr0 \n"
               :
               : "g"(page_directory)
               : "%eax"
        );
}

/* 
 * allocate
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: the page directory entry index
 *   RESULT: allocate a new page and flush the TLB
 */

int allocate() {
    int idx;
    int i;
    
    if (idx == PD_SIZE) {
        return -1; // return error when the page directory index is 1024
      }
      /* iterate through the directory entries to search for avaiable place */
    for (idx = 0; idx < PD_SIZE; idx++) {
        if (!(page_directory[idx] & BITWISE) && (idx !=SIZE)) { break; }
      }

    for (i = 0; i < SIZE * 2; i++) {
        if (i!= SIZE && !((free_page[i/SIZE] >> (i%SIZE)) & BITWISE)) {
            page_directory[idx] = (i*PD_SIZE*FOURKB) | USER_PRE;
            free_page[i/SIZE] |= (BITWISE << (i%SIZE));
            break;
        }
    }
    flush(); //flush the TLB
    return idx;
    }


/*
 * void map_init(int idx)
 * input: index of the page directory that holds the address to map to                   
 * output: none
 * description :maps the virtual address to the physical address that starts the program
 *              
 */
void map_init(int idx) {
    page_directory[VIRTUAL_ADDR>>BIT_22] = page_directory[idx] | USER_PRE; // map the physical address 
    flush(); // flush the TLB
}

void video_map(int terminal_num, int curr) {
    
    page_table[VID_MEM + terminal_num] = (FOURKB*VID_MEM + curr) | RWFLAG;
    flush();
}

void next_terminal(int next, int curr) {
  
  video_map(curr, curr*FOURKB);
    memcpy((void*)(FOURKB*VID_MEM + FOURKB*curr), (void*)(FOURKB*VID_MEM), FOURKB);
    memcpy((void*)(FOURKB*VID_MEM), (void*)(FOURKB*VID_MEM + FOURKB*next), FOURKB);
    video_map(next, 0);
}
/*
 * void flush();
 * input: none              
 * output: none
 * description : flush the TLB by loading page directory          
 */
void flush(){
  asm volatile ("movl %0, %%eax \n"
                  "movl %%eax, %%cr3 \n"
                  :
                  : "g"(page_directory)
                  : "memory", "cc");
}

