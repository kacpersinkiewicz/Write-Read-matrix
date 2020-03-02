#include <pdi.h>
#include<iostream>
#include<unistd.h>
#include<pthread.h>
#include <cstdlib>
#include <ctime>
#define SIZE 5


using namespace std;
pthread_mutex_t mutex;

void* read(void*)
{
    while(1)
    {
        int zzz;
        int input_s=1;
        int matrix[SIZE][SIZE];
        cout << "r reader waiting for data acces" << endl;
        pthread_mutex_lock(&mutex);
        cout << "r acces granted" << endl;
        sleep(1);
        PDI_expose("input", &input_s, PDI_OUT);
        PDI_expose("matrix_data", matrix, PDI_IN);
        pthread_mutex_unlock(&mutex);
        cout << "r data unlocked" << endl;

        for (int i = 0; i <5; i++)
        {
            for (int j = 0; j < 5; j++)
            {
                cout << matrix[i][j] << " ";
            }
            cout << endl;
        }
        zzz = (( std::rand() % 3 ) + 1 );
        sleep(zzz);
        cout << "r i was asleep for " << zzz << " seconds" << endl; 
    }
    return NULL;
}

void* write(void*)
{
    int zzz;
    int input_s=0;
    int matrix[SIZE][SIZE];
 
    for (int i = 0; i<SIZE; i++)
    {
        for(int j=0; j<SIZE; j++)
        {
            matrix[i][j]=0;
        }
    }
    while(1)
    {
        for (int l = 0; l<SIZE; l++)
        {
            for(int o=0; o<SIZE; o++)
            {
                matrix[l][o]++;
            }
        }
        cout << "w writer waiting for data acces" << endl;
        pthread_mutex_lock(&mutex);
        cout << "w acces granted" << endl;
        sleep(5);
        PDI_expose("input", &input_s, PDI_OUT);
        PDI_expose("matrix_data", matrix, PDI_OUT);
        pthread_mutex_unlock(&mutex);
        cout << "w data unlocked" << endl;
        zzz = (( std::rand() % 8 ) + 3 );
        sleep(zzz);
        cout << "w i was asleep for " << zzz << " seconds" << endl; 
    }
    return NULL;
}




int main(int argc, char* argv[])
{
    srand( time( NULL ) );

    PDI_init(PC_parse_path("write.yml"));
    int status = pthread_mutex_init(&mutex,NULL);
    if (status!=0)
    {
        cerr << "Error with creating a mutex" << endl;
        return status; 
    }
    pthread_t writer;
    pthread_t reader;

    pthread_create(&writer, NULL, write, NULL); 
    pthread_create(&reader, NULL, read, NULL); 
    pthread_join(writer, NULL);
    pthread_join(reader, NULL);
    pthread_mutex_destroy(&mutex);

    PDI_finalize();
    return 0;
}