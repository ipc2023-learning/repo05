#ifndef PROB_DIST_H
#define PROB_DIST_H

#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include <array>
#include <cassert>


/******************************************************************************
                                  INTERFACES
 ******************************************************************************/
template<typename T>
class HiddenConstIteIface {
 public:
  virtual ~HiddenConstIteIface() { }
  virtual T const& event() const = 0;
  virtual double prob() const = 0;
  virtual void increment() = 0;
  virtual bool equal(HiddenConstIteIface const* rhs) const = 0;
  virtual HiddenConstIteIface* clone() const = 0;
};




template<typename T>
class ProbDistIface {
 public:
//  virtual double probability(T const& t) const = 0;
  virtual void insert(T const& t, double pr) = 0;
  virtual void clear() = 0;

  class const_iterator;
  virtual const_iterator begin() const = 0;
  virtual const_iterator end() const = 0;


  virtual double normalizingConstant() const {
    double nc = 0;
    for (auto const& it : *this) {
      nc += it.prob();
    }
    return nc;
  }

  virtual T const& sample(double normalizing_constant = 1.0) const {
    if (normalizing_constant < 0) {
      normalizing_constant = normalizingConstant();
    }
    double r = drand48() * normalizing_constant;
    for (auto const& it : *this) {
      r -= it.prob();
      if (r <= 0) return it.event();
    }
    assert(false);
    return begin().event();
  }

  class const_iterator {
    public:
      const_iterator() : it_(0) { }
      const_iterator(HiddenConstIteIface<T>* it) : it_(it) { }
      const_iterator(const_iterator const& other) : it_(nullptr) {
        if (other.it_)
          it_ = other.it_->clone();
      }
      const_iterator(const_iterator&& other) noexcept : it_(other.it_) {
        other.it_ = nullptr;
      }
      const_iterator& operator=(const_iterator const& rhs) {
        if (&rhs != this) {
          delete it_;
          it_ = (rhs.it_ == nullptr ? nullptr : rhs.it_->clone());
        }
        return *this;
      }
      const_iterator& operator=(const_iterator&& rhs) noexcept {
        if (&rhs != this) {
          delete it_;
          it_ = rhs.it_;
          rhs.it_ = nullptr;
        }
        return *this;
      }
      ~const_iterator() { delete it_; }

      T const& event() const { assert(it_); return it_->event(); }
      double prob()    const { assert(it_); return it_->prob(); }

      const_iterator& operator++() {
        if (it_)
          it_->increment();
        return *this;
      }
      const_iterator& operator*() { return *this; }
      bool operator==(ProbDistIface<T>::const_iterator const& rhs) {
        if (it_ == rhs.it_) { return true;  }
        else if (it_)       { return it_->equal(rhs.it_); }
        // it_ == NULL and rhs.it_ != NULL
        else                { return rhs.it_->equal(it_); }
      }
      bool operator!=(ProbDistIface<T>::const_iterator const& rhs) {
        return ! operator==(rhs);
      }
    private:
      HiddenConstIteIface<T>* it_;
  };

};



/******************************************************************************
                            VECTOR-BASED TEMPLATE
 ******************************************************************************/
template<typename T>
class ProbDistVectorIte : public HiddenConstIteIface<T> {
 private:
  //typedef std::vector<std::pair<T, double> >::const_iterator InnerIte;
 public:
  ProbDistVectorIte(
      typename std::vector<std::pair<T, double> >::const_iterator const& it,
      typename std::vector<std::pair<T, double> >::const_iterator const& end)
    : it_(it), end_(end)
  { }
  T const& event() const { return it_->first; }
  double prob() const { return it_->second; }
  void increment() { ++it_; }
  virtual bool equal(HiddenConstIteIface<T> const* rhs) const {
    if (rhs == nullptr && it_ == end_) {
      return true;
    }
    ProbDistVectorIte const* conv_rhs = dynamic_cast<ProbDistVectorIte const*>(rhs);
    if (conv_rhs) {
      return it_ == conv_rhs->it_;
    }
    else {
      return false;
    }
  }
  virtual HiddenConstIteIface<T>* clone() const {
    return new ProbDistVectorIte(it_, end_);
  }
 private:
  typename std::vector<std::pair<T, double> >::const_iterator it_, end_;
};

