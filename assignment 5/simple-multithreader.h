#ifndef SIMPLE_MULTITHREADER_H
#define SIMPLE_MULTITHREADER_H

#include <iostream>
#include <list>
#include <functional>
#include <stdlib.h>
#include <cstring>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include <stdio.h>

int user_main(int argc, char **argv);

int main(int argc, char **argv) {
  return user_main(argc, argv);
}
#define main user_main

typedef struct {
  std::function<void(int)> lambda;
  int low;
  int high;
} thread_args_vector;

typedef struct {
  std:: function<void(int, int)> lambda;
  int low1;
  int high1;
  int low2;
  int high2;
} thread_args_matrix;

void* thread_funct_vector(void *ptr){
    thread_args_vector *t = (thread_args_vector*)ptr; 
    for(int i = t->low; i< t->high; i++){
      t->lambda(i);
    }
    return NULL;
}

void *thread_funct_matrix(void *ptr){
  thread_args_matrix *t = (thread_args_matrix*)ptr;
  for(int i = t->low1; i < t->high1; i++){
    for (int j = t->low2; j < t->high2; j++){
      t->lambda(i,j);
    }
  }
  return NULL;
}


void parallel_for(int low , int high , std:: function <void(int)> && lambda, int numThread){
  struct  timespec startTime, endTime;
  clock_gettime(CLOCK_MONOTONIC, &startTime); //start time 

  int numWorker = numThread - 1;
  int safeSize = (numWorker>0)?numWorker:1;
  pthread_t tid[safeSize];
  thread_args_vector args[safeSize];
  int chunkSize = (high - low)/numThread;

  for(int i = 0; i<numWorker; i++){
    args[i].lambda = lambda;
    args[i].low = low + (i*chunkSize);
    args[i].high = args[i].low + chunkSize;
    //create thread
    if (pthread_create(&tid[i],NULL, thread_funct_vector, (void*)&args[i])!=0){
      std::cerr << "Error creating thread " << std::endl;
      exit(1);
    }
  }

int main_start = low + (numWorker * chunkSize);
for(int i = main_start; i<high; i++){
    lambda(i);
}
  //thread join
  for(int i = 0; i<numWorker; i++){
    if(pthread_join(tid[i],NULL) != 0){
      std::cerr << "Error Joining thread" << std::endl;
      exit(1);
    }
  }
  // end time 
  clock_gettime(CLOCK_MONOTONIC, &endTime);
  double duration = (endTime.tv_sec - startTime.tv_sec) + (endTime.tv_nsec - startTime.tv_nsec)/1e9;
  std:: cout << "Time spend in parallel_for : " << std::fixed << duration << " seconds" << std::endl;
}

void parallel_for(int low1,int high1,int low2,int high2, std:: function<void(int, int)> &&lambda, int numThread){

  struct timespec startTime, endTime;  // start time for 2d
  clock_gettime(CLOCK_MONOTONIC, &startTime);
  int numWorker = numThread - 1;
  int safeSize = (numWorker>0)?numWorker:1;
  pthread_t tid[safeSize];
  thread_args_matrix args[safeSize];

  int chunkSize = (high1-low1)/numThread;

  for(int i = 0; i<numWorker; i++){
    args[i].lambda = lambda;
    args[i].low1 = low1 + (i*chunkSize);
    args[i].high1 = args[i].low1 + chunkSize;
    args[i].low2 = low2;
    args[i].high2 = high2;

    // create thread
    if (pthread_create(&tid[i],NULL, thread_funct_matrix,(void*)&args[i]) !=0){
        std::cerr << "Error creating thread " << std::endl;
        exit(1);
    }
  }

  int mainStartRow = low1 + (numWorker*chunkSize);
  for(int i = mainStartRow; i<high1; i++){
    for(int j = low2; j<high2; j++){
      lambda(i,j);
    }
  }
 // join threads
for (int i = 0; i<numWorker; i++){
  if(pthread_join(tid[i],NULL) !=0){
    std::cerr << "Error joinning Thread " << std::endl;
    exit(1);
  }
}
  //end time for 2d
  clock_gettime(CLOCK_MONOTONIC, &endTime);
  double duration = (endTime.tv_sec - startTime.tv_sec) + (endTime.tv_nsec - startTime.tv_nsec)/1e9;
  std:: cout << "Time spend in parallel_for : " << std::fixed << duration << " seconds" << std::endl;
}

#endif
