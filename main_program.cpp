#include <pdi.h>
#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <cstdlib>
#include <ctime>
#define SIZE 5


using namespace std;
pthread_mutex_t mutex;

void* read(void*)
{
    while(1)
    {
        int input_s = 1;
        int matrix[SIZE][SIZE];
        cout << "[READER] Waiting before entering critical section" << endl;
        pthread_mutex_lock(&mutex);
        cout << "[READER] Entered critical section, reading data..." << endl;
        sleep(1);
        PDI_expose("input", &input_s, PDI_OUT); // set input to read
        PDI_expose("matrix_data", matrix, PDI_IN);
        pthread_mutex_unlock(&mutex);
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
    }
    return NULL;
}

void* write(void*)
{
    int input_s = 0;
    int matrix[SIZE][SIZE];
 
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            matrix[i][j] = 0;
        }
    }
    while (1)
    {
        for int i = 0; i < SIZE; i++)
        {
            for (int j = 0; j < SIZE; j++)
            {
                matrix[i][j]++;
            }
        }
        cout << "[WRITER] Waiting before entering critical section" << endl;
        pthread_mutex_lock (&mutex);
        cout << "[WRITER] Entered critical section, writing data..." << endl;
        sleep (5);
        PDI_expose ("input", &input_s, PDI_OUT); // set input to write
        PDI_expose ("matrix_data", matrix, PDI_OUT);
        pthread_mutex_unlock(&mutex);
        cout << "[WRITER] Data written, leaving critical section" << endl;
        int sleep_time = (( std::rand() % 8 ) + 3 ); //sleep for 3 - 10s
        sleep(sleep_time);
        cout << "[WRITER] Waiting for " << sleep_time << " seconds" << endl; 
    }
    return NULL;
}




int main (int argc, char* argv[])
{
    srand ( time( NULL ) );

    PDI_init (PC_parse_path("matrix_event.yml"));
    int status = pthread_mutex_init (&mutex, NULL); //checking if mutex was implemented correctly
    if (status != 0)
    {
        cerr << "Error with creating a mutex" << endl;
        return status; 
    }
    pthread_t writer;
    pthread_t reader;

    pthread_create (&writer, NULL, write, NULL); 
    pthread_create (&reader, NULL, read, NULL); 
    pthread_join (writer, NULL); //prevents for killing threads in the end of main function
    pthread_join (reader, NULL);
    pthread_mutex_destroy (&mutex);

    PDI_finalize ();
    return 0;
}
