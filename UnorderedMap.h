#pragma once

#include "Types.h"

#include <mutex>
#include <shared_mutex>
#include <unordered_map>

namespace threadsafe {
template <typename Key, typename T, typename Hash = std::hash<Key>,
          typename Pred = std::equal_to<Key>,
          typename Alloc = std::allocator<std::pair<const Key, T>>>
class UnorderedMap {
  using BaseTy = std::unordered_map<Key, T, Hash, Pred, Alloc>;
  BaseTy Raw;

  mutable SharedMutexTy TheMutex;

public:
  using key_type = BaseTy::key_type;
  using mapped_type = BaseTy::mapped_type;
  using hasher = BaseTy::hasher;
  using key_equal = BaseTy::key_equal;
  using allocator_type = BaseTy::allocator_type;
  using value_type = BaseTy::value_type;
  using reference = BaseTy::reference;
  using const_reference = BaseTy::const_reference;
  using pointer = BaseTy::pointer;
  using const_pointer = BaseTy::const_pointer;
  using size_type = BaseTy::size_type;
  using difference_type = BaseTy::difference_type;
  using iterator = BaseTy::iterator;
  using const_iterator = BaseTy::const_iterator;
  using local_iterator = BaseTy::local_iterator;
  using const_local_iterator = BaseTy::const_local_iterator;

#if __cplusplus >= 201703L
  using node_type = BaseTy::node_type;
  using insert_return_type = BaseTy::insert_return_type;
#endif

  UnorderedMap() noexcept(
      std::is_nothrow_default_constructible_v<hasher> &&
      std::is_nothrow_default_constructible_v<key_equal> &&
      std::is_nothrow_default_constructible_v<allocator_type>) = default;

  explicit UnorderedMap(size_type N, const hasher &HF = hasher(),
                        const key_equal &Eql = key_equal(),
                        const allocator_type &A = allocator_type())
      : Raw(N, HF, Eql, A) {}

  template <typename InputIterator>
  UnorderedMap(InputIterator F, InputIterator L, size_type N = 0,
               const hasher &HF = hasher(), const key_equal &Eql = key_equal(),
               const allocator_type &A = allocator_type())
      : Raw(F, L, N, HF, Eql, A) {}

#if __cplusplus >= 202300L
  template <typename R>
  UnorderedMap(std::from_range_t, R &&Rg, size_type N = 0,
               const hasher &HF = hasher(), const key_equal &Eql = key_equal(),
               const allocator_type &A = allocator_type())
      : Raw(std::from_range, std::forward<R>(Rg), N, HF, Eql, A) {}
#endif

  explicit UnorderedMap(const allocator_type &A) : Raw(A) {}

  UnorderedMap(const UnorderedMap &Other) {
    ReadLockTy Lock(Other.TheMutex);
    Raw = Other.Raw;
  }

  UnorderedMap(const UnorderedMap &Other, const allocator_type &A) {
    ReadLockTy Lock(Other.TheMutex);
    Raw = BaseTy(Other.Raw, A);
  }

  UnorderedMap(UnorderedMap &&Other) noexcept(
      std::is_nothrow_move_constructible_v<hasher> &&
      std::is_nothrow_move_constructible_v<key_equal> &&
      std::is_nothrow_move_constructible_v<allocator_type>) {
    std::lock_guard Lock(Other.TheMutex);
    Raw = std::move(Other.Raw);
  }

  UnorderedMap(UnorderedMap &&Other, const allocator_type &A) {
    std::shared_lock Lock(Other.TheMutex);
    Raw = BaseTy(std::move(Other.Raw), A);
  }

  UnorderedMap(std::initializer_list<value_type> IL, size_type N = 0,
               const hasher &HF = hasher(), const key_equal &Eql = key_equal(),
               const allocator_type &A = allocator_type())
      : Raw(IL, N, HF, Eql, A) {}

  UnorderedMap(size_type N, const allocator_type &A)
      : UnorderedMap(N, hasher(), key_equal(), A) {}

