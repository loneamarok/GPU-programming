/*
 DP with conditional variables created from scratch - Not using the cond_variable provided by the OS
 */

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace std;

#define N 5
#define N_EAT 3

// Based on permits, determine if permit can be given
void get_permit(int *permits, mutex *permit_lock, condition_variable_any *cv) {
    permit_lock->lock();
    while(*permits == 0)
    {   
        // unlocks the permit_lock and waits for notification
        cv->wait(*permit_lock);
    }
    *permits--;
    permit_lock->unlock();
}

// Release the permit after use
void release_permit(int *permits, mutex *permit_lock, condition_variable_any *cv) {
    permit_lock->lock();
    *permits++;
    cv->notify_all();
    permit_lock->unlock();
}

class Philisopher {
    int id;
    mutex *left;
    mutex *right;
    condition_variable_any *cv;

public:
    void init(int i, mutex* _left, mutex* _right, condition_variable_any *_cv) {
        this->id = i;
        this->left = _left;
        this->right = _right;
        this->cv = _cv;
    }

    // Philisopher thinks for a random time (1-5 seconds)
    void think() {
        cout << "Philosopher " << this->id << " is thinking" << endl;
        this_thread::sleep_for(chrono::seconds(rand() % 5 + 1));
        cout << "Philisopher " << this->id << " has finished thinking" << endl;
    }

    // 1. Philisopher gets permit (Conditional variable to avoid deadlock)
    // 2. Philisopher gets the left and right forks
    // 3. Philisopher eats for a random time (1-5 seconds)
    // 4. Philisopher releases the left and right forks
    // 5. Philisopher releases the permit
    void eat(int *permits, mutex* permit_lock, condition_variable_any *cv) {
        get_permit(permits, permit_lock, cv);
        left->lock();
        right->lock();
        cout << "Philosopher " << this->id << " is eating" << endl;
        this_thread::sleep_for(chrono::seconds(rand() % 5 + 1));
        cout << "Philosopher " << this->id << " has finished eating" << endl;
        left->unlock();
        right->unlock();
        release_permit(permits, permit_lock, cv);
    }

    void dine(int *permits, mutex* permit_lock, condition_variable_any *cv) {
        for(int i = 0; i < N_EAT; i++) {
            think();
            eat(permits, permit_lock, cv);
        }
    }
};

int main() 
{
    int permits = N - 1;
    mutex permit_lock;
    mutex forks[N];
    Philisopher philisophers[N];
    thread philisopher_threads[N];
    condition_variable_any cv;

    // Initialize the left and right forks for each philisopher
    for(int i = 0; i < N; i++) {
        philisophers[i].init(i, &forks[i], &forks[(i + 1) % N], &cv);
    }

    // Start the philisopher threads
    for(int i = 0; i < N; i++) {
        philisopher_threads[i] = thread(&Philisopher::dine, &philisophers[i], &permits, &permit_lock, &cv);
    }

    // Join the philisopher threads
    for(int i = 0; i < N; i++) {
        philisopher_threads[i].join();
    }
}


