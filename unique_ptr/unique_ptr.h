#ifndef CC_IMPL_UNIQUE_PTR_UNIQUE_PTR_H
#define CC_IMPL_UNIQUE_PTR_UNIQUE_PTR_H

#include <cstddef>
#include <utility>

namespace ben {

template <typename T>
class unique_ptr {
 public:
  explicit unique_ptr(T* ptr = nullptr) noexcept : ptr_(ptr) {}

  ~unique_ptr() { delete ptr_; }

  unique_ptr(const unique_ptr&) noexcept = delete;

  unique_ptr& operator=(const unique_ptr&) noexcept = delete;

  unique_ptr(unique_ptr&& other) noexcept {
    ptr_ = std::exchange(other.ptr_, nullptr);
  }

  unique_ptr& operator=(unique_ptr&& other) noexcept {
    if (this == &other) {
      return *this;
    }
    delete ptr_;
    ptr_ = std::exchange(other.ptr_, nullptr);
    return *this;
  }

  T* get() const noexcept { return ptr_; }

  explicit operator bool() const noexcept { return ptr_ != nullptr; }

  T& operator*() const noexcept { return *ptr_; }

  T* operator->() const noexcept { return ptr_; }

  T* release() noexcept { return std::exchange(ptr_, nullptr); }

  void reset(T* ptr = nullptr) noexcept {
    if (ptr_ == ptr) {
      return;
    }
    delete ptr_;
    ptr_ = ptr;
  }

  void swap(unique_ptr& other) noexcept { std::swap(ptr_, other.ptr_); }

 private:
  T* ptr_;
};

template <typename T1, typename T2>
bool operator==(const unique_ptr<T1>& x, const unique_ptr<T2>& y) {
  return x.get() == y.get();
}

template <typename T1>
bool operator==(const unique_ptr<T1>& x, const std::nullptr_t y) {
  return x.get() == y;
}

template <typename T1>
bool operator==(const std::nullptr_t x, const unique_ptr<T1>& y) {
  return x == y.get();
}

template <typename T1, typename T2>
bool operator!=(const unique_ptr<T1>& x, const unique_ptr<T2>& y) {
  return x.get() != y.get();
}

template <typename T1>
bool operator!=(const unique_ptr<T1>& x, const std::nullptr_t y) {
  return x.get() != y;
}

template <typename T1>
bool operator!=(const std::nullptr_t x, const unique_ptr<T1>& y) {
  return x != y.get();
}

template <typename T, typename... Args>
unique_ptr<T> make_unique(Args&&... args) {
  return unique_ptr<T>(new T(std::forward<Args>(args)...));
}

}  // namespace ben

#endif  // CC_IMPL_UNIQUE_PTR_UNIQUE_PTR_H