  UnorderedMap(size_type N, const hasher &HF, const allocator_type &A)
      : UnorderedMap(N, HF, key_equal(), A) {}

  template <typename InputIterator>
  UnorderedMap(InputIterator F, InputIterator L, size_type N,
               const allocator_type &A)
      : UnorderedMap(F, L, N, hasher(), key_equal(), A) {}

  template <typename InputIterator>
  UnorderedMap(InputIterator F, InputIterator L, size_type N, const hasher &HF,
               const allocator_type &A)
      : UnorderedMap(F, L, N, HF, key_equal(), A) {}

#if __cplusplus >= 202300L
  template <typename R>
  UnorderedMap(std::from_range_t, R &&Rg, size_type N, const allocator_type &A)
      : UnorderedMap(std::from_range, std::forward<R>(Rg), N, hasher(),
                     key_equal(), A) {}

  template <typename R>
  UnorderedMap(std::from_range_t, R &&Rg, size_type N, const hasher &HF,
               const allocator_type &A)
      : UnorderedMap(std::from_range, std::forward<R>(Rg), N, HF, key_equal(),
                     A) {}
#endif

  UnorderedMap(std::initializer_list<value_type> IL, size_type N,
               const allocator_type &A)
      : UnorderedMap(IL, N, hasher(), key_equal(), A) {}

  UnorderedMap(std::initializer_list<value_type> IL, size_type N,
               const hasher &HF, const allocator_type &A)
      : UnorderedMap(IL, N, HF, key_equal(), A) {}

  ~UnorderedMap() = default;

  UnorderedMap &operator=(const UnorderedMap &Other) {
    if (this == &Other)
      return *this;

    WriteLockTy Lock1(TheMutex, std::defer_lock);
    ReadLockTy Lock2(Other.TheMutex, std::defer_lock);
    std::lock(Lock1, Lock2);
    Raw = Other.Raw;
    return *this;
  }

  UnorderedMap &operator=(UnorderedMap &&Other) noexcept(
      allocator_type::propagate_on_container_move_assignment::value &&
      std::is_nothrow_move_assignable_v<allocator_type> &&
      std::is_nothrow_move_assignable_v<hasher> &&
      std::is_nothrow_move_assignable_v<key_equal>) {
    if (this == &Other)
      return *this;

#if __cplusplus >= 201703L
    std::scoped_lock Lock(TheMutex, Other.TheMutex);
#else
    WriteLockTy Lock1(TheMutex, std::defer_lock);
    ReadLockTy Lock2(Other.TheMutex, std::defer_lock);
    std::lock(Lock1, Lock2);
#endif
    Raw = std::move(Other.Raw);
    return *this;
  }

  UnorderedMap &operator=(std::initializer_list<value_type> IL) {
    std::lock_guard Lock(TheMutex);
    Raw = IL;
    return *this;
  }

  allocator_type get_allocator() const noexcept { return Raw.get_allocator(); }

  bool empty() const noexcept {
    ReadLockTy Lock(TheMutex);
    return Raw.empty();
  }

  size_type size() const noexcept {
    ReadLockTy Lock(TheMutex);
    return Raw.size();
  }

  size_type max_size() const {
    ReadLockTy Lock(TheMutex);
    return Raw.max_size();
  }

  iterator begin() noexcept {
    ReadLockTy Lock(TheMutex);
    return Raw.begin();
  }

  iterator end() noexcept {
    ReadLockTy Lock(TheMutex);
    return Raw.end();
  }

  const_iterator begin() const noexcept {
    ReadLockTy Lock(TheMutex);
    return Raw.begin();
  }

  const_iterator end() const noexcept {
    ReadLockTy Lock(TheMutex);
    return Raw.end();
  }

  const_iterator cbegin() const noexcept {
    ReadLockTy Lock(TheMutex);
    return Raw.cbegin();
  }

  const_iterator cend() const noexcept {
    ReadLockTy Lock(TheMutex);
    return Raw.cend();
  }

