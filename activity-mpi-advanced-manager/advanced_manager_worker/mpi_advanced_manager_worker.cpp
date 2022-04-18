#include <mpi.h>
#include <iostream>

//Prasanna
#include <chrono>
#include <unistd.h>
#include <vector>
#include <map>


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

float getnumint(int functionid, double a, double b, int64_t n, int intensity, float mltp, int start, int end ){
  float loop_result = 0.0;

	//std::cout << "start and end are "<< start<< " and " <<end <<"\n";
  for (int i=start; i<end;i++) {
	switch (functionid) {
  	case 1:
    	loop_result+=f1((a + ((i + 0.5 ) * mltp)),intensity);
    	break;
  	case 2:
    	loop_result+=f2((a + ((i + 0.5 ) * mltp)),intensity);
    	break;
  	case 3:
    	loop_result+=f3((a + ((i + 0.5 ) * mltp)),intensity);
    	break;
  	case 4:
    	loop_result+=f4((a + ((i + 0.5 ) * mltp)),intensity);
    	break;
  	default:
    	std::cerr << "Invalid Functionid input " << functionid ;
    	return -1;
	}
  }
  return loop_result;
}

void printvector(const std::string & msg, const std::vector<int>& v) {
  std::cout<<msg;
  for (auto& a : v)
	std::cout<<a<<" ";
  	std::cout<<"\n";
}

bool ready_to_exit (std::map<int, std::map<int,int> >m,int process_rank){
    for (auto &item:m){
		//std::cout << "(RTE) item.first = " << item.first<<std::endl;
        if (item.first == process_rank){
			//std::cout << "(RTE) Inside "<< item.first<<std::endl;
            for (auto &inn:item.second){
				//std::cout << "(RTE) inn.first = " << inn.first<<std::endl;
				//std::cout << "(RTE) inn.second = " << inn.second<<std::endl;
                if (inn.second == 1){
                    return false;
                }
            }
        }
		else {
			//std::cout << "(RTE) skip"<<std::endl;
			continue;
		}
    }
    return true;
}

