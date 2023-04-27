#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "rational.h"
#include "../../utils/exceptions.h"
#include "../../utils/die.h"


#ifdef RATIONAL
#pragma message "Using Rational_mGPT as Rational"
//#else
//#pragma message "Using Float as Rational"
#endif


static int gcd(int n, int m) {
  int a = abs(n);
  int b = abs(m);
  while (b > 0) {
    int c = b;
    b = a % b;
    a = c;
  }
  return a;
}

static int lcm(int n, int m) {
  int gcd_n_m = gcd( n, m );
  int lcm_n_m = n / gcd_n_m * m;
//  if (lcm_n_m / m != n / gcd_n_m) {
//    std::cout << lcm_n_m << " / " << m << " = " << (lcm_n_m / m)
//              << " ? "
//              << (n / gcd_n_m) << " = " << n << " / " << gcd_n_m
//              << std::endl;
//  }
  DIE(lcm_n_m / m == n / gcd_n_m, "BUG_0: Rational_mGPT overflow", 200); // BUG_0
  return lcm_n_m;
}

std::pair<int,int> Rational_mGPT::multipliers( int n, int m ) {
  int f = lcm( n, m );
  return( std::make_pair( f/n, f/m ) );
}

Rational_mGPT::Rational_mGPT( int n, int m ) {
//  if( m == 0 )
    //throw Exception( "division by zero" );
    DIE(m != 0, "division by zero", 146);
//  else
//  {
    int d = gcd( n, m );
    numerator_ = n / d;
    denominator_ = m / d;
    if( denominator_ < 0 )
    {
      numerator_ *= -1;
      denominator_ *= -1;
    }
//  }
}

Rational_mGPT::Rational_mGPT( const char* s ) : numerator_(0) {
  const char* si = s;
  for( ; *si != '\0' && *si != '.' && *si != '/'; ++si )
    numerator_ = 10 * numerator_ + (*si - '0');

  if( *si == '/' )
  {
    denominator_ = 0;
    for( ++si; *si != '\0'; ++si )
      denominator_ = 10*denominator_ + (*si - '0');
//    if( denominator_ == 0 )
//      throw Exception( "division by zero" );
    DIE(denominator_ != 0, "division by zero", 146);
    int d = gcd( numerator_, denominator_ );
    numerator_ /= d;
    denominator_ /= d;
  }
  else if( *si == '.' )
  {
    int a = numerator_;
    numerator_ = 0;
    denominator_ = 1;
    for( ++si; *si != '\0'; ++si )
    {
      numerator_ = 10 * numerator_ + (*si - '0');
      denominator_ *= 10;
    }
    int d = gcd( numerator_, denominator_ );
    numerator_ /= d;
    denominator_ /= d;
    numerator_ += a * denominator_;
  }
  else
  {
    denominator_ = 1;
  }
//  std::cout << "[Rational_mGPT] Parsed " << s << " as " << *this << std::endl;
}

void Rational_mGPT::convertDouble(double d, int max_denominator) {
  DIE(max_denominator > 1, "Non-Positive max_denominator", 160);
  DIE(max_denominator % 10 == 0, "Not base 10 max_denominator", 160);
  int num = static_cast<int>(floor(fabs(d)));  // -2.3 -> 2
  if (d < 0)
    num *= -1;
  double f_prime = fabs(d) - fabs(floor(d));  // -2.3 -> 0.3
  int den = 1;
  while (f_prime > 0 && f_prime < 1) {
    if (den >= max_denominator) break;
    den *= 10;
    f_prime *= 10;
    num = 10 * num + floor(f_prime);
    f_prime -= floor(f_prime);
  }
  // Extra slow down, however, it makes sure that whatever assumption of the
  // relation between num and den will be present
  (*this) = Rational_mGPT(num, den);
//  std::cout << "r = " << *this << "\n"
//            << "r.double_value() = " << double_value() << "\n"
//            << "bound: [" << (fabs(f) - 1/(double) max_denominator) << " ,"
//                          << (fabs(f) + 1/(double) max_denominator) << "]\n"
//            << "\n" << std::endl;
  DIE(fabs(double_value()) < fabs(d) + 1/(double) max_denominator &&  // 160
      fabs(double_value()) > fabs(d) - 1/(double) max_denominator,
      "Convertion error" , 160);
}

bool operator<( const Rational_mGPT& q, const Rational_mGPT& p ) {
  std::pair<int,int> m = Rational_mGPT::multipliers( q.denominator(), p.denominator() );
  return( q.numerator() * m.first < p.numerator() * m.second );
}

bool operator<=( const Rational_mGPT& q, const Rational_mGPT& p ) {
  std::pair<int,int> m = Rational_mGPT::multipliers( q.denominator(), p.denominator() );
  return( q.numerator() * m.first <= p.numerator() * m.second );
}

bool operator==( const Rational_mGPT& q, const Rational_mGPT& p ) {
  std::pair<int,int> m = Rational_mGPT::multipliers( q.denominator(), p.denominator() );
  return( q.numerator() * m.first == p.numerator() * m.second );
}

bool operator!=( const Rational_mGPT& q, const Rational_mGPT& p ) {
  std::pair<int,int> m = Rational_mGPT::multipliers( q.denominator(), p.denominator() );
  return( q.numerator() * m.first != p.numerator() * m.second );
}

bool operator>=( const Rational_mGPT& q, const Rational_mGPT& p ) {
  std::pair<int,int> m = Rational_mGPT::multipliers( q.denominator(), p.denominator() );
  return( q.numerator() * m.first >= p.numerator() * m.second );
}

bool operator>( const Rational_mGPT& q, const Rational_mGPT& p ) {
  std::pair<int,int> m = Rational_mGPT::multipliers( q.denominator(), p.denominator() );
  return( q.numerator() * m.first > p.numerator() * m.second );
}

Rational_mGPT operator+( const Rational_mGPT& q, const Rational_mGPT& p ) {
  std::pair<int,int> m = Rational_mGPT::multipliers( q.denominator(), p.denominator() );
  return( Rational_mGPT( q.numerator() * m.first + p.numerator() * m.second,
        q.denominator() * m.first ) );
}

Rational_mGPT operator-( const Rational_mGPT& q, const Rational_mGPT& p ) {
  std::pair<int,int> m = Rational_mGPT::multipliers( q.denominator(), p.denominator() );
  return( Rational_mGPT( q.numerator() * m.first - p.numerator() * m.second,
        q.denominator() * m.first ) );
}

Rational_mGPT operator*( const Rational_mGPT& q, const Rational_mGPT& p ) {
  int d1 = gcd( q.numerator(), p.denominator() );
  int d2 = gcd( p.numerator(), q.denominator() );
  return( Rational_mGPT( (q.numerator()/d1) * (p.numerator()/d2),
        (q.denominator()/d2) * (p.denominator()/d1) ) );
}

Rational_mGPT operator/( const Rational_mGPT& q, const Rational_mGPT& p ) {
  //if( p == 0 ) throw Exception( "division by zero" );
  DIE(p != 0, "division by zero", 146);
  int d1 = gcd( q.numerator(), p.numerator() );
  int d2 = gcd( p.denominator(), q.denominator() );
  return( Rational_mGPT( (q.numerator()/d1) * (p.denominator()/d2),
        (q.denominator()/d2) * (p.numerator()/d1) ) );
}

std::ostream& operator<<( std::ostream& os, const Rational_mGPT& q ) {
  os << q.numerator();
  if( q.denominator() != 1 )
    os << '/' << q.denominator();
  return( os );
}