  template <typename... Args> std::pair<iterator, bool> emplace(Args &&...A) {
    std::lock_guard Lock(TheMutex);
    return Raw.emplace(std::forward<Args>(A)...);
  }

  template <typename... Args>
  iterator emplace_hint(const_iterator Position, Args &&...A) {
    std::lock_guard Lock(TheMutex);
    return Raw.emplace_hint(Position, std::forward<Args>(A)...);
  }

  std::pair<iterator, bool> insert(const value_type &Obj) {
    std::lock_guard Lock(TheMutex);
    return Raw.insert(Obj);
  }

  template <typename P> std::pair<iterator, bool> insert(P &&Obj) {
    std::lock_guard Lock(TheMutex);
    return Raw.insert(std::forward<P>(Obj));
  }

  iterator insert(const_iterator Hint, const value_type &Obj) {
    std::lock_guard Lock(TheMutex);
    return Raw.insert(Hint, Obj);
  }

  template <typename P> iterator insert(const_iterator Hint, P &&Obj) {
    std::lock_guard Lock(TheMutex);
    return Raw.insert(Hint, std::forward<P>(Obj));
  }

  template <typename InputIterator>
  void insert(InputIterator First, InputIterator Last) {
    std::lock_guard Lock(TheMutex);
    Raw.insert(First, Last);
  }

#if __cplusplus >= 202300L
  template <typename R> void insert_range(R &&Rg) {
    std::lock_guard Lock(TheMutex);
    Raw.insert(std::from_range, std::forward<R>(Rg));
  }
#endif

  void insert(std::initializer_list<value_type> IL) {
    std::lock_guard Lock(TheMutex);
    Raw.insert(IL);
  }

  node_type extract(const_iterator Position) {
    std::lock_guard Lock(TheMutex);
    return Raw.extract(Position);
  }

  node_type extract(const key_type &X) {
    std::lock_guard Lock(TheMutex);
    return Raw.extract(X);
  }

  insert_return_type insert(node_type &&NH) {
    std::lock_guard Lock(TheMutex);
    return Raw.insert(std::move(NH));
  }

  iterator insert(const_iterator Hint, node_type &&NH) {
    std::lock_guard Lock(TheMutex);
    return Raw.insert(Hint, std::move(NH));
  }

  template <typename... Args>
  std::pair<iterator, bool> try_emplace(const key_type &K, Args &&...A) {
    std::lock_guard Lock(TheMutex);
    return Raw.try_emplace(K, std::forward<Args>(A)...);
  }

  template <typename... Args>
  std::pair<iterator, bool> try_emplace(key_type &&K, Args &&...A) {
    std::lock_guard Lock(TheMutex);
    return Raw.try_emplace(std::move(K), std::forward<Args>(A)...);
  }

  template <typename... Args>
  iterator try_emplace(const_iterator Hint, const key_type &K, Args &&...A) {
    std::lock_guard Lock(TheMutex);
    return Raw.try_emplace(Hint, K, std::forward<Args>(A)...);
  }

  template <typename... Args>
  iterator try_emplace(const_iterator Hint, key_type &&K, Args &&...A) {
    std::lock_guard Lock(TheMutex);
    return Raw.try_emplace(Hint, std::move(K), std::forward<Args>(A)...);
  }

  template <typename M>
  std::pair<iterator, bool> insert_or_assign(const key_type &K, M &&Obj) {
    std::lock_guard Lock(TheMutex);
    return Raw.insert_or_assign(K, std::forward<M>(Obj));
  }

  template <typename M>
  std::pair<iterator, bool> insert_or_assign(key_type &&K, M &&Obj) {
    std::lock_guard Lock(TheMutex);
    return Raw.insert_or_assign(std::move(K), std::forward<M>(Obj));
  }

  template <typename M>
  iterator insert_or_assign(const_iterator Hint, const key_type &K, M &&Obj) {
    std::lock_guard Lock(TheMutex);
    return Raw.insert_or_assign(Hint, K, std::forward<M>(Obj));
  }