template<typename T>
class ProbDistVector : public ProbDistIface<T> {
 public:
  void insert(T const& i, double pr) {
    v_.push_back(std::make_pair(i, pr));
  }

  void clear() { v_.clear(); }

  typename ProbDistIface<T>::const_iterator begin() const {
    ProbDistVectorIte<T>* i = new ProbDistVectorIte<T>(v_.begin(), v_.end());
    return typename ProbDistIface<T>::const_iterator(i);
  }
  typename ProbDistIface<T>::const_iterator end() const {
    return typename ProbDistIface<T>::const_iterator(nullptr);
  }

 private:
  std::vector<std::pair<T, double> > v_;
};



/******************************************************************************
                             ARRAY-BASED TEMPLATE
 ******************************************************************************/
template<typename T, size_t N>
class ProbDistStlArrayConstIte : public HiddenConstIteIface<T> {
 public:
  ProbDistStlArrayConstIte(std::array<std::pair<T, double>, N> const& array,
      size_t i, size_t len)
    : array_(array), i_(i), len_(len)
  {
    assert(len <= N);
    assert(i <= len);
  }
  T const& event() const { return array_[i_].first; }
  double prob() const { return array_[i_].second; }
  void increment() {
    i_++;
    assert(i_ <= len_);
  }
  virtual bool equal(HiddenConstIteIface<T> const* rhs) const {
    if (rhs == nullptr && i_ == len_) {
      return true;
    }
    ProbDistStlArrayConstIte const* conv_rhs =
                             dynamic_cast<ProbDistStlArrayConstIte const*>(rhs);
    if (conv_rhs) {
      return len_ == conv_rhs->len_ && i_ == conv_rhs->i_;
    }
    else {
      return false;
    }
  }
  virtual HiddenConstIteIface<T>* clone() const {
    return new ProbDistStlArrayConstIte(array_, i_, len_);
  }

 private:
  typename std::array<std::pair<T, double>, N> const& array_;
  size_t i_;
  size_t len_;
};




template<typename T, size_t N>
class ProbDistStlArray : public ProbDistIface<T> {
 public:
  ProbDistStlArray() : next_available_(0) { }
  /*
   * FWT: The move constructor and move assign operator are being added in order
   * to flag potential pitfalls. Since std::array is basically the same as a
   * preallocated array (i.e., T var[N]), the move semantics doesn't make sense
   * and a copy is made instead. This becomes a problem when a class use the
   * move semantics, e.g., std::vector, since a pointer or reference for the old
   * item would not be valid after being moved (because it was copied instead).
   * If a move semantics is needed, then use ProbDistAllocArray.
   */
  ProbDistStlArray(ProbDistStlArray&& o) noexcept {
    std::cout << "ProbDistStlArray: Move constructor called. Potential bug!"
              << std::endl;
    assert(false);
    exit(-1);
  }
  ProbDistStlArray operator=(ProbDistStlArray&& o) noexcept {
    std::cout << "ProbDistStlArray: Move assignment operator called. "
              << "Potential bug!" << std::endl;
    assert(false);
    exit(-1);
    return *this;
  }
  void insert(T const& i, double pr) {
    assert(next_available_ < N);
    a_[next_available_] = std::make_pair(i, pr);
    next_available_++;
  }

  void clear() { next_available_ = 0; }

  typename ProbDistIface<T>::const_iterator begin() const {
    ProbDistStlArrayConstIte<T, N>* i = new ProbDistStlArrayConstIte<T, N>(
        a_, 0, next_available_);
    return typename ProbDistIface<T>::const_iterator(i);
  }
  typename ProbDistIface<T>::const_iterator end() const {
    return typename ProbDistIface<T>::const_iterator(nullptr);
  }


