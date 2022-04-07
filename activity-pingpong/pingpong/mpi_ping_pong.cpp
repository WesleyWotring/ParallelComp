#include <iostream>
#include <cmath>
#include <cstdlib>
#include <chrono>
#include <mpi.h>
#include <vector>

int main (int argc, char* argv[]) {

  MPI_Init(&argc, &argv);

  int size;
  int rank;
  int input;

  

  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  
  if (argc < 2) {
    std::cerr<<"usage: mpirun "<<argv[0]<<" <value>"<<std::endl;
    return -1;
  }

  if(rank == 0){
   input = atoi(argv[1]);
    MPI_Status t;
    MPI_Send(&input, 1, MPI_INT, 1, 123, MPI_COMM_WORLD);
    std::cout<<"rank 0 input= "<<input<<" sent\n";
    MPI_Recv(&input, 1, MPI_INT, 1, 456, MPI_COMM_WORLD, &t);
    std::cout<<"rank 0 input= "<<input<<" recv\n";
      std::cout << input;
  }
  
  else if(rank == 1){
    MPI_Status s;
    MPI_Recv(&input, 1, MPI_INT, 0, 123, MPI_COMM_WORLD, &s);
    std::cout<<"rank 1 input= "<<input<<" recv\n";
    input = input + 2;
    MPI_Send(&input, 1, MPI_INT, 0, 456, MPI_COMM_WORLD);
    std::cout<<"rank 1 input= "<<input<<" sent\n";
  }

  //MPI_Send( what you send, amount of items, MPI_datatype, destination, tag, comm):
  

  MPI_Finalize();  

  return 0;
}
