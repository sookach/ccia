#pragma once

#include <mutex>
#include <shared_mutex>

#if __cplusplus >= 201703L
using SharedMutexTy = std::shared_mutex;
using ReadLockTy = std::shared_lock<SharedMutexTy>;
using WriteLockTy = std::unique_lock<SharedMutexTy>;
#else
using SharedMutexTy = std::mutex;
using ReadLockTy = std::unique_lock<SharedMutexTy>;
using WriteLockTy = std::unique_lock<SharedMutexTy>;
#endif

#if __cplusplus >= 202000L
#define THREADSAFE_CONSTEXPR_20 constexpr
#else
#define THREADSAFE_CONSTEXPR_20
#endif