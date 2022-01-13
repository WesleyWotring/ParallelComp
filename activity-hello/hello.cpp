#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

int main () {

  char host[HOST_NAME_MAX + 1];
  //char array called host that hast the maximum of the name + 1 additional for size

  gethostname(host, HOST_NAME_MAX +1);
    //call the gethostname mathod to get the name that will iterate through the second variable being the length
    
    printf("Host: %s\n", host);
    //print the host name
    
    return EXIT_SUCCESS;
}
