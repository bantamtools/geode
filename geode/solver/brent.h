#pragma once

// Brent's method for one-dimensional optimization.  The implementation
// is taken from scipy, which requires the following notice:
// ******NOTICE***************
// optimize.py module by Travis E. Oliphant
//
// You may copy and use this module as you see fit with no
// guarantee implied provided you keep this notice in all copies.
// *****END NOTICE************

#include <geode/array/RawArray.h>
#include <geode/structure/Tuple.h>
#include <geode/utility/function.h>
namespace geode {

// Minimize a one-dimensional function using Brent's method.  Unlike scipy, all tolerances are absolute.
// Arguments:
//   bracket: a starting interval (a,b) for bracketing (the solution is not necessarily inside this range)
//   x: starting point
//   xtol: absolute point tolerance
//   ftol: absolute function value tolerance
// x is updated to minimize f(x), and x,f(x),iters is returned.
GEODE_CORE_EXPORT Tuple<real,real,int> brent(const function<real(real)>& f, Vector<real,2> bracket, real xtol, int maxiter);

// Same as above, but specify the full bracketing interval explicitly.  bracket = (a,b,c) must satisfy b in [a,c], f(b) < f(a),f(c).
GEODE_CORE_EXPORT Tuple<real,real,int> brent(const function<real(real)>& f, Vector<real,3> bracket, real xtol, int maxiter);

}
