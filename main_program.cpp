#include <pdi.h>
#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <cstdlib>
#include <ctime>
#include <csignal>
#define SIZE 5
bool work;


using namespace std;

pthread_mutex_t mutex_data;
pthread_mutex_t mutex_init_reader;
pthread_mutex_t mutex_signal_reader;
pthread_mutex_t mutex_signal_writer;

void mask_sig(void)
{
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask,SIGINT);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);
}

void* read(void*)
{
    mask_sig();
    pthread_mutex_lock (&mutex_init_reader);
    pthread_mutex_lock (&mutex_signal_reader); 
    while(work==true)
    {
        pthread_mutex_unlock (&mutex_signal_reader);
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

        for (int i = 0; i < SIZE; i++) //reading data
        {
            for (int j = 0; j < SIZE; j++)
            {
                cout << matrix[i][j] << " ";
            }
            cout << endl;
        }
        int sleep_time = (( std::rand() % 3 ) + 1 ); //sleep for 1 - 3s
        sleep(sleep_time);
        cout << "[READER] Waiting for " << sleep_time << " seconds" << endl;
        pthread_mutex_lock (&mutex_signal_reader); 
    }
    return NULL;
}

void* write(void*)
{
    mask_sig();
    int input_s = 0;
    int matrix[SIZE][SIZE];
 
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            matrix[i][j] = 0;
        }
    }

    PDI_expose ("input", &input_s, PDI_OUT);
    PDI_expose ("matrix_data", matrix, PDI_OUT);
    pthread_mutex_unlock (&mutex_init_reader);
    pthread_mutex_lock (&mutex_signal_writer); 

    while (work==true)
    {
        pthread_mutex_unlock (&mutex_signal_writer);
        for (int i = 0; i < SIZE; i++)
        {
            for (int j = 0; j < SIZE; j++)
            {
                matrix[i][j]++;
            }
        }
        cout << "[WRITER] Waiting before entering critical section" << endl;
        pthread_mutex_lock (&mutex_data);
        cout << "[WRITER] Entered critical section, writing data..." << endl;
        sleep (5);
        PDI_expose ("input", &input_s, PDI_OUT); // set input to write
        PDI_expose ("matrix_data", matrix, PDI_OUT);
        pthread_mutex_unlock(&mutex_data);
        cout << "[WRITER] Data written, leaving critical section" << endl;
        int sleep_time = (( std::rand() % 8 ) + 3 ); //sleep for 3 - 10s
        sleep(sleep_time);
        cout << "[WRITER] Waiting for " << sleep_time << " seconds" << endl;
        pthread_mutex_lock (&mutex_signal_writer); 
    }
    return NULL;
}

void signalHandler (int signum)
{
    pthread_mutex_lock (&mutex_signal_reader);
    pthread_mutex_lock (&mutex_signal_writer);
    work=false;
    pthread_mutex_unlock (&mutex_signal_reader);
    pthread_mutex_unlock (&mutex_signal_writer);
}


int main (int argc, char* argv[])
{
    work = true;
    srand ( time( NULL ) );
    signal(SIGINT, signalHandler);

    PDI_init (PC_parse_path("matrix_event.yml"));
    int status = pthread_mutex_init (&mutex_data, NULL); //checking if mutex was implemented correctly
    if (status != 0)
    {
        cerr << "Error with creating a mutex" << endl;
        return status; 
    }
    
    int status_reader = pthread_mutex_init (&mutex_init_reader, NULL);   
    if (status_reader != 0)
    {
        cerr << "Error with creating a mutex" << endl;
        return status; 
    }
    
    pthread_mutex_lock (&mutex_init_reader);

    pthread_t writer;
    pthread_t reader;

    pthread_create (&writer, NULL, write, NULL); 
    pthread_create (&reader, NULL, read, NULL); 
    pthread_join (writer, NULL); //prevents for killing threads in the end of main function
    pthread_join (reader, NULL);
    pthread_mutex_destroy (&mutex_data);
    pthread_mutex_destroy (&mutex_init_reader);
    pthread_mutex_destroy (&mutex_signal_reader);
    pthread_mutex_destroy (&mutex_signal_writer);

    PDI_finalize ();
    return 0;
}
