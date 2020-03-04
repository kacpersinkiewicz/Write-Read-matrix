#include <pdi.h>
#include <iostream>
#include <unistd.h> //sleep
#include <stdlib.h>
#include <pthread.h>
#include <ctime>
#include <csignal>
#define SIZE 5
bool work;

using namespace std;

pthread_mutex_t mutex_data; //main mutex, protect access to matrix
pthread_mutex_t mutex_init_reader; //used to write initial miatrix before reader thread will begin  
pthread_mutex_t mutex_signal_reader; //used to safe changing data by ctrl+c signal for reader
pthread_mutex_t mutex_signal_writer; //used to safe changing data by ctrl+c signal for writer

void mask_sig(void) // function allowing to mask threads
{
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask,SIGINT);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);
}

void* read(void*) // reader function
{
    mask_sig();
    pthread_mutex_lock(&mutex_init_reader);
    pthread_mutex_lock(&mutex_signal_reader); 
    while(work == true)
    {
        pthread_mutex_unlock(&mutex_signal_reader);
        int input_s = 1;
        int matrix[SIZE][SIZE];
        cout << "[READER] Waiting before entering critical section" << endl;
        pthread_mutex_lock(&mutex_data);
        cout << "[READER] Entered critical section, reading data..." << endl;
        sleep(1);
        PDI_expose("input", &input_s, PDI_OUT); // set input to read
        PDI_expose("matrix_data", matrix, PDI_IN);
        pthread_mutex_unlock(&mutex_data);
        cout << "[READER] Data read, leaving critical section" << endl;

        for(int i = 0; i < SIZE; i++) //reading data
        {
            for(int j = 0; j < SIZE; j++)
            {
                cout << matrix[i][j] << " ";
            }
            cout << endl;
        }
        int sleep_time = (( std::rand() % 3 ) + 1 ); //sleep for 1 - 3s
        sleep(sleep_time);
        cout << "[READER] Waiting for " << sleep_time << " seconds" << endl;
        pthread_mutex_lock(&mutex_signal_reader); 
    }
    cout << "[READER] Read proces ended." << endl;
    return NULL;
}

void* write(void*)
{
    mask_sig();
    int input_s = 0;
    int matrix[SIZE][SIZE];
 
    for(int i = 0; i < SIZE; i++)
    {
        for(int j = 0; j < SIZE; j++)
        {
            matrix[i][j] = 0;
        }
    }

    PDI_expose("input", &input_s, PDI_OUT);
    PDI_expose("matrix_data", matrix, PDI_OUT);
    pthread_mutex_unlock(&mutex_init_reader);
    pthread_mutex_lock(&mutex_signal_writer); 

    while(work == true)
    {
        pthread_mutex_unlock(&mutex_signal_writer);
        for(int i = 0; i < SIZE; i++)
        {
            for(int j = 0; j < SIZE; j++)
            {
                matrix[i][j]++;
            }
        }
        cout << "[WRITER] Waiting before entering critical section" << endl;
        pthread_mutex_lock(&mutex_data);
        cout << "[WRITER] Entered critical section, writing data..." << endl;
        sleep(5);
        PDI_expose("input", &input_s, PDI_OUT); // set input to write
        PDI_expose("matrix_data", matrix, PDI_OUT);
        pthread_mutex_unlock(&mutex_data);
        cout << "[WRITER] Data written, leaving critical section" << endl;
        int sleep_time = (( std::rand() % 8 ) + 3 ); //sleep for 3 - 10s
        sleep(sleep_time);
        cout << "[WRITER] Waiting for " << sleep_time << " seconds" << endl;
        pthread_mutex_lock(&mutex_signal_writer); 
    }
    cout << "[WRITER] Write proces ended." << endl;
    return NULL;
}

void signalHandler(int signum) // function used when ctrl+c is pressed
{
    pthread_mutex_lock(&mutex_signal_reader);
    pthread_mutex_lock(&mutex_signal_writer);
    work = false;
    cout << "[SIGNAL] Program OFF, ending threads" << endl;
    pthread_mutex_unlock(&mutex_signal_reader);
    pthread_mutex_unlock(&mutex_signal_writer);
}


int main(int argc, char* argv[])
{
    work = true;
    srand( time( NULL ) );
    signal(SIGINT, signalHandler); //catching ctrl+c signal and running signalHandler function

    PDI_init(PC_parse_path("matrix_event.yml"));

    int status = pthread_mutex_init(&mutex_data, NULL); //implementing and checking if mutex was implemented correctly
    if(status != 0)
    {
        cerr << "Error with creating a mutex_data" << endl;
        return status; 
    }
    status = pthread_mutex_init(&mutex_init_reader, NULL);   
    if(status != 0)
    {
        cerr << "Error with creating a mutex_init_reader" << endl;
        return status; 
    }
    
    status = pthread_mutex_init(&mutex_signal_reader, NULL);   
    if(status != 0)
    {
        cerr << "Error with creating a mutex_signal_reader" << endl;
        return status; 
    }
    status = pthread_mutex_init(&mutex_signal_writer, NULL);   
    if(status != 0)
    {
        cerr << "Error with creating a mutex_signal_writer" << endl;
        return status; 
    }

    pthread_mutex_lock(&mutex_init_reader);

    pthread_t writer;
    pthread_t reader;

    pthread_create(&writer, NULL, write, NULL); 
    pthread_create(&reader, NULL, read, NULL); 
    pthread_join(writer, NULL); //prevents for killing threads in the end of main function
    pthread_join(reader, NULL);
    pthread_mutex_destroy(&mutex_data);
    pthread_mutex_destroy(&mutex_init_reader);
    pthread_mutex_destroy(&mutex_signal_reader);
    pthread_mutex_destroy(&mutex_signal_writer);

    PDI_finalize();
    return 0;
}
