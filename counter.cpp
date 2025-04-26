#include <iostream>
#include <thread>
#include <vector>
#include <map>
#include <atomic>
#include <cassert>
#include <string>

// Mock Helper06 class for logging
class Helper06 {
public:
    void log(const std::string& message) {
        std::cout << "[LOG] " << message << std::endl;
    }
};

// Your original Counter class
class Counter {
private:
    int size;
    std::map<int, std::atomic<int>> data;
public:
    Counter(int _size, Helper06 helper) : size(_size) {
        helper.log("Initialized Counter with " + std::to_string(_size) + " pages");
    }
    void incrementVisitCount(int pageIndex) {
        data[pageIndex].fetch_add(1);
    }
    int getVisitCount(int pageIndex) {
        return data[pageIndex].load();
    }
};

// Test function to validate the Counter class
void runTests() {
    Helper06 helper;

    // Test 1: Basic Increment and Get
    std::cout << "\nTest 1: Basic Increment and Get\n";
    {
        Counter counter(2, helper);
        counter.incrementVisitCount(0);
        counter.incrementVisitCount(0);
        counter.incrementVisitCount(1);
        counter.incrementVisitCount(1);
        counter.incrementVisitCount(1);

        int count0 = counter.getVisitCount(0);
        int count1 = counter.getVisitCount(1);
        int count2 = counter.getVisitCount(2); // Unvisited page

        assert(count0 == 2 && "Page 0 should have 2 visits");
        assert(count1 == 3 && "Page 1 should have 3 visits");
        assert(count2 == 0 && "Page 2 should have 0 visits");
        std::cout << "Test 1 Passed: Page 0 = " << count0 << ", Page 1 = " << count1 << ", Page 2 = " << count2 << std::endl;
    }

    // Test 2: Increment Out-of-Bounds
    std::cout << "\nTest 2: Increment Out-of-Bounds\n";
    {
        Counter counter(1000, helper);
        counter.incrementVisitCount(999);  // Max valid page
        counter.incrementVisitCount(1000); // Just beyond max
        counter.incrementVisitCount(-1);   // Negative index

        int count999 = counter.getVisitCount(999);
        int count1000 = counter.getVisitCount(1000);
        int countNeg1 = counter.getVisitCount(-1);

        assert(count999 == 1 && "Page 999 should have 1 visit");
        // Note: Your solution creates new map entries for invalid indices
        std::cout << "Test 2 Result: Page 999 = " << count999
                  << ", Page 1000 = " << count1000
                  << ", Page -1 = " << countNeg1
                  << " (Note: Out-of-bounds indices create new entries)" << std::endl;
    }

    // Test 3: Multi-Threaded Increments
    std::cout << "\nTest 3: Multi-Threaded Increments\n";
    {
        Counter counter(2, helper);
        const int num_threads = 10;
        const int increments_per_thread = 100;
        std::vector<std::thread> threads;

        // Launch threads to increment page 0
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back([&counter, increments_per_thread]() {
                for (int j = 0; j < increments_per_thread; ++j) {
                    counter.incrementVisitCount(0);
                }
            });
        }

        // Join all threads
        for (auto& t : threads) {
            t.join();
        }

        int expected = num_threads * increments_per_thread;
        int count0 = counter.getVisitCount(0);
        assert(count0 == expected && "Page 0 should have correct visit count after concurrent increments");
        std::cout << "Test 3 Passed: Page 0 = " << count0 << " (Expected " << expected << ")" << std::endl;
    }

    // Test 4: Concurrent Increments on Multiple Pages
    std::cout << "\nTest 4: Concurrent Increments on Multiple Pages\n";
    {
        Counter counter(5, helper);
        const int num_pages = 5;
        const int num_threads = 10;
        const int increments_per_thread = 50;
        std::vector<std::thread> threads;

        // Launch threads to increment different pages
        for (int page = 0; page < num_pages; ++page) {
            for (int i = 0; i < num_threads; ++i) {
                threads.emplace_back([&counter, page, increments_per_thread]() {
                    for (int j = 0; j < increments_per_thread; ++j) {
                        counter.incrementVisitCount(page);
                    }
                });
            }
        }

        // Join all threads
        for (auto& t : threads) {
            t.join();
        }

        // Verify counts
        bool all_passed = true;
        for (int page = 0; page < num_pages; ++page) {
            int expected = num_threads * increments_per_thread;
            int count = counter.getVisitCount(page);
            if (count != expected) {
                all_passed = false;
                std::cout << "Page " << page << " = " << count << " (Expected " << expected << ")" << std::endl;
            }
        }
        assert(all_passed && "All pages should have correct visit counts");
        std::cout << "Test 4 Passed: All pages have " << (num_threads * increments_per_thread) << " visits" << std::endl;
    }

    // Test 5: Stress Test with Max Pages
    std::cout << "\nTest 5: Stress Test with Max Pages\n";
    {
        Counter counter(1000, helper);
        const int increments = 100;

        // Increment all pages
        for (int page = 0; page < 1000; ++page) {
            for (int i = 0; i < increments; ++i) {
                counter.incrementVisitCount(page);
            }
        }

        // Verify counts
        bool all_passed = true;
        for (int page = 0; page < 1000; ++page) {
            int count = counter.getVisitCount(page);
            if (count != increments) {
                all_passed = false;
                std::cout << "Page " << page << " = " << count << " (Expected " << increments << ")" << std::endl;
            }
        }
        assert(all_passed && "All pages should have correct visit counts");
        std::cout << "Test 5 Passed: All 1000 pages have " << increments << " visits" << std::endl;
    }

    std::cout << "\nAll Tests Completed Successfully!\n";
}

int main() {
    runTests();
    return 0;
}