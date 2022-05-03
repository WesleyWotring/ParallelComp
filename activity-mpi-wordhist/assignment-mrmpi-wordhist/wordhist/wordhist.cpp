#include "mpi.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sys/stat.h"
#include "mapreduce.h"
#include "keyvalue.h"
#include <chrono>
#include <sstream>
#include <iostream>
#include <string>
#include <cctype>
#include <cstring>
#include <fstream>
#include <map>
#include <vector>
using namespace std;

using namespace MAPREDUCE_NS;

void printvector(std::vector<std::string> vec){
  for (auto &i: vec)
    std::cout << i << std::endl;
}

template<typename Key, typename Value>
void printKVpair (char* key, int keybytes,
          char* value, int valuebytes,
          void*) {
  //std::cout<<"(";
  //print keys
  {
    Key* base = (Key*)key;
    for (uint64_t i = 0; i<keybytes/sizeof(Key); i++) { //be careful of divisibility in malformed data
    //std::cout<<"keybytes is "<<keybytes<<std::endl;
    //std::cout<<"size(key) is "<<sizeof(Key)<<std::endl;
      //if (i != 0)
    //std::cout<<", ";
      std::cout<<base[i];
    }
  }

  std::cout<<", ";
  //print values
  {
    Value* base = (Value*)value;
    for (uint64_t i = 0; i<valuebytes/sizeof(Value); i++) { //be careful of divisibility in malformed data
      //if (i != 0)
    //std::cout<<", ";
      std::cout<<base[i];
    }
  }

  //std::cout<<")\n";
  std::cout<<std::endl;
}

//Step 1: Map (filename,word),count)
void mapfileword(int taskid, char* filename, MAPREDUCE_NS::KeyValue* kv, void* ptr){

  //std::cout<<"========= "<<taskid<<" "<<filename<<std::endl;  
  bool convert_to_lowercase = false;
  std::string line;
  std::stringstream dosString;
  std::ifstream inFile;
  inFile.open (filename);
  //std::vector<std::string> wordlist;
  char one = '1';
  char* ONE = &one;
  int filenameSize = strlen(filename);
  std::string fileNameArr = static_cast<std::string>(filename);
  std::string keyTupleString = (fileNameArr + ", ");
  //char* fileName = static_cast<char*>(filename.c_str());

      while(getline(inFile, line))
      {           
          dosString<<line<<"\r\n";
          std::string word;
          istringstream buffer(line);
        

          while (buffer >> word)
          {
            char* char_lower;
            char_lower =&word[0];
             for (std::string::size_type i=0; i<word.length(); ++i){
                if (convert_to_lowercase) {
                  char_lower[i] = std::tolower(word[i]);
                }
                else {
                  char_lower[i] = word[i];
                }
                 
             }
            std::string word_lower(char_lower);
            std::string keyTupleString2 = keyTupleString + word_lower;
            char* keyTuple = const_cast<char*>(keyTupleString2.c_str());

            int keyTupleSize = strlen(keyTuple);

            kv->add(keyTuple,keyTupleSize, ONE, 1);  //Only assign keys and NULL values
          }           
      }  
 
}

void red_wordcount(char *key, int keybytes,
        char *multivalue, int nvalue, int *valuebytes,
        MAPREDUCE_NS::KeyValue *kvout, void *ptr) {
    
    ////std::cout <<"nvalue is "<<nvalue<<std::endl;
    kvout->add(key,keybytes,(char *) &nvalue,sizeof(int));
}

void mapwc_freq(uint64_t taskid, char *key, int keybytes, char *value,
      int valuebytes, MAPREDUCE_NS::KeyValue *kv, void *context)
{
  char one = '1';
  char* ONE = &one;
  //std::cout <<"Key is "<<key<<std::endl;
  int int_value = *(int *) value;
  std::string str(key);  //"test_rain.txt, the"
  //std::cout <<"VAlue is "<<int_value<<std::endl;
  //std::string value_str = (std::string) int_value;
  std::stringstream ss;
  ss << int_value;
  std::string value_str = ss.str();

  std::string find_str = ",";
  

  std::size_t found = str.find(find_str);  //found = 13

  std::string key_sub = str.substr(0,found);    //test_rain.txt,
  std::string new_key = key_sub + ", " + value_str;    //"test_rain.txt,2"
  
  char* new_key_char = const_cast<char*>(new_key.c_str());
  int new_key_Size = strlen(new_key_char);

  kv->add(new_key_char,new_key_Size, ONE, 1);    // (test_rain.txt,2,1)
}

int main(int argc, char **argv)
{

  if (argc < 3) {
    std::cerr<<"usage: usage: ./wordfreq <filename1> <filename2> <filename3> ..."<<std::endl;
    return -1;
  }

  int nstr = argc-1;
  char** filelist = argv;
  uint64_t ret_reduce;

  for (int fileid = 1; fileid < argc; ++fileid) {
      filelist[fileid-1] = argv[fileid];
  }

  MPI_Init(&argc, &argv);
    int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank); 

  std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();  
  //Add time
  MAPREDUCE_NS::MapReduce mr;
  mr.map(nstr, filelist, 0, 0, 0, mapfileword, nullptr);
  //std::cout << "Map 1 output: "<<std::endl;
  //mr.scan(printKVpair<char, char>, nullptr);

  mr.collate(nullptr);
  mr.reduce(red_wordcount,NULL);
  
  //std::cout <<std::endl;
  //std::cout << "Reduce 1 output: "<<std::endl;
  //mr.scan(printKVpair<char, int>, nullptr);

  mr.map(&mr,mapwc_freq,nullptr);

  //std::cout <<std::endl;
  //std::cout << "Map 2 output: "<<std::endl;
  //mr.scan(printKVpair<char, char>, nullptr);

  mr.collate(nullptr);
  mr.reduce(red_wordcount,NULL);  

  //std::cout <<std::endl;
  //std::cout << "Reduce 2 output: "<<std::endl;
  mr.scan(printKVpair<char, int>, nullptr);  

  //End time
    std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;  
  if (rank==0){  
      std::cerr<<elapsed_seconds.count()<<std::endl;  
  }
  MPI_Finalize();
  //compute time and add cerr
  std::cerr<<elapsed_seconds.count()<<std::endl;  
  return 0;
}


