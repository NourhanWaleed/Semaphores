#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <queue>

using namespace std;
#define N 5
int Counter;

deque<int> buffer;
sem_t mutex, int_mutx;
int mo_index = 0;
int co_index = 0;
[[noreturn]] void mMonitor_function(void *arg) {
    int random_sleep = rand() % 10+1;
    sleep(random_sleep);
    while (true) {
        cout << "Monitor thread: waiting to read counter\n";
        sem_wait(&int_mutx);
        int temp = Counter;
        cout << "Monitor thread: reading a count of value: " << temp << endl;
        Counter=0;
        sem_post(&int_mutx);
        sem_wait(&mutex);
        if (buffer.size() == N){
            cout << "Monitor thread: Buffer is full!!\n";
            buffer.pop_back();
            buffer.push_back(temp);
        }else {
            if(mo_index == N)
                mo_index = 0;
            buffer.push_front(temp);
            cout << "Monitor thread: writing to buffer at position " << mo_index++<< endl;
        }
        sem_post(&mutex);

        int random_sleep = rand() % 10 + 1;
        sleep(random_sleep);
    }
}

[[noreturn]] void mCollector_Function(void *arg) {
    int random_sleep = rand() % 10 +1;
    sleep(random_sleep);

    while(true){
        sem_wait(&mutex);
        if (buffer.empty())
            cout << "Collector thread: nothing is in the buffer! :( \n";
        else {
            if(co_index == N)
                co_index = 0;
            cout << "Collector thread: reading from buffer at position: " << co_index++<< " reads: " << buffer.front()
                 << endl;
            buffer.pop_front();
        }
        sem_post(&mutex);

        int random_sleep = (rand() % (30-20+1)) + 10;
        sleep(random_sleep);
    }

}
[[noreturn]] void mCounter_Function(void *arg) {
    int myid = *(int*)arg;
    int random_sleep = rand() % 10+1;
    sleep(random_sleep);
    while (true) {
        cout << "Counter thread " << myid<< ": received a message\n";
        sem_wait(&int_mutx);
        if (Counter != 0)
            cout << "Counter thread " << myid<< ": waiting to write\n";
        sleep(random_sleep);
        Counter ++;
        cout << "Counter thread " << myid<< ": now adding to counter,counter value=" << Counter << endl;
        sem_post(&int_mutx);

        int random_sleep = rand() % 10 +1;
        sleep(random_sleep);
    }
}

int main() {
    sem_init(&mutex, 0, 1);
    sem_init(&int_mutx, 0, 1);
    Counter = 0;
    pthread_t mCounter[N];
    pthread_t mMonitor, mCollector;
    int thread_ids[N];
    for (int i = 0; i < N; ++i){
        thread_ids[i]=i+1;
        pthread_create(&mCounter[i], NULL,
                       reinterpret_cast<void *(*)(void *)>(mCounter_Function),
                       &thread_ids[i] );
    }

    pthread_create(&mMonitor, NULL,
                   reinterpret_cast<void *(*)(void *)>(mMonitor_function),
                   NULL);
    pthread_create(&mCollector, NULL,
                   reinterpret_cast<void *(*)(void *)>(mCollector_Function),
                   NULL);
    for (int i = 0; i < N; ++i) {
        pthread_join(mCounter[i], NULL);
    }
    pthread_join(mMonitor, NULL);
    pthread_join(mCollector, NULL);

    sem_destroy(&mutex);
    return 0;
}
