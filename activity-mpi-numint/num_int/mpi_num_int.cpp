#include <iostream>
#include <cmath>
#include <cstdlib>
#include <chrono>
#include <mpi.h>
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif

float f1(float x, int intensity);
float f2(float x, int intensity);
float f3(float x, int intensity);
float f4(float x, int intensity);

#ifdef __cplusplus
}
#endif

double calc_numerical_integration(int functionid, int a, int b, int n, int intensity, int rank, int size){
    double x = (double) a;
    double y = (double) b;
    
    double res = 0;
    
    double temp;
    //switch case should be in for loop
    double partitionStart = rank * (n / size);
    double partitionEnd = (rank + 1) * (n / size);
  
   
    
  
    for (partitionStart; partitionStart < partitionEnd; partitionStart++){
        temp = (x+(partitionStart+0.5) * ((y-x)/n));
        
        switch(functionid){
    
        case 1 : 
            res += f1(temp, intensity);
            break;
        case 2 : 
            res += f2(temp, intensity);
            break;
        case 3 : 
            res += f3(temp, intensity);
            break;
        case 4 : 
            res += f4(temp, intensity);
        } 
    }
  
    
    res = ((y-x)/n) * res;
    
    return res;
}
  
int main (int argc, char* argv[]) {
  MPI_Init(&argc, &argv);

  int size;
  int rank;
  double zero = 0;
  

  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  
  int function_id = atoi(argv[1]);
  int a = atoi(argv[2]);
  int b = atoi(argv[3]);
  int n = atoi(argv[4]);
  int intensity = atoi(argv[5]);
  
  if (argc < 6) {
    std::cerr<<"usage: "<<argv[0]<<" <functionid> <a> <b> <n> <intensity>"<<std::endl;
    return -1;
  }
 
  
  auto start = std::chrono::system_clock::now();

  
  double r = calc_numerical_integration(function_id, a, b, n, intensity, rank, size);
  MPI_Reduce(&r, &zero, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

  auto end = std::chrono::system_clock::now(); //

  std::chrono::duration<double> elapsed_seconds = end-start;
  std::time_t end_time = std::chrono::system_clock::to_time_t(end);

  //printf("%lf\n", r);
  if(rank == 0){
    std::cout << zero;
  }
  
  fprintf(stderr, "%f\n", elapsed_seconds.count());
  //std::cerr << elapsed_seconds.count();

  MPI_Finalize();  

  return 0;
}

