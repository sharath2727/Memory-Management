
#include <stdio.h>
#include <unistd.h>

struct s_block {

  size_t size;
  struct s_block *next;
  int free;
  int dummy;
}*base = NULL ;

#define aligning4(x) (((((x)-1)>>2)<<2)+4)
#define BLOCK_SIZE (sizeof(struct s_block))

struct s_block * extend_heap(struct s_block *last, size_t size){

  struct s_block *b;

  b = sbrk(0);
  printf("Initial address: %u\n",b);
  if(sbrk(BLOCK_SIZE + size) == (void *)-1)
    return NULL;
  void *temp = sbrk(0);
  printf("Creating memory of size: %u\n",temp);
  b->size = size;
  b->next = NULL;
  if(last)
    last->next = b;
  b->free = 0;

  return b; 
}

// Find a free block
struct s_block * find_block(struct s_block **last, size_t size){

  struct s_block *b = base;
  while(b && !(b->free && b->size >= size)){
    *last = b;
    b = b->next;
  }
  return b;
}

//Split block
void split_block(struct s_block *b, size_t size){

  struct s_block * new;
  new = (char *)(b) + BLOCK_SIZE + size;
  new->size = b->size - size - BLOCK_SIZE;
  new->next = b->next;
  new->free = 1;
  b->size = size;
  b->next = new;
}  

//Malloc Functionality

void * my_malloc(size_t s){

  struct s_block *b, *last;
  size_t size;

  size = aligning4((s));
  if(base){
    last = base;
    b =  find_block(&last,size);
    if(b){
      /*Split the size to put data for the current block and meta-data for the next element */
      if((b->size - size) > (BLOCK_SIZE + 4))
        split_block(b,size);
      b->free = 0; // first block isn't free anymore
    }

    else{
      /* No fitting block, extend heap */
      b =  extend_heap(last,size);
      if(b ==  NULL)
        return NULL;
    }
  }

  else{
  /* first time base is NULL */
  b = extend_heap(NULL,size);
  if(b == NULL)
    return NULL;
  base = b;
  }
  return ((char *)b+BLOCK_SIZE);
}


int main(void){

  void *p = my_malloc(10);
  printf("%lu\n",p);
  void *q = my_malloc(10);
  printf("%lu\n",q);
  return 0;
}
