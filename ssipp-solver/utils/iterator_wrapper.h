#ifndef ITERATOR_WRAPPER_H
#define ITERATOR_WRAPPER_H

#include <iostream>
#include <memory>

/*
 * This is a type erasure for iterator, i.e., a wrapper around any iterator that
 * let subclasses return their own implementation of the iterator. For a full
 * solution see:
 *
 * http://thbecker.net/free_software_utilities/type_erasure_for_cpp_iterators/any_iterator.html
 *
 */


/*
 * The whole design of this had the following things in mind:
 *  - T is a const reference to something, e.g., action_t const& and state_t
 *    const&
 *  - Their are intended to be used with range-based for-loop, i.e.,
 *    for (auto const& x : y) { ... }
 *  - A ConstIteratorWrapper is returned by a begin() and end() method. If the
 *    class doesn't have a suitable begin() and end(), then return a ConstRange
 *    since it wraps a begin() and end() given 2 ConstIteratorWrapper.
 */
template<typename T> class ConstIteratorIface;

template<typename T> using ConstIteratorIfaceUniqPtr =
                                        std::unique_ptr<ConstIteratorIface<T>>;


/*
 * Interface for an iterator over T. Different classes should implement this
 * interface and use the ConstIteratorWrapper by itself or to build ConstRange
 */
template<typename T>
class ConstIteratorIface {
 public:
  virtual ~ConstIteratorIface() { }
  virtual ConstIteratorIface& operator++() = 0;
  virtual bool operator!=(ConstIteratorIfaceUniqPtr<T> const& rhs) const = 0;
  virtual T operator*() const = 0;
  virtual ConstIteratorIfaceUniqPtr<T> clone() const = 0;
};


/*
 * Wrapper for ActionConstIteIface.
 *
 * This wrapper is necessary because ConstRange returns values (instead of
 * pointers or references) in the methods begin() and end(). Therefore, the
 * class returned by ConstRange (in this case ConstIteratorWrapper)
 * cannot be extended without suffering object slicing.
 */
template<typename T>
class ConstIteratorWrapper {
 public:
  /*
   * Rule of 5.
   */
  ConstIteratorWrapper() : it_iface_(nullptr) { }
  ConstIteratorWrapper(ConstIteratorIfaceUniqPtr<T> it_iface)
    : it_iface_(std::move(it_iface)) { }
  ConstIteratorWrapper(ConstIteratorWrapper const& other) {
    if (other.it_iface_) {
      it_iface_ = other.it_iface_->clone();
    }
  }
  ConstIteratorWrapper& operator=(ConstIteratorWrapper const& rhs) {
    if (rhs.it_iface_) {
      it_iface_ = rhs.it_iface_->clone();
    }
    return *this;
  }
  ConstIteratorWrapper(ConstIteratorWrapper&& rhs) noexcept
    : it_iface_(std::move(rhs.it_iface_)) { }
  ConstIteratorWrapper& operator=(ConstIteratorWrapper&& rhs) noexcept {
    if (this != &rhs) {
      it_iface_ = std::move(rhs.it_iface_);
    }
    return *this;
  }
  ~ConstIteratorWrapper() { }


  ConstIteratorWrapper& operator++() {
    if (it_iface_) ++(*it_iface_);
    return *this;
  }

  // This should never be called if it_iface_ is nullptr
  T operator*() const {
    assert(it_iface_);
    return it_iface_->operator*();
  }

  bool operator!=(ConstIteratorWrapper const& rhs) const {
    if (it_iface_ == rhs.it_iface_) { return false; }
    else if (it_iface_)       { return it_iface_->operator!=(rhs.it_iface_); }
    // it_iface_ == NULL and rhs.it_iface_ != NULL
    else                      { return rhs.it_iface_->operator!=(it_iface_); }
  }

 private:
  ConstIteratorIfaceUniqPtr<T> it_iface_;
};


/*
 * Representation of a range to be used in range-based for loops (and other
 * loops).
 *
 * It returns the given iterators as begin and end. This class mimics the return
 * value of std::map::equal_range and it is used to provide "conditional"
 * iterators, i.e., iterators for a subset of the container (in this case, a
 * subset of all actions). Notice that equal_range returns a pair of iterators
 * and in C++11, a pair of iterators is NO longer valid for range-based for
 * loops.
 */
template<typename T>
class ConstRange {
 public:
  // Empty ctor. Both iterators have nullptr
  ConstRange() : b_(), e_() { }
  // This ctor copies the wrappers
  ConstRange(ConstIteratorWrapper<T> b, ConstIteratorWrapper<T> e)
    : b_(b), e_(e)
  { }
  // This ctor build new wrappers using the given pointers to the
  // ActionConstIteIface. The wrapper keeps the ownership.
  ConstRange(ConstIteratorIfaceUniqPtr<T> b, ConstIteratorIfaceUniqPtr<T> e)
    : b_(std::move(b)), e_(std::move(e))
  { }
  ConstIteratorWrapper<T> begin() const { return b_; }
  ConstIteratorWrapper<T> end() const { return e_; }
 private:
  ConstIteratorWrapper<T> b_, e_;
};

#endif   // ITERATOR_WRAPPER_H
