#define PAGE_INTS (4096 / sizeof(int)) // 1024

// Force a 2-page (8KB) aligned data segment.
int big_data[PAGE_INTS * 2] __attribute__((aligned(4096))) = { 
    [0] = 10,                 // Page 1
    [PAGE_INTS] = 20          // Page 2
};

int _start() {
  
  // Causes Fault 2. Loader reads 4096 bytes for Page 1.
  int val1 = big_data[0];         
  
  // Causes Fault 3. Loader reads 4096 bytes for Page 2.
  int val2 = big_data[PAGE_INTS]; 
  
  return val1 + val2; // 10 + 20 = 30
}