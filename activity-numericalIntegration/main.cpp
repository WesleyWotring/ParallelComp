#include <iostream>
#include <cmath>
#include <cstdlib>
#include <chrono>

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


//function
double integrate(int id, double a, double b, int n, int intensity){

  double solution = 0;

  double equation;
  for (int i = 0; i < n; i++){
    //set the equation to a variable to iterate with the cases
    equation = (a + (i + 0.5) * ((b - a)/n));
    
    switch(id){
      //sum of the iterations
    case 1:
      solution += f1(equation, intensity);
      break;

    case 2:
      solution += f2(equation, intensity);
      break;

    case 3:
      solution += f3(equation, intensity);
      break;

    case 4:
      solution += f4(equation, intensity);
      break;
    }
  }
  //multiply the first part of the equation to the rest of the equation
    solution = ((b - a)/n) * solution;

    
    //return the value
    return solution;
}

  
int main (int argc, char* argv[]) {
  int id = atoi(argv[1]);
  int a = atoi(argv[2]);
  int b = atoi(argv[3]);
  int n = atoi(argv[4]); //number times of iteration
  int intensity = atoi(argv[5]);

  if (argc < 6) {
    std::cerr<<"usage: "<<argv[0]<<" <functionid> <a> <b> <n> <intensity>"<<std::endl;
    return -1;
  }
  //time stuff
		       auto start = std::chrono::system_clock::now();
		       double functionCall = integrate(id, a, b, n, intensity);
		       auto end = std::chrono::system_clock::now();

		       std::chrono::duration<double> time = end - start;
		       std::time_t end_time = std::chrono::system_clock::to_time_t(end);
		       fprintf(stdout, "%f\n", functionCall);
		       fprintf(stderr, "%f\n", time.count());
  
  return 0;
}
