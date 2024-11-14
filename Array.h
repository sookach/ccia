#pragma once

#include "Types.h"

#include <array>

namespace threadsafe {
template <typename T, std::size_t N> struct Array {
  using BaseTy = std::array<T, N>;
  BaseTy Raw;

public:
  using value_type = T;
  using pointer = T *;
  using const_pointer = const T *;
  using reference = T &;
  using const_reference = const T &;
  using size_type = size_t;
  using difference_type = ptrdiff_t;
  using iterator = implementation - defined;
  using const_iterator = implementation - defined;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  THREADSAFE_CONSTEXPR_20
  void fill(const T &U) { Raw.fill(U); }

  THREADSAFE_CONSTEXPR_20 void
  swap(Array &A) noexcept(std::is_nothrow_swappable_v<T>) {
    Raw.swap(A);
  }

  THREADSAFE_CONSTEXPR_20 iterator begin() noexcept { return Raw.begin(); }

  THREADSAFE_CONSTEXPR_20 const_iterator begin() const noexcept {
    return Raw.begin();
  }

  THREADSAFE_CONSTEXPR_20 iterator end() noexcept { return Raw.end(); }

  THREADSAFE_CONSTEXPR_20 const_iterator end() const noexcept {
    return Raw.end();
  }

  THREADSAFE_CONSTEXPR_20 reverse_iterator rbegin() noexcept {
    return Raw.rbegin();
  }

  THREADSAFE_CONSTEXPR_20 const_reverse_iterator rbegin() const noexcept {
    return Raw.rbegin();
  }

  THREADSAFE_CONSTEXPR_20 reverse_iterator rend() noexcept {
    return Raw.rend();
  }

  THREADSAFE_CONSTEXPR_20 const_reverse_iterator rend() const noexcept {
    return Raw.rend();
  }

  THREADSAFE_CONSTEXPR_20 const_iterator cbegin() const noexcept {
    return Raw.cbegin();
  }

  THREADSAFE_CONSTEXPR_20 const_iterator cend() const noexcept {
    return Raw.cend();
  }

  THREADSAFE_CONSTEXPR_20 const_reverse_iterator crbegin() const noexcept {
    return Raw.crbegin();
  }

  THREADSAFE_CONSTEXPR_20 const_reverse_iterator crend() const noexcept {
    return Raw.crend();
  }

  THREADSAFE_CONSTEXPR_20 size_type size() const noexcept { return Raw.size(); }

  THREADSAFE_CONSTEXPR_20 size_type max_size() const noexcept {
    return Raw.max_size();
  }

  THREADSAFE_CONSTEXPR_20 bool empty() const noexcept { return Raw.empty(); }

  THREADSAFE_CONSTEXPR_20 reference operator[](size_type N) { return Raw[N]; }

  THREADSAFE_CONSTEXPR_20 const_reference operator[](size_type N) const {
    return Raw[N];
  }

  THREADSAFE_CONSTEXPR_20 reference at(size_type N) { return Raw.at(N); }

  THREADSAFE_CONSTEXPR_20 const_reference at(size_type N) const {
    return Raw.at(N);
  }

  THREADSAFE_CONSTEXPR_20 reference front() { return Raw.front(); }

  THREADSAFE_CONSTEXPR_20 const_reference front() const { return Raw.front(); }

  THREADSAFE_CONSTEXPR_20 reference back() { return Raw.back(); }

  THREADSAFE_CONSTEXPR_20 const_reference back() const { return Raw.back(); }

  THREADSAFE_CONSTEXPR_20 T *data() noexcept { return Raw.data(); }

  THREADSAFE_CONSTEXPR_20 const T *data() const noexcept { return Raw.data(); }
};

#if 0
template <class T, class... U>
  array(T, U...) -> array<T, 1 + sizeof...(U)>;                 // C++17

template <class T, size_t N>
  bool operator==(const array<T,N>& x, const array<T,N>& y);    // constexpr in C++20
template <class T, size_t N>
  bool operator!=(const array<T,N>& x, const array<T,N>& y);    // removed in C++20
template <class T, size_t N>
  bool operator<(const array<T,N>& x, const array<T,N>& y);     // removed in C++20
template <class T, size_t N>
  bool operator>(const array<T,N>& x, const array<T,N>& y);     // removed in C++20
template <class T, size_t N>
  bool operator<=(const array<T,N>& x, const array<T,N>& y);    // removed in C++20
template <class T, size_t N>
  bool operator>=(const array<T,N>& x, const array<T,N>& y);    // removed in C++20
template<class T, size_t N>
  constexpr synth-three-way-result<T>
    operator<=>(const array<T, N>& x, const array<T, N>& y);    // since C++20

template <class T, size_t N >
  void swap(array<T,N>& x, array<T,N>& y) noexcept(noexcept(x.swap(y))); // constexpr in C++20

template <class T, size_t N>
  constexpr array<remove_cv_t<T>, N> to_array(T (&a)[N]);  // C++20
template <class T, size_t N>
  constexpr array<remove_cv_t<T>, N> to_array(T (&&a)[N]); // C++20

template <class T> struct tuple_size;
template <size_t I, class T> struct tuple_element;
template <class T, size_t N> struct tuple_size<array<T, N>>;
template <size_t I, class T, size_t N> struct tuple_element<I, array<T, N>>;
template <size_t I, class T, size_t N> T& get(array<T, N>&) noexcept;               // constexpr in C++14
template <size_t I, class T, size_t N> const T& get(const array<T, N>&) noexcept;   // constexpr in C++14
template <size_t I, class T, size_t N> T&& get(array<T, N>&&) noexcept;             // constexpr in C++14
template <size_t I, class T, size_t N> const T&& get(const array<T, N>&&) noexcept; // constexpr in C++14

}  // std
#endif

} // namespace threadsafe