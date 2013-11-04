// Partially apply a function object.
// This is essentially a simpler version of boost::bind.
#pragma once

#include <geode/utility/SanitizeFunction.h>
#include <geode/utility/Enumerate.h>
#include <geode/utility/move.h>
#include <geode/structure/Tuple.h>
#include <boost/utility/declval.hpp>
#include <boost/utility/result_of.hpp>
#include <boost/type_traits/add_const.hpp>
#include <boost/type_traits/add_reference.hpp>
namespace geode {

namespace {

#ifdef GEODE_VARIADIC

template<class F,class... Args> struct Curry {
  typedef typename SanitizeFunction<F>::type S;

  const S f;
  const Tuple<Args...> args;

  template<class... Args_> Curry(const F& f, Args_&&... args)
    : f(f), args(args...) {}

  template<class... Rest> GEODE_ALWAYS_INLINE auto operator()(Rest&&... rest) const
    -> decltype(boost::declval<const S&>()(boost::declval<const Args&>()...,rest...)) {
    return call(Enumerate<Args...>(),geode::forward<Rest>(rest)...);
  }

private:
  template<class... Enum,class... Rest> GEODE_ALWAYS_INLINE auto call(Types<Enum...>, Rest&&... rest) const 
    -> decltype(boost::declval<const S&>()(boost::declval<const Args&>()...,rest...)) {
    return f(args.template get<Enum::index>()...,geode::forward<Rest>(rest)...);
  }
};

template<class F,class... Args> static inline Curry<F,typename remove_reference<Args>::type...> curry(const F& f, const Args&... args) {
  return Curry<F,typename remove_reference<Args>::type...>(f,args...);
}

#else // Unpleasant nonvariadic versions

template<class F> static inline const F& curry(const F& f) {
  return f;
}

#define GEODE_CURRY(n,ARGS,Args,fields,Argsargs,init,args,consts) \
  template<class F,GEODE_REMOVE_PARENS(ARGS)> struct Curry##n { \
    typedef typename SanitizeFunction<F>::type S; \
    typedef typename boost::add_reference<typename boost::add_const<S>::type>::type CS; \
    typename boost::add_const<S>::type f; \
    GEODE_REMOVE_PARENS(fields) \
    Curry##n(CS f,GEODE_REMOVE_PARENS(Argsargs)) : f(f),GEODE_REMOVE_PARENS(init) {} \
    typename boost::result_of<S consts>::type operator()() const { return f args; } \
    template<class R0> typename boost::result_of<S(GEODE_REMOVE_PARENS(consts),R0&&)>::type operator()(R0&& r0) const { return f(GEODE_REMOVE_PARENS(args),r0); } \
    template<class R0,class R1> typename boost::result_of<S(GEODE_REMOVE_PARENS(consts),R0&&,R1&&)>::type operator()(R0&& r0,R1&& r1) const { return f(GEODE_REMOVE_PARENS(args),r0,r1); } \
    template<class R0,class R1,class R2> typename boost::result_of<S(GEODE_REMOVE_PARENS(consts),R0&&,R1&&,R2&&)>::type operator()(R0&& r0,R1&& r1,R2&& r2) const { return f(GEODE_REMOVE_PARENS(args),r0,r1,r2); } \
    template<class R0,class R1,class R2,class R3> typename boost::result_of<S(GEODE_REMOVE_PARENS(consts),R0&&,R1&&,R2&&,R3&&)>::type operator()(R0&&r0,R1&&r1,R2&&r2,R3&&r3) const { return f(GEODE_REMOVE_PARENS(args),r0,r1,r2,r3); } \
  }; \
  \
  template<class F,GEODE_REMOVE_PARENS(ARGS)> static inline Curry##n<F,GEODE_REMOVE_PARENS(Args)> curry(const F& f,GEODE_REMOVE_PARENS(Argsargs)) { \
    return Curry##n<F,GEODE_REMOVE_PARENS(Args)>(f,GEODE_REMOVE_PARENS(args)); \
  }

GEODE_CURRY(1,(class A0),(A0),(const A0 a0;),(const A0& a0),(a0(a0)),(a0),(const A0&))
GEODE_CURRY(2,(class A0,class A1),(A0,A1),(const A0 a0;const A1 a1;),(const A0& a0,const A1& a1),(a0(a0),a1(a1)),(a0,a1),(const A0&,const A1&))
GEODE_CURRY(3,(class A0,class A1,class A2),(A0,A1,A2),(const A0 a0;const A1 a1;const A2 a2;),(const A0& a0,const A1& a1,const A2& a2),(a0(a0),a1(a1),a2(a2)),(a0,a1,a2),(const A0&,const A1&,const A2&))
GEODE_CURRY(4,(class A0,class A1,class A2,class A3),(A0,A1,A2,A3),(const A0 a0;const A1 a1;const A2 a2;const A3 a3;),
  (const A0& a0,const A1& a1,const A2& a2,const A3& a3),(a0(a0),a1(a1),a2(a2),a3(a3)),(a0,a1,a2,a3),(const A0&,const A1&,const A2&,const A3&))
GEODE_CURRY(5,(class A0,class A1,class A2,class A3,class A4),(A0,A1,A2,A3,A4),(const A0 a0;const A1 a1;const A2 a2;const A3 a3;const A4 a4;),
  (const A0& a0,const A1& a1,const A2& a2,const A3& a3,const A4& a4),(a0(a0),a1(a1),a2(a2),a3(a3),a4(a4)),(a0,a1,a2,a3,a4),(const A0&,const A1&,const A2&,const A3&,const A4&))
#undef GEODE_CURRY

#endif

}
}
