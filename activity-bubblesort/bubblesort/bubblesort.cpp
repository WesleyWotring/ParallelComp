#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <chrono>

#include "omploop.hpp"

#ifdef __cplusplus
extern "C" {
#endif

  void generateMergeSortData (int* arr, size_t n);
  void checkMergeSortResult (const int* arr, size_t n);

#ifdef __cplusplus
}
#endif

void swap(int* arr, int i, int j) {
  int temp = arr[i];
  arr[i] = arr[j];
  arr[j] = temp;
}

int main (int argc, char* argv[]) {
  if (argc < 3) { std::cerr<<"usage: "<<argv[0]<<" <n> <nbthreads>"<<std::endl;
    return -1;
  }

  int n = atoi(argv[1]);
  int nbthreads = atoi(argv[2]);
  
  // get arr data
  int * arr = new int [n];
  generateMergeSortData (arr, n);

  //insert sorting code here.

  std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
  
  OmpLoop ompLoop;
  ompLoop.setNbThread(nbthreads);
  ompLoop.setGranularity( n / nbthreads );


  int swapped = 1;
  while (swapped) {
    swapped = 0;
    int beginning = 1;
    int end = n;
    //even parfor
    ompLoop.parfor<float>(beginning, end, 2, 
			[&](float& tls) -> void{
				//tls = 0;
			  }, 
				//middle of parfor
			[&](int i, float& tls) -> void{
			
			     if (arr[i-1] > arr[i]) {
       				 swap(arr, i-1, i);
        			swapped = 1;
      			     }  
			      
			  

			
			}
			,[&](float& tls) -> void{
			    //result += tls;
			});
    beginning =2;
    //odd parfor
    ompLoop.parfor<float>(beginning, end, 2, 
			[&](float& tls) -> void{
				//tls = 0;
			  }, 
				//middle of parfor
			[&](int i, float& tls) -> void{
			 if (arr[i-1] > arr[i]) {
       				 swap(arr, i-1, i);
       				 swapped = 1;
      			    }
			      
			     
			}
			,[&](float& tls) -> void{
			    //result += tls;
			});

  }

  std::chrono::time_point<std::chrono::system_clock> stop = std::chrono::system_clock::now();
  std::chrono::duration<double> elpased_seconds = stop-start;
  
  checkMergeSortResult (arr, n);
  std::cerr<<elpased_seconds.count()<<std::endl;

  
  //checkMergeSortResult (arr, n);
  
  delete[] arr;

  return 0;
}
