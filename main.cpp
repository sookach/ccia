#include <condition_variable>
#include <iostream>
#include <memory>
#include <queue>
#include <shared_mutex>
#include <thread>

#include "Vector.h"

namespace threadsafe {
template <typename T> class Queue {
  std::queue<T> TheQueue;
  std::condition_variable CV;
  std::shared_mutex TheMutex;

public:
  Queue() = default;
  Queue(const Queue &Other) {
    std::shared_lock Lock(Other.TheMutex);
    TheQueue = Other.TheQueue;
  }

  auto size() const {
    std::shared_lock Lock(TheMutex);
    return std::size(TheQueue);
  }

  bool empty() const {
    std::shared_lock Lock(TheMutex);
    return std::empty(TheQueue);
  }

  void push(const T &Value) {
    std::lock_guard Lock(TheMutex);
    TheQueue.push(Value);
    CV.notify_one();
  }

  void wait_and_pop(T &Value) {
    std::unique_lock Lock(TheMutex);
    CV.wait(Lock, [this] { return !std::empty(TheQueue); });
    Value = TheQueue.front();
    TheQueue.pop();
  }

  std::shared_ptr<T> wait_and_pop() {
    std::unique_lock Lock(TheMutex);
    CV.wait(Lock, [this] { return !std::empty(TheQueue); });
    std::shared_ptr Value(TheQueue.front());
    TheQueue.pop();
    return Value;
  }

  bool try_pop(T &Value) {
    std::lock_guard Lock(TheMutex);
    if (std::empty(TheQueue))
      return false;
    Value = TheQueue.front();
    TheQueue.pop();
    return true;
  }

  std::shared_ptr<T> try_pop() {
    std::lock_guard Lock(TheMutex);
    if (std::empty(TheQueue))
      return nullptr;
    std::shared_ptr Value(TheQueue.front());
    TheQueue.pop();
    return true;
  }
};
} // namespace threadsafe

int main() { threadsafe::Vector<int> V; }