#pragma once

#include <condition_variable>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <vector>

namespace threadsafe {
template <typename T, typename Allocator = std::allocator<T>> class Vector {
private:
  using BaseTy = std::vector<T, Allocator>;

public:
  using value_type = BaseTy::value_type;
  using allocator_type = BaseTy::allocator_type;
  using size_type = BaseTy::size_type;
  using difference_type = BaseTy::difference_type;
  using reference = BaseTy::reference;
  using const_reference = BaseTy::const_reference;
  using pointer = BaseTy::pointer;
  using const_pointer = BaseTy::const_pointer;
  using iterator = BaseTy::iterator;
  using const_iterator = BaseTy::const_iterator;
  using reverse_iterator = BaseTy::reverse_iterator;
  using const_reverse_iterator = BaseTy::const_reverse_iterator;

private:
  BaseTy TheVector;
  std::shared_mutex TheMutex;
  std::condition_variable TheCV;

public:
  bool empty() const {
    std::shared_lock Lock(TheMutex);
    return std::empty(TheVector);
  }

  size_type size() const {
    std::shared_lock Lock(TheMutex);
    return std::size(TheVector);
  }

  size_type max_size() const {
    std::shared_lock Lock(TheMutex);
    return TheVector.max_size();
  }

  void reserve(size_type Capacity) {
    std::lock_guard Lock(TheMutex);
    TheVector.reserve(Capacity);
  }

  size_type capacity() const {
    std::shared_lock Lock(TheMutex);
    return TheVector.capacity();
  }

  void clear() const {
    std::lock_guard Lock(TheMutex);
    TheVector.clear();
  }

  std::shared_mutex &mutex() { return TheMutex; }

  bool try_pop_back(reference Value) {
    std::lock_guard Lock(TheMutex);
    if (std::empty(TheVector))
      return false;
    Value = TheVector.back();
    return true;
  }

  std::unique_ptr<value_type> try_pop_back() {
    std::lock_guard Lock(TheMutex);
    if (std::empty(TheVector))
      return false;

    if constexpr (std::is_move_constructible_v<value_type>) {
      auto Value = std::make_unique(std::move(TheVector.back()));
      TheVector.pop_back();
      return Value;
    } else {
      auto Value = std::make_unique(TheVector.back());
      TheVector.pop_back();
      return Value;
    }
  }

  void wait_and_pop_back(reference Value) {
    std::unique_lock Lock(TheMutex);
    TheCV.wait(Lock, [this] { return !std::empty(TheVector); });
    if constexpr (std::is_move_assignable_v<T>)
      Value = std::move(TheVector.back());
    else
      Value = TheVector.back();
    TheVector.pop_back();
  }

  size_type try_pop(size_type Count, BaseTy &Value) {
    std::lock_guard Lock(TheMutex);
    size_type N = std::min(Count, std::size(TheVector));

    Value.clear();
    std::move(std::end(TheVector) - N, std::end(TheVector),
              std::back_inserter(Value));
    TheVector.resize(std::size(TheVector) - N);

    return std::size(Value);
  }

  std::unique_ptr<BaseTy> try_pop(size_type Count) {
    std::lock_guard Lock(TheMutex);
    size_type N = std::min(Count, std::size(TheVector));
    std::unique_ptr<BaseTy> Value(std::move_iterator(std::end(TheVector) - N),
                                  std::move_iterator(std::end(TheVector)));

    TheVector.resize(std::size(TheVector) - N);

    return Value;
  }

  bool try_pop_require(size_type Count, BaseTy &Value) {
    std::lock_guard Lock(TheMutex);

    if (std::size(TheVector) < Count)
      return false;

    Value.clear();
    size_type N = std::min(Count, std::size(TheVector));
    std::move(std::end(TheVector) - N, std::end(TheVector),
              std::back_inserter(Value));
    TheVector.resize(std::size(TheVector) - N);

    return true;
  }

  std::unique_ptr<BaseTy> try_pop_require(size_type Count) {
    std::lock_guard Lock(TheMutex);

    if (std::size(TheVector) < Count)
      return nullptr;

    size_type N = std::min(Count, std::size(TheVector));
    std::unique_ptr<BaseTy> Value(std::move_iterator(std::end(TheVector) - N),
                                  std::move_iterator(std::end(TheVector)));

    TheVector.resize(std::size(TheVector) - N);

    return Value;
  }

  void push_back(const T &Value) {
    std::lock_guard Lock(TheMutex);
    TheVector.push_back(Value);
  }

  void push_back(T &&Value) {
    std::lock_guard Lock(TheMutex);
    TheVector.push_back(std::forward<T>(Value));
  }

  template <typename... ArgsTy> void push(ArgsTy &&...Args) {
    std::lock_guard Lock(TheMutex);
    (TheVector.push_back(std::forward<ArgsTy>(Args)), ...);
  }

  template <typename... ArgsTy> void emplace_back(ArgsTy &&...Args) {
    std::lock_guard Lock(TheMutex);
    TheVector.emplace_back(std::forward<ArgsTy>(Args)...);
  }
};
} // namespace threadsafe