/*
Date      : 04/15/2016
Code      : Malloc and Free Implementation - First Fit Strategy
*/


#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>

#define align4(s) (((((s)-1)>>2)<<2)+4)
#define BLOCK_SIZE (sizeof(struct meta_block))

struct meta_block{

  size_t size;
  struct meta_block *next;
  struct meta_block *prev;
  int free;
};


void *global_base = NULL;


struct meta_block * extend_heap(struct meta_block *last, size_t size){

  struct meta_block *b;

  b = sbrk(0);

  if(sbrk(BLOCK_SIZE + size) == (void *)-1)
    return NULL;  

  b->size = size;
  b->next = NULL;
  b->prev = last;
  if(last)  
    last->next = b;

  b->free = 0;
  return b;
}


struct meta_block * find_block(struct meta_block **last, size_t size){

  struct meta_block *b = global_base;
  while(b && !(b->free == 1 && b->size >= size)){
    *last = b;
    b =  b->next;
  }
  return b;
}


/* Split the first fit block */
void split_block(struct meta_block *b, size_t size){

  struct meta_block *new;

  new = (struct meta_block *)((char *)(b) + (BLOCK_SIZE + size));
  new->size = b->size - (BLOCK_SIZE + size);
  new->free = 1;
  new->prev = b;
  new->next = b->next;
  if(new->next)
    new->next->prev = new;
  b->size = size;
  b->next = new;
}


void * my_malloc(size_t s){

  struct meta_block *b,*last;
  size_t size;

  if(size <= 0)
    return NULL;


  size = align4(s);
  printf("%lu\n",size); 
  /*First fit - memory that is enough to satisfy the first request
  Find the block whose size is greater than the requested size */
  if(global_base){
    last = global_base;
    b = find_block(&last,size);
    if(b){
      if((b->size - size) >= (BLOCK_SIZE + 4)){
        split_block(b,size);
      b->free = 0;
      }
    }
    /* No fitting block, extend heap */
    else{
      b = extend_heap(last,size);
      if( b == NULL)
        return NULL;
    }
  }
    

  else{
    b = extend_heap(NULL,size);    
    if(b == NULL)
      return NULL;
    global_base = b;
  }

  return (b+1);
}
  

bool isValid(struct meta_block *p){

  if(global_base){
    if(p > (struct meta_block *)global_base && p < (struct meta_block *)sbrk(0))
      return true;
  }
  return false;
}

struct meta_block * getBlock(void *p){

  p = (char *)p - BLOCK_SIZE;
  return p;

}

struct meta_block * join(struct meta_block *b){

  if(b->next && b->next->free == 1){
    b->size += BLOCK_SIZE + b->next->size;
    b->next  = b->next->next;
    if(b->next)
      b->next->prev = b;
  }
  return b;
}

void my_free(struct meta_block *p){

  struct meta_block *b;

  if(isValid(p)){
    b = getBlock(p);
    b->free = 1;
    /*Fusion */
    if(b->prev && b->prev->free)
      b = join(b->prev);
    
    if(b->next)
      join(b);

    else{
      /* last block then free it */
      if(b->prev)
        b->prev->next = NULL;
      else
        global_base = NULL;
      brk(b);
    }
  }
}

int main(void){
  
  printf("In main\n");
  void *p = my_malloc(10);
  printf("Base : %lu\n",(uint64_t)global_base);
  printf("p: %lu\n",(uint64_t)p);
  void *q = my_malloc(10);
  printf("q: %lu\n",(uint64_t)q);
  my_free(p);
  printf("p: %lu\n",(uint64_t)p);
  void *r = my_malloc(8);
  printf("r: %lu\n",(uint64_t)r);
  return 0;
}
