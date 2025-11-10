// A read-only string long enough to span two pages.
const char long_string[5000] __attribute__((aligned(4096))) = "START";

int _start() {
  
  // Access page 1 of the .rodata segment
  char a = long_string[0];
  
  // Access page 2 of the .rodata segment
  char b = long_string[4097]; 
  
  return a + b;
}