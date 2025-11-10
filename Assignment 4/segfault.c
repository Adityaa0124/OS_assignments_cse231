int _start() {
  
  // This will cause Fault 1 and load the .text page
  int x = 1; 

  // This is an invalid address, NOT in any of your ELF segments.
  int *ptr = (int*)0xDDEEAADD; 
  
  // This causes a REAL SIGSEGV.
  *ptr = 42; 
  
  return -1;
}