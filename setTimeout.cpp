#include <bits/stdc++.h>
#include <functional>
#include <chrono>
#include <future>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace std;

using ll = long long int;

class Threadpool {
private:
    int m_threads;
    vector<thread> threads;
    queue<function<void()>> tasks;
    mutex mtx;
    condition_variable cv;
    bool stop; 
public:

    explicit Threadpool(int numThreads) : m_threads(numThreads), stop(false) {
        for(int i=0;i<m_threads;i++){
            threads.emplace_back([this](){
                function<void()> task;
                while(1){
                    unique_lock<mutex> lock(mtx);
                    cv.wait(lock, [this]() {
                        return !tasks.empty()  || stop;
                    });

                    if(stop) {
                        return;
                    }

                    task = move(tasks.front());
                    tasks.pop();

                    lock.unlock();
                    task();
                }
            });
        }
    }

    ~Threadpool(){
        unique_lock<mutex> lock(mtx);
        stop = true;
        lock.unlock();
        cv.notify_all();

        for(auto &th : threads){
            th.join();
        }
    }

    template<class F,class... Args>
    auto executeTask(F&& f, Args&&... args) -> future<decltype(f(args...))>{
        
        using return_type = decltype(f(args...));

        auto task = make_shared<packaged_task<return_type()>>(bind(forward<F>(f),forward<Args>(args)...));
        
        future<return_type> res = task->get_future();
        
        unique_lock<mutex> lock(mtx);
        tasks.emplace([task]() -> void{
            (*task)();
        });
        lock.unlock();

        cv.notify_one();

        return res;
    }
};


class Timeout {
private:
    vector<pair<ll,function<void()>>> data;
    mutex mtx;
    condition_variable cv;
    Threadpool *pool;

public:
    Timeout(){
        pool = new Threadpool(8);
        pool->executeTask([this](){
            while(1){
                unique_lock<mutex> lock(mtx);
                cv.wait(lock,[this](){
                    return !data.empty();
                });

                vector<pair<ll,function<void()>>> temp;
                if(!data.empty()){
                    swap(temp,data);
                    for(auto p : temp){
                        pool->executeTask([this, p]() {
                            mutex mtx2;
                            unique_lock<mutex> lock(mtx2);
                            auto status = cv.wait_for(lock,chrono::milliseconds(p.first));
                            if (status == cv_status::timeout){
                                (p.second)();
                            }
                        });
                    }
                }
            }
        });
    }

    void setTimeout(function<void()> func,ll timeoutTime){
        unique_lock<mutex> lock(mtx);
        data.push_back(make_pair(timeoutTime,func));
        lock.unlock();
        cv.notify_one();
    }
};

int main() {

    Timeout *tobj = new Timeout();
    tobj->setTimeout([](){
        cout << "hello world" << "\n";
    },1000);

    tobj->setTimeout([](){
        cout << "hello world" << "\n";
    },1000);

    tobj->setTimeout([](){
        cout << "hello world" << "\n";
    },1000);

    cout << "Main ends" << "\n";

    // Threadpool pool(8);
    // pool.executeTask([](){
    //     cout << "hello world";
    // });

    this_thread::sleep_for(chrono::milliseconds(3000));

    return 0;
}