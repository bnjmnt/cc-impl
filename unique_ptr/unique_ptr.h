#ifndef CC_IMPL_UNIQUE_PTR_UNIQUE_PTR_H
#define CC_IMPL_UNIQUE_PTR_UNIQUE_PTR_H

#include <utility>

namespace ben {

template <typename T>
class unique_ptr {
 public:
  unique_ptr(T* ptr = nullptr) noexcept : ptr_(ptr) {}

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

  void reset(T* ptr = nullptr) noexcept {
    if (ptr_ == ptr) {
      return;
    }
    delete ptr_;
    ptr_ = ptr;
  }

 private:
  T* ptr_;
};

}  // namespace ben

#endif  // CC_IMPL_UNIQUE_PTR_UNIQUE_PTR_H