 private:
  size_t next_available_;
  std::array<std::pair<T, double>, N> a_;
};



/******************************************************************************
                          C-STYLE ARRAY-BASED TEMPLATE
 A tiny bit less efficient than the std::array; however, it has a move semantics
 and can be used as items in other containers, specially std::vector.
 ******************************************************************************/
template<typename T, size_t N>
class ProbDistAllocArrayConstIte : public HiddenConstIteIface<T> {
 public:
  ProbDistAllocArrayConstIte(T const* array_event, double const* array_pr,
      size_t i, size_t len)
    : e_(array_event), p_(array_pr), i_(i), len_(len)
  {
    assert(len_ <= N);
    assert(i_ <= len_);
  }

  T const& event() const { return e_[i_]; }
  double prob() const { return p_[i_]; }
  void increment() {
    i_++;
    assert(i_ <= len_);
  }
  virtual bool equal(HiddenConstIteIface<T> const* rhs) const {
    if (rhs == nullptr && i_ == len_) {
      return true;
    }
    ProbDistAllocArrayConstIte const* conv_rhs =
      dynamic_cast<ProbDistAllocArrayConstIte const*>(rhs);
    if (conv_rhs) {
      return conv_rhs->p_ == p_
              && conv_rhs->e_ == e_
              && len_ == conv_rhs->len_
              && i_ == conv_rhs->i_;
    }
    else {
      return false;
    }
  }
  virtual HiddenConstIteIface<T>* clone() const {
    return new ProbDistAllocArrayConstIte(e_, p_, i_, len_);
  }

 private:
  T const* e_;
  double const* p_;
  size_t i_;
  size_t len_;
};

template<typename T, size_t N>
class ProbDistAllocArray : public ProbDistIface<T> {
 public:
  ProbDistAllocArray() : next_available_(0), e_(new T[N]), p_(new double[N])
  { }
  ProbDistAllocArray(ProbDistAllocArray const& other)
    : next_available_(0), e_(new T[N]), p_(new double[N])
  {
    memcpy(p_, other.p_, other.next_available_ * sizeof(double));
    // TODO(fwt): Not sure if there is a more efficient way of doing this.
    // Keep in mind that we need to invoke the copy constructor of T since it
    // might be a complex object managing dynamic memory (e.g., state_t).
    for (size_t i = 0; i < other.next_available_; ++i) {
      e_[i] = other.e_[i];
    }
    next_available_ = other.next_available_;
  }

  ProbDistAllocArray& operator=(ProbDistAllocArray const& other) {
    if (&other != this) {
      next_available_ = other.next_available_;
      memcpy(p_, other.p_, other.next_available_ * sizeof(double));
      // TODO(fwt): Not sure if there is a more efficient way of doing this.
      // Keep in mind that we need to invoke the copy constructor of T since
      // it might be a complex object managing dynamic memory (e.g., state_t).
      for (size_t i = 0; i < other.next_available_; ++i) {
        e_[i] = other.e_[i];
      }
    }
    return *this;
  }

  // Move constructor
  ProbDistAllocArray(ProbDistAllocArray&& other) noexcept
    : next_available_(other.next_available_), e_(other.e_), p_(other.p_)
  {
    other.e_ = nullptr;
    other.p_ = nullptr;
  }

  // Move assign operator
  ProbDistAllocArray& operator=(ProbDistAllocArray&& other) noexcept {
    if (&other != this) {
      next_available_ = other.next_available_;
      e_ = other.e_;
      p_ = other.p_;
      other.e_ = nullptr;
      other.p_ = nullptr;
    }
  }

  ~ProbDistAllocArray() {
    delete[] p_;
    delete[] e_;
  }

  void insert(T const& i, double pr) {
    assert(next_available_ < N);
    e_[next_available_] = i;
    p_[next_available_] = pr;
    next_available_++;
  }

