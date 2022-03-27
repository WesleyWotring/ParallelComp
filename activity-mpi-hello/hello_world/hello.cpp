#include <mpi.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <iostream>

int main(int argc, char*argv[]) {
  MPI_Init(&argc, &argv);

  char host[HOST_NAME_MAX + 1];
  //char array called host that hast the maximum of the name + 1 additional for size

  gethostname(host, HOST_NAME_MAX +1);
    //call the gethostname mathod to get the name that will iterate through the second variable being the length


  int size;
  int rank;

  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  sleep(rank);
  
  std::cout<<"I am process " <<rank<< " out of " <<size<< ". I am running on " <<host<< ". \n";

  MPI_Finalize();
}