  template <typename M>
  iterator insert_or_assign(const_iterator Hint, key_type &&K, M &&Obj) {
    std::lock_guard Lock(TheMutex);
    return Raw.insert_or_assign(Hint, std::move(K), std::forward<M>(Obj));
  }

  iterator erase(const_iterator Position) {
    std::lock_guard Lock(TheMutex);
    return Raw.erase(Position);
  }

  iterator erase(iterator Position) {
    std::lock_guard Lock(TheMutex);
    return Raw.erase(Position);
  }

  size_type erase(const key_type &K) {
    std::lock_guard Lock(TheMutex);
    return Raw.erase(K);
  }

  iterator erase(const_iterator First, const_iterator Last) {
    std::lock_guard Lock(TheMutex);
    return Raw.erase(First, Last);
  }

  void clear() noexcept {
    std::lock_guard Lock(TheMutex);
    Raw.clear();
  }

  template <typename H2, typename P2>
  void merge(std::unordered_map<Key, T, H2, P2, Alloc> &Source) {
    std::lock_guard Lock(TheMutex);
    Raw.merge(Source);
  }

  template <typename H2, typename P2>
  void merge(std::unordered_map<Key, T, H2, P2, Alloc> &&Source) {
    std::lock_guard Lock(TheMutex);
    Raw.merge(std::move(Source));
  }

  template <typename H2, typename P2>
  void merge(std::unordered_multimap<Key, T, H2, P2, Alloc> &Source) {
    std::lock_guard Lock(TheMutex);
    Raw.merge(Source);
  }

  template <typename H2, typename P2>
  void merge(std::unordered_multimap<Key, T, H2, P2, Alloc> &&Source) {
    std::lock_guard Lock(TheMutex);
    Raw.merge(std::move(Source));
  }

  void swap(UnorderedMap &Other) noexcept(
      (!allocator_type::propagate_on_container_swap::value ||
       std::is_nothrow_swappable_v<allocator_type>) &&
      std::is_nothrow_swappable_v<hasher> &&
      std::is_nothrow_swappable_v<key_equal>) {
    if (this == &Other)
      return;

    std::scoped_lock lock(TheMutex, Other.TheMutex);

    if constexpr (allocator_type::propagate_on_container_swap::value)
      std::swap(Raw, Other.Raw);
    std::swap(Raw, Other.Raw);
  }

  hasher hash_function() const {
    ReadLockTy Lock(TheMutex);
    return Raw.hash_function();
  }

  key_equal key_eq() const {
    ReadLockTy Lock(TheMutex);
    return Raw.key_eq();
  }

  iterator find(const key_type &K) {
    ReadLockTy Lock(TheMutex);
    return Raw.find(K);
  }

  const_iterator find(const key_type &K) const {
    ReadLockTy Lock(TheMutex);
    return Raw.find(K);
  }

#if __cplusplus >= 202000L
  template <typename K> iterator find(const K &X) {
    ReadLockTy Lock(TheMutex);
    return Raw.find(X);
  }

  template <typename K> const_iterator find(const K &X) const {
    ReadLockTy Lock(TheMutex);
    return Raw.find(X);
  }
#endif

  size_type count(const key_type &K) const {
    std::shared_lock Lock(TheMutex);
    return Raw.count(K);
  }

#if __cplusplus >= 202000L
  template <typename KeyTy> size_type count(const KeyTy &K) const {
    ReadLockTy Lock(TheMutex);
    return Raw.count(K);
  }

  bool contains(const key_type &K) const {
    ReadLockTy Lock(TheMutex);
    return Raw.contains(K);
  }

  template <typename KeyTy> bool contains(const KeyTy &K) const {
    ReadLockTy Lock(TheMutex);
    return Raw.contains(K);
  }
#endif

  std::pair<iterator, iterator> equal_range(const key_type &K) {
    ReadLockTy Lock(TheMutex);
    return Raw.equal_range(K);
  };

