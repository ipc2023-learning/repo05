#ifndef RATIONAL_H
#define RATIONAL_H

#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <utility>
#include <vector>

class Rational_mGPT;
class Rational_Float;


/*******************************************************************************
 *
 * Rational as defined on mGPT and in mdpsim
 * FWT: I patched parts of it.
 *
 ******************************************************************************/
class Rational_mGPT {
 public:
  Rational_mGPT(int n = 0) : numerator_(n), denominator_(1) { }
  Rational_mGPT(size_t n) : numerator_(n), denominator_(1) { }
  Rational_mGPT(float f) { convertDouble(f); }
  Rational_mGPT(double d) { convertDouble(d); }
  Rational_mGPT(int n, int m);
  Rational_mGPT(char const* s);
  double double_value() const { return (double)numerator()/denominator(); }
  Rational_mGPT abs() const {
    if (numerator_ < 0)
	    return(Rational_mGPT(-numerator_, denominator_));
    else
	    return *this;
  }

  Rational_mGPT inverse() const {
    return Rational_mGPT(denominator_, numerator_);
  }
  friend class VectorOfRationals_mGPT;
  friend bool operator<(const Rational_mGPT& q, const Rational_mGPT& p);
  friend bool operator<=(const Rational_mGPT& q, const Rational_mGPT& p);
  friend bool operator==(const Rational_mGPT& q, const Rational_mGPT& p);
  friend bool operator!=(const Rational_mGPT& q, const Rational_mGPT& p);
  friend bool operator>=(const Rational_mGPT& q, const Rational_mGPT& p);
  friend bool operator>(const Rational_mGPT& q, const Rational_mGPT& p);
  friend Rational_mGPT operator+(const Rational_mGPT& q, const Rational_mGPT& p);
  friend Rational_mGPT operator-(const Rational_mGPT& q, const Rational_mGPT& p);
  friend Rational_mGPT operator*(const Rational_mGPT& q, const Rational_mGPT& p);
  friend Rational_mGPT operator/(const Rational_mGPT& q, const Rational_mGPT& p);
  Rational_mGPT& operator+=(const Rational_mGPT& p)
    { (*this) = operator+(*this, p); return *this; }
  Rational_mGPT& operator-=(const Rational_mGPT& p)
    { (*this) = operator-(*this, p); return *this; }
  Rational_mGPT& operator*=(const Rational_mGPT& p)
    { (*this) = operator*(*this, p); return *this; }
  Rational_mGPT& operator/=(const Rational_mGPT& p)
    { (*this) = operator/(*this, p); return *this; }
  friend std::ostream& operator<<(std::ostream& os, const Rational_mGPT& q);

 protected:
  static std::pair<int, int> multipliers( int n, int m );
  int numerator() const { return numerator_; }
  int denominator() const { return denominator_; }

 private:
  int numerator_;
  int denominator_;

  void convertDouble(double d, int max_denominator = 10000);
};


class Rational_Float {
 public:
  Rational_Float(int n = 0) : val_(n) { }
  Rational_Float(size_t n) : val_(n) { }

  Rational_Float(int n, int m) : val_(n/(float) m) { }
  Rational_Float(char const* s) : val_(0) {
    if (strstr(s, "/")) {
      // Number given as a Rational
      Rational_mGPT r(s);
      val_ = r.double_value();
    } else {
      val_ = atof(s);
    }
//    std::cout << "[Rational_Float] Parsed " << s << " as " << *this << std::endl;
  }
  Rational_Float(float f) : val_(f) { }
  Rational_Float(double d) : val_(d) { }
  double double_value() const { return (double) val_; }
  Rational_Float abs() const {
    if (val_ < 0.0) return Rational_Float(-val_);
    else return *this;
  }

  Rational_Float inverse() const {
    return Rational_Float(1/val_);
  }

  Rational_Float& operator+=(const Rational_Float& p)
    { (*this) = operator+(*this, p); return *this; }
  Rational_Float& operator-=(const Rational_Float& p)
    { (*this) = operator-(*this, p); return *this; }
  Rational_Float& operator*=(const Rational_Float& p)
    { (*this) = operator*(*this, p); return *this; }
  Rational_Float& operator/=(const Rational_Float& p)
    { (*this) = operator/(*this, p); return *this; }

 private:
  float val_;

  friend bool operator<  (Rational_Float const& p, Rational_Float const& q);
  friend bool operator<= (Rational_Float const& p, Rational_Float const& q);
  friend bool operator== (Rational_Float const& p, Rational_Float const& q);
  friend bool operator!= (Rational_Float const& p, Rational_Float const& q);
  friend bool operator>= (Rational_Float const& p, Rational_Float const& q);
  friend bool operator>  (Rational_Float const& p, Rational_Float const& q);
  friend Rational_Float operator+ (Rational_Float const& p,
                                   Rational_Float const& q);
  friend Rational_Float operator- (Rational_Float const& p,
                                   Rational_Float const& q);
  friend Rational_Float operator* (Rational_Float const& p,
                                   Rational_Float const& q);
  friend Rational_Float operator/ (Rational_Float const& p,
                                   Rational_Float const& q);
  friend std::ostream& operator<< (std::ostream& os, Rational_Float const& p);
};
inline bool operator<  (Rational_Float const& p, Rational_Float const& q)
  { return p.val_ <  q.val_; }
inline bool operator<= (Rational_Float const& p, Rational_Float const& q)
  { return p.val_ <= q.val_; }
inline bool operator== (Rational_Float const& p, Rational_Float const& q)
  { return p.val_ == q.val_; }
inline bool operator!= (Rational_Float const& p, Rational_Float const& q)
  { return p.val_ != q.val_; }
inline bool operator>= (Rational_Float const& p, Rational_Float const& q)
  { return p.val_ >= q.val_; }
inline bool operator>  (Rational_Float const& p, Rational_Float const& q)
  { return p.val_ >  q.val_; }
inline Rational_Float operator+ (Rational_Float const& p, Rational_Float const& q)
  { return Rational_Float(p.val_ + q.val_); }
inline Rational_Float operator- (Rational_Float const& p, Rational_Float const& q)
  { return Rational_Float(p.val_ - q.val_); }
inline Rational_Float operator* (Rational_Float const& p, Rational_Float const& q)
  { return Rational_Float(p.val_ * q.val_); }
inline Rational_Float operator/ (Rational_Float const& p, Rational_Float const& q)
  { return Rational_Float(p.val_ / q.val_); }
inline std::ostream& operator<< (std::ostream& os, Rational_Float const& p)
  { os << p.val_; return os; }


#ifdef RATIONAL
using Rational = Rational_mGPT;
#else
using Rational = Rational_Float;
#endif

using VecRationals = std::vector<Rational>;

#endif // RATIONAL_H
