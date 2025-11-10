#define PAGE_INTS (4096 / sizeof(int)) // 1024
int __attribute__((aligned(4096))) big_array[PAGE_INTS * 10] = {0}; // 10 pages

int _start() {
  int sum = 0;
  
  // Access page 8
  big_array[PAGE_INTS * 8] = 10; 
  
  // Access page 1
  big_array[PAGE_INTS * 1] = 20;
  
  // Access page 8 again (no fault)
  sum = big_array[PAGE_INTS * 8];
  
  return sum; // 10
}