  std::pair<const_iterator, const_iterator>
  equal_range(const key_type &K) const {
    ReadLockTy Lock(TheMutex);
    return Raw.equal_range(K);
  }

#if __cplusplus >= 202000L
  template <typename KeyTy>
  std::pair<iterator, iterator> equal_range(const KeyTy &K) {
    ReadLockTy Lock(TheMutex);
    return Raw.equal_range(K);
  }

  template <typename KeyTy>
  std::pair<const_iterator, const_iterator> equal_range(const KeyTy &K) const {
    ReadLockTy Lock(TheMutex);
    return Raw.equal_range(K);
  }
#endif

  mapped_type &operator[](const key_type &K) {
    std::lock_guard Lock(TheMutex);
    return Raw[K];
  }

  mapped_type &operator[](key_type &&K) {
    std::lock_guard Lock(TheMutex);
    return Raw[std::move(K)];
  }

  mapped_type &at(const key_type &K) {
    ReadLockTy Lock(TheMutex);
    return Raw.at(K);
  }

  const mapped_type &at(const key_type &K) const {
    ReadLockTy Lock(TheMutex);
    return Raw.at(K);
  }

  size_type bucket_count() const noexcept {
    ReadLockTy Lock(TheMutex);
    return Raw.bucket_count();
  }

  size_type max_bucket_count() const noexcept {
    ReadLockTy Lock(TheMutex);
    return Raw.max_bucket_count();
  }

  size_type bucket_size(size_type N) const {
    ReadLockTy Lock(TheMutex);
    return Raw.bucket_size(N);
  }

  size_type bucket(const key_type &K) const {
    ReadLockTy Lock(TheMutex);
    return Raw.bucket(K);
  }

  local_iterator begin(size_type N) {
    ReadLockTy Lock(TheMutex);
    return Raw.begin(N);
  }

  local_iterator end(size_type N) {
    ReadLockTy Lock(TheMutex);
    return Raw.end(N);
  }

  const_local_iterator begin(size_type N) const {
    ReadLockTy Lock(TheMutex);
    return Raw.begin(N);
  }

  const_local_iterator end(size_type N) const {
    ReadLockTy Lock(TheMutex);
    return Raw.end(N);
  }

  const_local_iterator cbegin(size_type N) const {
    ReadLockTy Lock(TheMutex);
    return Raw.cbegin(N);
  }

  const_local_iterator cend(size_type N) const {
    ReadLockTy Lock(TheMutex);
    return Raw.cend(N);
  }

  float load_factor() const noexcept {
    ReadLockTy Lock(TheMutex);
    return Raw.load_factor();
  }

  float max_load_factor() const noexcept {
    ReadLockTy Lock(TheMutex);
    return Raw.max_load_factor();
  }

  void max_load_factor(float Z) {
    ReadLockTy Lock(TheMutex);
    Raw.max_load_factor(Z);
  }

  void rehash(size_type N) {
    std::lock_guard Lock(TheMutex);
    Raw.rehash(N);
  }

