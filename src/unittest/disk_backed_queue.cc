#include <queue>

#include "unittest/gtest.hpp"

#include "concurrency/coro_pool.hpp"
#include "concurrency/queue/disk_backed_queue_wrapper.hpp"
#include "containers/disk_backed_queue.hpp"
#include "arch/runtime/starter.hpp"
#include "arch/timing.hpp"

namespace unittest {

void run_many_ints_test() {
    static const int NUM_ELTS_IN_QUEUE = 1000;
    disk_backed_queue_t<int> queue("test");
    std::queue<int> ref_queue;

    for (int i = 0; i < NUM_ELTS_IN_QUEUE; ++i) {
        queue.push(i);
        ref_queue.push(i);
    }

    for (int i = 0; i < NUM_ELTS_IN_QUEUE; ++i) {
        EXPECT_FALSE(queue.empty());
        EXPECT_EQ(queue.pop(), ref_queue.front());
        ref_queue.pop();
    }
}

TEST(DiskBackedQueue, ManyInts) {
    run_in_thread_pool(&run_many_ints_test, 2);
}

void run_big_values_test() {
    static const int NUM_BIG_ELTS_IN_QUEUE = 100;
    disk_backed_queue_t<std::string> queue("test");
    std::queue<std::string> ref_queue;

    std::string val;
    val.resize(MEGABYTE, 'a');
    for (int i = 0; i < NUM_BIG_ELTS_IN_QUEUE; ++i) {
        queue.push(val);
        ref_queue.push(val);
    }

    for (int i = 0; i < NUM_BIG_ELTS_IN_QUEUE; ++i) {
        EXPECT_FALSE(queue.empty());
        EXPECT_EQ(queue.pop(), ref_queue.front());
        ref_queue.pop();
    }
}

TEST(DiskBackedQueue, BigVals) {
    run_in_thread_pool(&run_big_values_test, 2);
}

static void randomly_delay(int, signal_t *) {
    nap(randint(100));
}

void run_concurrent_test() {
    disk_backed_queue_wrapper_t<int> queue("test");
    coro_pool_t<int>::boost_function_callback_t callback(&randomly_delay);
    coro_pool_t<int> coro_pool(10, &queue, &callback);
    for (int i = 0; i < 1000; i++) {
        queue.push(i);
        nap(randint(10));
    }
    nap(1000);
}

TEST(DiskBackedQueue, Concurrent) {
    run_in_thread_pool(&run_concurrent_test, 1);
}

} //namespace unittest