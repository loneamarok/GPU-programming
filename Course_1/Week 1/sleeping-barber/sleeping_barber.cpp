// Sleeping barber problem with multiple barbers and a waiting room

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <semaphore>
#include <queue>

#define WAITING_ROOM_SIZE 15
#define N_BARBERS 3

// Waiting room related structures
std::queue<void*> waiting_room;
std::mutex waiting_room_lock;
std::condition_variable_any barber_cv;
int earnings;
std::mutex earnings_lock;

class Customer{
public:
    int id;
    int money;
    Customer(int id, int money) {
        this->id = id;
        this->money = money;
    }

    void reach_barber_shop() {
        std::cout << "Customer " << this->id << " has arrived" << std::endl;
        // If waiting room is full, leave
        waiting_room_lock.lock();
        if(waiting_room.size() == WAITING_ROOM_SIZE)
        {
            std::cout << "Customer " << this->id << " is leaving" << std::endl;
        }
        // If not, update the queue and insert yourself in it
        else
        {
            waiting_room.push(this);
            std::cout << "Customer " << this->id << " is waiting at position - " << waiting_room.size() << std::endl;
            // If queue size is now 1, wake up the barbers
            if(waiting_room.size() == 1)
            {
                barber_cv.notify_one();
            }
        }
        waiting_room_lock.unlock();
    }
};

class Barber {
    int pace;
public:
    int id;
    Barber(int pace, int id) {
        this->pace = pace;
        this->id = id;
    }

    void cut_hair() {
        std::cout << "Barber " << id << " is cutting hair" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(pace));
        std::cout << "Barber " << id << " has finished cutting hair" << std::endl;
    }

    void work() {
        while(true)
        {
            // Check the waiting room - If empty, sleep
            waiting_room_lock.lock();
            if(waiting_room.size() == 0)
            {
                std::cout << "Barber " << id << " is sleeping" << std::endl;
                barber_cv.wait(waiting_room_lock);
                std::cout << "Barber " << id << " was called for duty" << std::endl;
            }
            // Get customer from the top of the queue
            Customer *customer = (Customer*)waiting_room.front();
            waiting_room.pop();
            // Print customer details
            std::cout << "Barber " << id << " is cutting hair of customer " << customer->id << std::endl;
            waiting_room_lock.unlock();
            // Cut hair
            cut_hair();
            // Update and print earnings
            earnings_lock.lock();
            earnings += customer->money;
            std::cout << "Barbers have earned $" << earnings << " so far" << std::endl;
            earnings_lock.unlock();
            // Delete customer
            delete customer;
        }
    }
};

int main()
{
    //Create barbers
    Barber *barbers[N_BARBERS];

    for(int i = 0; i < N_BARBERS; i++)
    {
        int pace = 10;
        barbers[i] = new Barber(pace, i+1);
    }

    // Create threads for barbers
    std::thread barber_threads[N_BARBERS];
    for(int i = 0; i < N_BARBERS; i++)
    {
        barber_threads[i] = std::thread(&Barber::work, barbers[i]);
    }

    // Create threads for customers
    std::thread customer_threads[50];

    // Create 50 customers at random intervals
    for (int i = 0; i < 50; i++)
    {
        std::this_thread::sleep_for(std::chrono::seconds(rand() % 3 + 1));
        int money = rand() % 100 + 1;
        Customer *customer = new Customer(i+1, money);
        // Create a thread for each customer
        customer_threads[i] = std::thread(&Customer::reach_barber_shop, customer);
    }

    // Join the barber threads
    for(int i = 0; i < N_BARBERS; i++)
    {
        barber_threads[i].join();
    }

    // Join the customer threads
    for(int i = 0; i < 50; i++)
    {
        customer_threads[i].join();
    }
}