//
//  memmgr.c
//  memmgr
//
//  Created by William McCarthy on 17/11/20.
//  Copyright © 2020 William McCarthy. All rights reserved.
//
//  Hien Nguyen 889341772

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define ARGC_ERROR 1
#define FILE_ERROR 2
#define BUFLEN 256
#define FRAME_SIZE  256
#define PAGES 256
#define MEM_SIZE PAGES * FRAME_SIZE

int pagetable[PAGES];
signed char main_mem[MEM_SIZE];
signed char *backing_ptr;

//-------------------------------------------------------------------
unsigned getpage(unsigned x) { return (0xff00 & x) >> 8; }

unsigned getoffset(unsigned x) { return (0xff & x); }

void getpage_offset(unsigned x) {
  unsigned  page   = getpage(x);
  unsigned  offset = getoffset(x);
  printf("x is: %u, page: %u, offset: %u, address: %u, paddress: %u\n", x, page, offset,
         (page << 8) | getoffset(x), page * 256 + offset);
}

int main(int argc, const char* argv[]) {

  if (argc != 2) {
       printf( "Please enter 2 args: <./file_exe_name> <input        	file>\n");
        exit(ARGC_ERROR);
    }
  
  const char *input_file = argv[1];
  FILE* fadd = fopen( input_file, "r");    // open file addresses.txt  (contains the logical addresses)
  if (fadd == NULL) { fprintf(stderr, "Could not open file: 'addresses.txt'\n");  exit(FILE_ERROR);  }

  FILE* fcorr = fopen("correct.txt", "r");     // contains the logical and physical address, and its value
  if (fcorr == NULL) { fprintf(stderr, "Could not open file: 'correct.txt'\n");  exit(FILE_ERROR);  }

  int backing_ptr_fd = open("BACKING_STORE.bin", O_RDONLY);
  backing_str = mmap(0, MEM_SIZE, PROT_READ, MAP_PRIVATE, backing_ptr_fd, 0);

  char buf[BUFLEN];
  int total_add = 0;
  int pageFault = 0; 
  int tbl_hit = 0;
  for (int i = 0; i < PAGES; i++){
	pagetable[i] = -1;
  }
  unsigned   page, offset, physical_add, frame = 0;
  unsigned   logic_add;                  // read from file address.txt
  unsigned   virt_add, phys_add, value;  // read from file correct.txt

  unsigned char free_page = 0;

  while (fgets(buf, BUFLEN, fadd) != NULL) {

    total_add++;

    fscanf(fcorr, "%s %s %d %s %s %d %s %d", buf, buf, &virt_add,
           buf, buf, &phys_add, buf, &value);  // read from file correct.txt

    fscanf(fadd, "%d", &logic_add);  // read from file address.txt
    page   = getpage(  logic_add);
    offset = getoffset(logic_add);
    
    physical_add = frame++ * FRAME_SIZE + offset;
    
    assert(physical_add == phys_add);
   
    int physical = pagetable[page];

    if(physical == -1){
    		pageFault++;
		
		physical = free_page;

		free_page++;

		memcpy(main_mem + physical * FRAME_SIZE, backing_ptr + page * FRAME_SIZE, FRAME_SIZE);

		pagetable[page] = physical;
    }
    else{
		tlb_hit++;
    }
    
    printf("logical: %5u (page: %3u, offset: %3u) ---> physical: %5u -- passed\n", logic_add, page, offset, physical_add);
  }
  fclose(fcorr);
  fclose(fadd);
  
  printf("Number of Translated Addresses = %d\n", total_add);
  printf("Page Faults = %d\n", pageFault);
  printf("Page Fault Rate = %.3f\n", pageFault/ (1. * total_addresses));
  printf("TLB Hits = %d\n", tlb_hit);
  printf("TLB Hit Rate = %.3f\n", tlb_hit / (1. * total_addresses));
  
  printf("\n\t\t...done.\n");
  return 0;
}
