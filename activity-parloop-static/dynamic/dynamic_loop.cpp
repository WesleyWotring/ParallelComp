#ifndef __DYNAMIC_LOOP_H
#define __DYNAMIC_LOOP_H

#include <functional>
#include <thread>
#include <vector>
#include <iostream>
#include <mutex>


class DynamicLoop {

private:
  // @brief you will want to have class member to store the number of threads
  // add a public setter function for this member.
  int nbthreads;
  int granularity;
public:
  // @breif write setters here.
  void setThreads(int nbt){
    this -> nbthreads = nbt;
  }

   void setGranularity(int gran){
    this -> granularity = gran;
  }



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
  void parfor(size_t beg, size_t end, size_t increment,
           std::function<void(TLS&)> before,
           std::function<void(int, TLS&)> f,
           std::function<void(TLS&)> after
           ) {
    std::vector<std::thread> mythread;
    //tls holder
    size_t chunk = end / granularity;
    std::vector<TLS> tlsVect;
    TLS tls[chunk];
    std::mutex mu;
    
    //chunk allocation
    std::vector<bool> checker(chunk, false);
    size_t sharedVariable = 0;
    
   
    //int numerator = 1;
    for(size_t b = 0; b < chunk; b++){
       TLS tlsBefore;
       tlsVect.push_back(tlsBefore);
       before(tlsVect[b]);
    }
    
    for(size_t j = 0; j < nbthreads; j++){
       //before(tls[j]);

           
    mythread.push_back(std::thread([&]() {
        while(sharedVariable < chunk){
          bool threadCheck = false;
          mu.lock();
          checker[sharedVariable] = true;

          size_t min = sharedVariable * granularity;
          size_t max = (min + granularity);
          TLS& tempTLS = tlsVect[sharedVariable];
          //std::cout<<tlsVect[sharedVariable]<< " \n";
          
          sharedVariable++;
          mu.unlock();
        
        
         for (size_t i=min; i< max; i+= increment) {
             //std::cout<<tempTLS<< " \n";
             f(i, tempTLS);
         }
         }
                   }));

      
     
      
          //std::cout<<tlsVect[1]<< " ";
    }

     //leave this alone
     std::for_each(mythread.begin(), mythread.end(), [](std::thread &t){
        t.join();
    });

     for(auto& t: tlsVect){
     //std::cout<< t << " \n";
      after(t);

    }
     
    //join threads
   

    
   
  }
 
};

#endif
