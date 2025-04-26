#include <bits/stdc++.h>
#include <functional>
#include <chrono>
#include <future>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace std;

using ll = long long int;

class Timeout {
private:
    using Task = pair<chrono::steady_clock::time_point, function<void()>>;
    struct TaskCompare {
        bool operator()(const Task& a, const Task& b) const {
            return a.first > b.first; // Earliest time first
        }
    };
    priority_queue<Task, vector<Task>, TaskCompare> tasks;
    mutex mtx;
    condition_variable cv;
    bool stop = false;
    thread timerThread;

    void timerLoop() {
        unique_lock<mutex> lock(mtx);
        while (!stop) {
            if (tasks.empty()) {
                cv.wait(lock);
            } else {
                auto nextTime = tasks.top().first;
                if (cv.wait_until(lock, nextTime) == cv_status::timeout) {
                    while (!tasks.empty() && tasks.top().first <= chrono::steady_clock::now()) {
                        auto task = tasks.top();
                        tasks.pop();
                        lock.unlock();
                        task.second();
                        lock.lock();
                    }
                }
            }
        }
    }

public:
    Timeout() {
        timerThread = thread([this] { timerLoop(); });
    }

    ~Timeout() {
        {
            unique_lock<mutex> lock(mtx);
            stop = true;
            cv.notify_all();
        }
        timerThread.join();
    }

    void setTimeout(function<void()> func, ll timeoutMs) {
        auto execTime = chrono::steady_clock::now() + chrono::milliseconds(timeoutMs);
        {
            unique_lock<mutex> lock(mtx);
            tasks.emplace(execTime, func);
            cv.notify_all();
        }
    }
};

int main() {
    Timeout tobj;
    tobj.setTimeout([](){
        cout << "hello world" << "\n";
    }, 1000);

    tobj.setTimeout([](){
        cout << "hello world" << "\n";
    }, 1000);

    tobj.setTimeout([](){
        cout << "hello world" << "\n";
    }, 1000);

    cout << "Main ends" << "\n";

    this_thread::sleep_for(chrono::milliseconds(3000));

    return 0;
}