#include <iostream>
#include <string.h>
#include <cmath>
#include <functional>
#include <chrono>
#include <stdio.h>
#include <stdlib.h>
#include "dynamic_loop.cpp"

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

//add granularity
double calc_numerical_integration(int functionid, int a, int b, int n, int intensity, int nb_threads, int granularity){
    DynamicLoop dynLoop;
    dynLoop.setThreads(nb_threads);
    dynLoop.setGranularity(granularity);
    
    double x = (double) a;
    double y = (double) b;
    
    double res = 0;
    double resG = 0;
    double temp;
    //switch case should be in for loop
    dynLoop.parfor<double>(0, n, 1,

			 //before
			  [&](double& tls) -> void{
						tls = 0;
			  },

			 //mid

	[&](int i, double& tls) -> void{

        temp = (x+(i+0.5) * ((y-x)/n));
       
        switch(functionid){
    
        case 1 : 
            tls  += f1(temp, intensity);
            break;
        case 2 : 
            tls += f2(temp, intensity);
            break;
        case 3 : 
           tls  += f3(temp, intensity);
            break;
        case 4 : 
            tls += f4(temp, intensity);
        }
	//tls=4444.884;
	},
	         //after
	        [&](double& tls) -> void{
		    res += tls;
		}
    );
    
    res = ((y-x)/n) * res;
    
    return res;
}


int main (int argc, char* argv[]) {

  int function_id = atoi(argv[1]);
  int a = atoi(argv[2]);
  int b = atoi(argv[3]);
  int n = atoi(argv[4]);
  int intensity = atoi(argv[5]);
  int nbthreads = atoi(argv[6]);
  int granularity = atoi(argv[7]);

  if (argc < 8) {
    std::cerr<<"usage: "<<argv[0]<<" <functionid> <a> <b> <n> <intensity> <nbthreads> <granularity>"<<std::endl;
    return -1;
  }

  auto start = std::chrono::system_clock::now(); //
  double r = calc_numerical_integration(function_id, a, b, n, intensity, nbthreads, granularity); //
  auto end = std::chrono::system_clock::now(); //

  std::chrono::duration<double> elapsed_seconds = end-start; //
  std::time_t end_time = std::chrono::system_clock::to_time_t(end); //

  printf("%lf\n", r);
  
  fprintf(stderr, "%f\n", elapsed_seconds.count()); //
  

  return 0;
}