  void reserve(size_type N) {
    std::lock_guard Lock(TheMutex);
    Raw.reserve(N);
  }
};

#if __cplusplus >= 201703L
template <class InputIt, class Hash = std::hash<std::__iter_key_type<InputIt>>,
          class Pred = std::equal_to<std::__iter_key_type<InputIt>>,
          class Alloc = std::allocator<std::__iter_key_type<InputIt>>>
UnorderedMap(InputIt, InputIt,
             typename std::allocator_traits<Alloc>::size_type = 0,
             Hash = Hash(), Pred = Pred(), Alloc = Alloc())
    -> UnorderedMap<std::__iter_key_type<InputIt>,
                    std::__iter_mapped_type<InputIt>, Hash, Pred, Alloc>;

template <class Key, class T, class Hash = std::hash<Key>,
          class Pred = std::equal_to<Key>,
          class Alloc = std::allocator<std::pair<const Key, T>>>
UnorderedMap(std::initializer_list<std::pair<Key, T>>,
             typename std::allocator_traits<Alloc>::size_type = 0,
             Hash = Hash(), Pred = Pred(), Alloc = Alloc())
    -> UnorderedMap<Key, T, Hash, Pred, Alloc>;

template <class InputIt, class Alloc>
UnorderedMap(InputIt, InputIt, typename std::allocator_traits<Alloc>::size_type,
             Alloc)
    -> UnorderedMap<std::__iter_key_type<InputIt>,
                    std::__iter_mapped_type<InputIt>,
                    std::hash<std::__iter_key_type<InputIt>>,
                    std::equal_to<std::__iter_key_type<InputIt>>, Alloc>;

template <class InputIt, class Alloc>
UnorderedMap(InputIt, InputIt, Alloc)
    -> UnorderedMap<std::__iter_key_type<InputIt>,
                    std::__iter_mapped_type<InputIt>,
                    std::hash<std::__iter_key_type<InputIt>>,
                    std::equal_to<std::__iter_key_type<InputIt>>, Alloc>;

template <class InputIt, class Hash, class Alloc>
UnorderedMap(InputIt, InputIt, typename std::allocator_traits<Alloc>::size_type,
             Hash, Alloc)
    -> UnorderedMap<std::__iter_key_type<InputIt>,
                    std::__iter_mapped_type<InputIt>, Hash,
                    std::equal_to<std::__iter_key_type<InputIt>>, Alloc>;

template <class Key, class T, typename Alloc>
UnorderedMap(std::initializer_list<std::pair<Key, T>>,
             typename std::allocator_traits<Alloc>::size_type, Alloc)
    -> UnorderedMap<Key, T, std::hash<Key>, std::equal_to<Key>, Alloc>;

template <class Key, class T, typename Alloc>
UnorderedMap(std::initializer_list<std::pair<Key, T>>, Alloc)
    -> UnorderedMap<Key, T, std::hash<Key>, std::equal_to<Key>, Alloc>;

template <class Key, class T, class Hash, class Alloc>
UnorderedMap(std::initializer_list<std::pair<Key, T>>,
             typename std::allocator_traits<Alloc>::size_type, Hash, Alloc)
    -> UnorderedMap<Key, T, Hash, std::equal_to<Key>, Alloc>;

#if __cplusplus >= 202300L
template <std::ranges::input_range R,
          class Hash = std::hash<std::__range_key_type<R>>,
          class Pred = std::equal_to<std::__range_key_type<R>>,
          class Alloc = std::allocator<std::__range_to_alloc_type<R>>>
UnorderedMap(std::from_range_t, R &&,
             typename std::allocator_traits<Alloc>::size_type = 0,
             Hash = Hash(), Pred = Pred(), Alloc = Alloc())
    -> UnorderedMap<std::__range_key_type<R>, std::__range_mapped_type<R>, Hash,
                    Pred, Alloc>;

template <std::ranges::input_range R, class Alloc>
UnorderedMap(std::from_range_t, R &&,
             typename std::allocator_traits<Alloc>::size_type, Alloc)
    -> UnorderedMap<std::__range_key_type<R>, std::__range_mapped_type<R>,
                    std::hash<std::__range_key_type<R>>,
                    std::equal_to<std::__range_key_type<R>>, Alloc>;

template <std::ranges::input_range R, class Alloc>
UnorderedMap(std::from_range_t, R &&, Alloc)
    -> UnorderedMap<std::__range_key_type<R>, std::__range_mapped_type<R>,
                    std::hash<std::__range_key_type<R>>,
                    std::equal_to<std::__range_key_type<R>>, Alloc>;

template <std::ranges::input_range R, class Hash, class Alloc>
UnorderedMap(std::from_range_t, R &&,
             typename std::allocator_traits<Alloc>::size_type, Hash, Alloc)
    -> UnorderedMap<std::__range_key_type<R>, std::__range_mapped_type<R>, Hash,
                    std::equal_to<std::__range_key_type<R>>, Alloc>;

#endif
#endif
} // namespace threadsafe