int main (int argc, char* argv[]) {

	if (argc < 6) {
	std::cerr<<"usage: mpirun -np <processes> "<<argv[0]<<" <functionid> <a> <b> <n> <intensity>"<<std::endl;
	return -1;
	}

	MPI_Init (&argc, &argv);

	int size;
	int rank;

	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);  

	int functionid = atoi(argv[1]);
	double a = atof(argv[2]);
	double b = atof(argv[3]);
	int64_t n = atoi(argv[4]);     //requirement said 64-bit integer
	int intensity = atoi(argv[5]);
	float mltp = ((b - a) / n );
	float loop_result = 0.0;
	float final_value = 0.0;
	int chunk;
	int counter = 0;
	float lp_res = 0.0;
	int quit_code = 0;
	std::vector<int> vec_comm_status(size);
    int vecsize = 2;		//Set how many chunks we want to send in one MPI_SEND	
	int vecsize_return = 3;		//holds loop result,chunk lowerbound, original sender TAG
	std::map<int, std::map<int,int> > map_send_tag_chunk;	//maps each chunk to a tag ID used in SEND
	std::map<int, std::map<int,int> > map_send_chunk_status;	//maps each chunk to a receive status (0 = nothing to receive, 1 = waiting to receive)

	//std::cout<<"Variables declared"<<std::endl;
	if ((n/(size * 2)) > 2){
		chunk = n/(size *2);
	}
	else{
		chunk = n/(size);
	}
	//std::cout<<"chunk value is set to "<<chunk<<std::endl;
  std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();

	if (rank ==0){
		//std::cout<<"In rank 0"<<std::endl;

    	std::vector<int> vec_work(vecsize);		
		vec_comm_status[0] = 0;		// 0 means nothing pending to receive. 1 means sent but not received.
		int init_dist = 0;
		int tag_id = 10000;
		bool can_break = false;

		while (can_break ==false ) {
			if (counter <=n ){
				//std::cout<<"While loop started"<<std::endl;
				if (init_dist == 0){
					//std::cout<<"Initial distribution hasnt been done"<<std::endl;
					for (int i=1;i<size;i++){   //I'm skipping zero because its the master but still is one of the processes.
						////std::cout << "counter is "<<counter<<"\n";
						//std::cout<<"preparing chunk for processor "<<i<<std::endl;
						vec_work.clear();
						for (int j =0; j<vecsize;j++){
							if (counter  <= n){
								//vec_work[j] = counter;
								vec_work.push_back(counter);
								map_send_tag_chunk[i][counter] = tag_id;
								map_send_chunk_status[i][counter] = 1;		//Im assigning value 1 assuming it will be sent.
								counter += chunk;
								//std::cout<<"Process "<<i<< " assigned with "<<vec_work[j]<<std::endl;
							}
							else{
								//std::cout<<"Initial dist is already over. Breaking for"<<std::endl;
								break;
							}						
						}	//end for j
						//std::cout<<"Loaded vector size now is "<<vec_work.size()<<std::endl;
						if (vec_work.size()!=0){
							//std::cout<<"send chunks to processor "<<i<<" using tag id "<<tag_id<<std::endl;
							MPI_Request req_send_0_init;
							MPI_Isend(&(vec_work[0]), vecsize, MPI_INT, i, tag_id, MPI_COMM_WORLD,&req_send_0_init);
							tag_id += 1; 

							MPI_Wait(&req_send_0_init,MPI_STATUS_IGNORE);
						}

					}  //end for i
					init_dist = 1;		//This means all initial distributions are over. dont go into for loop anymore.
				}	//end if init dist
				//std::cout<<" Initial dist passed"<<std::endl;

				if (counter  <= n){
					MPI_Status s;
					MPI_Request request_recv_0;
					std::vector<float> vec_result(vecsize_return);
					//std::cout<<"preparing to receive"<<std::endl;
					MPI_Irecv(&(vec_result[0]), vecsize_return, MPI_FLOAT, MPI_ANY_SOURCE, 456, MPI_COMM_WORLD, &request_recv_0);
					MPI_Wait(&request_recv_0,&s);
					//std::cout<< "Receiving result in 0 from "<<s.MPI_SOURCE <<"\n";
					//std::cout << vec_result[0] <<" is the interim result\n";
					
					loop_result+= vec_result[0]; //Equivalent to reduce but p2p.
					//std::cout<<"cumulative value of loop_Result is "<<loop_result;
					int ret_ctr = vec_result[1];
					map_send_chunk_status[s.MPI_SOURCE][ret_ctr] = 0;		//set flag to zero confirming result data is received
				

					//std::cout<<"chunks still more to go. Let me send next batch"<<std::endl;
					vec_work.clear();
					for (int j =0; j<vecsize;j++){
						if (counter  <= n){
							//vec_work[j] = counter;
							vec_work.push_back(counter);
							map_send_tag_chunk[s.MPI_SOURCE][counter] = tag_id;
							map_send_chunk_status[s.MPI_SOURCE][counter] = 1;		//Im assigning value 1 assuming it will be sent.
							counter += chunk;
							//std::cout<<"Process xx"<<s.MPI_SOURCE<< " assigned with "<<vec_work[j]<<std::endl;
						}
					}//end vecsize j loop
					//std::cout<<"Loaded vector size now is "<<vec_work.size()<<std::endl;
					if (vec_work.size()!=0){
						//std::cout<<"send chunks to processor "<<s.MPI_SOURCE<<" using tag id "<<tag_id<<std::endl;
						MPI_Request req_send_0_next;
						MPI_Isend(&(vec_work[0]), vecsize, MPI_INT, s.MPI_SOURCE, tag_id, MPI_COMM_WORLD,&req_send_0_next);
						tag_id += 1; 

						MPI_Wait(&req_send_0_next,MPI_STATUS_IGNORE);

					}
				}//if counter not execeeded
			}	
			if (counter > n){
				//std::cout<<"No more chunks left to send.  Let me check if i received all responses"<<std::endl;
				can_break = true;
				for (int i=1;i<size;i++){
					if (!ready_to_exit(map_send_chunk_status,i)){
						MPI_Status s;
						MPI_Request request_recv_0;
						std::vector<float> vec_result(vecsize_return);
						//std::cout<<"preparing to receive"<<std::endl;
						MPI_Irecv(&(vec_result[0]), vecsize_return, MPI_FLOAT, MPI_ANY_SOURCE, 456, MPI_COMM_WORLD, &request_recv_0);
						MPI_Wait(&request_recv_0,&s);
						//std::cout<< "Receiving result STILL in 0 from "<<s.MPI_SOURCE <<"\n";
						//std::cout << vec_result[0] <<" is the interim result\n";
						
						loop_result+= vec_result[0]; //Equivalent to reduce but p2p.
						//std::cout<<"cumulative value of loop_Result is "<<loop_result <<std::endl;
						int ret_ctr = vec_result[1];
						map_send_chunk_status[s.MPI_SOURCE][ret_ctr] = 0;		//set flag to zero confirming result data is received
						can_break = false;
					}
					else{
						//std::cout<<"All chunks received. Sending 99 signal in a loop to each process"<<std::endl;
						MPI_Send(&vec_work[0],vecsize, MPI_INT, i, 99, MPI_COMM_WORLD);	//vec_work is dummy here.
					}
					
				}
				if(can_break){
					//std::cout <<"Looks all processes are ready and have sent 99 code"<<std::endl;
					break;				
				}
				//std::cout << "Last line of while: counter is "<<counter<<std::endl;
			}				


		}	//end while
	}	//rank = 0
	else{		//worker nodes
		//std::cout<<"(worker) Executing receiving in " << rank <<std::endl;
		while (quit_code != -1){
			//int recv_vecsize = 2;
			int wip = 0;
			std::vector<int> vec_recv(vecsize);    
			MPI_Status s;
			MPI_Request request_recv_wkr;
			MPI_Irecv(&(vec_recv[0]), vecsize, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &request_recv_wkr);
			MPI_Wait(&request_recv_wkr,&s);
			
			//std::cout << "(worker) Received "<<s.MPI_TAG << " from master \n";
			if (s.MPI_TAG == 99 && wip == 0) {
				//std::cout << "(worker) Received 99 from master \n";
				quit_code = -1;
				break;
			}
			else{
				wip = 1;
				int fstart;
				int fend;
				for (int i=0; i< vec_recv.size(); ++i){
					fstart = vec_recv[i];
					fend = vec_recv[i] + (chunk);
					if (fend > n){
						fend = n;
					}
					//std::cout <<"(worker) fstart and fend are "<< fstart<<" "<<fend<<std::endl;
					loop_result = getnumint(functionid, a, b, n, intensity, mltp, fstart, fend);
					//std::cout <<"(worker) loop result " << loop_result<<" with TAG " <<s.MPI_TAG<<" and rank "<<rank<<std::endl;
					std::vector<float> vec_return(vecsize_return);
					vec_return[0] = loop_result;
					vec_return[1] = vec_recv[i];	//send which chunk is this result
					vec_return[2] = s.MPI_TAG;
					MPI_Request request_send_wkr;
					MPI_Isend(&(vec_return[0]), vecsize_return, MPI_FLOAT, 0, 456, MPI_COMM_WORLD,&request_send_wkr);   //Send back result to 0
					//std::cout<<"(worker) Sending result vector back to 0 using 456 tag"<<std::endl;
					MPI_Wait(&request_send_wkr,MPI_STATUS_IGNORE);
					//std::cout<<"(worker) after wait"<<std::endl;
					wip = 0;	
					if (fend == n){
						break;
					}
				}
			}
		}		//end while quit_code	
	}//worker nodes else


	std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = end-start;  

	if (rank == 0){
		//std::cout<<"In rank 0 again"<<std::endl;
		if (counter > n){
			//std::cout<<"All executions are complete.  Time to calculate final_value"<<std::endl;
			//printvector("Comm status",vec_comm_status);
			final_value = mltp * loop_result;
			std::cout << final_value <<std::endl;   
			std::cerr<<elapsed_seconds.count()<<std::endl;   
		}   
	}
  	MPI_Finalize();

  	return 0;
}