  void clear() { next_available_ = 0; }

  typename ProbDistIface<T>::const_iterator begin() const {
    ProbDistAllocArrayConstIte<T, N>* i = new ProbDistAllocArrayConstIte<T, N>(
        e_, p_, 0, next_available_);
    return typename ProbDistIface<T>::const_iterator(i);
  }
  typename ProbDistIface<T>::const_iterator end() const {
    return typename ProbDistIface<T>::const_iterator(nullptr);
  }

 private:
  size_t next_available_;
  T* e_;
  double* p_;
};




/******************************************************************************
                              MAP-BASED TEMPLATE
 ******************************************************************************/
template<typename T>
class ProbDistMapIte : public HiddenConstIteIface<T> {
 public:
  ProbDistMapIte(typename std::map<T, double>::const_iterator it) : it_(it) { }
  T const& event() const { return it_->first; }
  double prob() const { return it_->second; }
  void increment() { ++it_; }
  virtual bool equal(HiddenConstIteIface<T> const* rhs) const {
    ProbDistMapIte const* conv_rhs = dynamic_cast<ProbDistMapIte const*>(rhs);
    if (conv_rhs) {
      return it_ == conv_rhs->it_;
    }
    else {
      return false;
    }
  }
  virtual HiddenConstIteIface<T>* clone() const {
    return new ProbDistMapIte(it_);
  }

 private:
  typename std::map<T, double>::const_iterator it_;
};


template<typename T>
class ProbDistMap : public ProbDistIface<T> {
 public:
  void insert(T const& i, double pr) { m_[i] += pr; }

  void clear() { m_.clear(); }

  typename ProbDistIface<T>::const_iterator begin() const {
    ProbDistMapIte<T>* i = new ProbDistMapIte<T>(m_.begin());
    return typename ProbDistIface<T>::const_iterator(i);
  }
  typename ProbDistIface<T>::const_iterator end() const {
    ProbDistMapIte<T>* i = new ProbDistMapIte<T>(m_.end());
    return typename ProbDistIface<T>::const_iterator(i);
  }

 private:
  std::map<T, double> m_;
};


/******************************************************************************
                             HASH-BASED TEMPLATE
 ******************************************************************************/
template<typename T, typename Hash>
class ProbDistHashIte : public HiddenConstIteIface<T> {
 public:
  ProbDistHashIte(
      typename std::unordered_map<T, double, Hash>::const_iterator it)
    : it_(it) { }
  T const& event() const { return it_->first; }
  double prob() const { return it_->second; }
  void increment() { ++it_; }
  virtual bool equal(HiddenConstIteIface<T> const* rhs) const {
    ProbDistHashIte const* conv_rhs = dynamic_cast<ProbDistHashIte const*>(rhs);
    if (conv_rhs) {
      return it_ == conv_rhs->it_;
    }
    else {
      return false;
    }
  }
  virtual HiddenConstIteIface<T>* clone() const {
    return new ProbDistHashIte(it_);
  }

 private:
  typename std::unordered_map<T, double, Hash>::const_iterator it_;
};


template<typename T, typename Hash>
class ProbDistHash : public ProbDistIface<T> {
 public:
  void insert(T const& i, double pr) { m_[i] += pr; }

  void clear() { m_.clear(); }

  typename ProbDistIface<T>::const_iterator begin() const {
    ProbDistHashIte<T, Hash>* i = new ProbDistHashIte<T, Hash>(m_.begin());
    return typename ProbDistIface<T>::const_iterator(i);
  }
  typename ProbDistIface<T>::const_iterator end() const {
    ProbDistHashIte<T, Hash>* i = new ProbDistHashIte<T, Hash>(m_.end());
    return typename ProbDistIface<T>::const_iterator(i);
  }

 private:
  std::unordered_map<T, double, Hash> m_;
};

#endif  // PROB_DIST_H
