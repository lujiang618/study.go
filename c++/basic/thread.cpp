#include <atomic>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

std::queue<int>         buffer; // 共享缓冲区
std::mutex              mtx;    // 用于同步对缓冲区的访问
std::condition_variable cv;     // 条件变量
std::atomic<bool>       stop;

void producer()
{
    for (int i = 0; i < 5; ++i)
    {
        if (stop)
        {
            std::cout << "producer exit" << std::endl;
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 模拟生产过程
        std::lock_guard<std::mutex> lock(mtx);                       // 锁定互斥量
        buffer.push(i);
        std::cout << "Produced: " << i << std::endl;
        cv.notify_one(); // 通知消费者线程

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

void consumer()
{
    for (int i = 0; i < 5; ++i)
    {
        if (stop)
        {
            std::cout << "consumer exit" << std::endl;
            break;
        }

        std::unique_lock<std::mutex> lock(mtx); // 使用 unique_lock
        cv.wait(lock, []
        {
            return !buffer.empty();
        }); // 等待缓冲区有数据

        int item = buffer.front(); // 获取缓冲区中的数据
        buffer.pop();
        std::cout << "Consumed: " << item << std::endl;
    }
}

void init()
{
    std::thread producerThread(producer);
    std::thread consumerThread(consumer);

    // producerThread.detach();
    // consumerThread.detach();

    producerThread.join();
    consumerThread.join();
}

void toStop()
{
    stop = true;
    cv.notify_all();
}

int main()
{
    init();

    std::cout << "inited....." << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(1));

    toStop();

    std::this_thread::sleep_for(std::chrono::seconds(5));

    return 0;
}
