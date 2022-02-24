#ifndef __STATIC_LOOP_H
#define __STATIC_LOOP_H

#include <functional>
#include <thread>
#include <vector>
#include <iostream>

class StaticLoop {
private:
  // @brief you will want to have class member to store the number of threads
  // add a public setter function for this member.

public:
  // @breif write setters here.



  /// @brief execute the function f multiple times with different
  /// parameters possibly in parallel
  ///
  /// f will be executed multiple times with parameters starting at
  /// beg, no greater than end, in inc increment. These execution may
  /// be in parallel
  void parfor (size_t beg, size_t end, size_t inc,
	       std::function<void(int)> f) {
    for (size_t i=beg; i<end; i+= inc) {
      f(i);
    }
  }

  /// @brief execute the function f multiple times with different
  /// parameters possibly in parallel
  ///
  /// f will be executed multiple times with parameters starting at
  /// beg, no greater than end, in inc increment. These execution may
  /// be in parallel.
  ///
  /// Each thread that participate in the calculation will have its
  /// own TLS object.
  ///
  /// Each thread will execute function before prior to any
  /// calculation on a TLS object.
  ///
  /// Each thread will be passed the TLS object it executed before on.
  ///
  /// Once the iterations are complete, each thread will execute after
  /// on the TLS object. No two thread can execute after at the same time.
  template<typename TLS>
  void parfor(size_t beg, size_t end, size_t increment, size_t nbthreads,
	       std::function<void(TLS&)> before,
	       std::function<void(int, TLS&)> f,
	       std::function<void(TLS&)> after
	       ) {
    std::vector<std::thread> mythread;
    TLS tls[nbthreads];
   
    //int numerator = 1;
    for(size_t b = 0; b , nbthreads; b++){
       before(tls[b]); 
    }
	
    for(size_t j = 0; j < nbthreads; j++){
       //before(tls[j]);

       size_t newBeg = ((j) * end)/ nbthreads;
       size_t  newEnd = ((j+1) * end)/ nbthreads;
       
       mythread.push_back(std::thread([&]() {
        if(j == (nbthreads - 1)){
	  //special case here for the last thread
	     for (size_t i=newBeg; i<end; i+= increment) {
	         f(i, tls[j]);
		 //tls is all 0's at after
	     }
        }else{
	  //regular loop
             for (size_t i=newBeg; i<newEnd; i+= increment) {
	        f(i, tls[j]);

	     }}//std::cout<<tls[j]<< " ";
     }));

      //size_t  newBeg = newEnd;
     //numerator++;
      
          //std::cout<<tls[1]<< " ";
    }

     std::for_each(mythread.begin(), mythread.end(), [](std::thread &t){
        t.join();
    });

     for(int a = 0; a < nbthreads; a++){
	//std::cout<<tls[a]<< " ";
      after(tls[a]);

    }
     
    //join threads
   

    
   
  }
  
};

#endif
