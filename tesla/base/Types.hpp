/*
 *
 * Author
 *     Thomas Liu
 * Date
 *     4/17/2014
 * Description
 *     - Provide implicit_cast<>, down_cast<>
 *     - Provide __gnu_cxx::__sso_string(short string optimization) as a replacement
 *     - Copied from protobuf and boost.
 *
 * Licensed under the Apache License, Version 2.0
 *
 */

#ifndef TESLA_BASE_TYPES_HPP
#define TESLA_BASE_TYPES_HPP

#include <stdint.h>
#ifdef TESLA_USE_STD_STRING
#include <string>
#else
#include <ext/vstring.h>
#include <ext/vstring_fwd.h>
#endif // TESLA_USE_STD_STRING

#ifndef NDEBUG
#include <assert.h>
#endif // NDEBUG

namespace tesla
{

namespace base
{

#ifdef TESLA_USE_STD_STRING
using std::string;
#else
typedef __gnu_cxx::__sso_string string;
#endif // TESLA_USE_STD_STRING

// Use implicit_cast as a safe version of static_cast or const_cast
// for upcasting in the type hierarchy (i.e. casting a pointer to Foo
// to a pointer to SuperclassOfFoo or casting a pointer to Foo to
// a const pointer to Foo).
// When you use implicit_cast, the compiler checks that the cast is safe.
// Such explicit implicit_casts are necessary in surprisingly many
// situations where C++ demands an exact type match instead of an
// argument type convertable to a target type.
//
// The From type can be inferred, so the preferred syntax for using
// implicit_cast is the same as for static_cast etc.:
//
//   implicit_cast<ToType>(expr)
//
// implicit_cast would have been part of the C++ standard library,
// but the proposal was submitted too late.  It will probably make
// its way into the language in the future.

template<typename To, typename From>
inline To implicit_cast(From const &f)
{
    return f;
}

// When you upcast (that is, cast a pointer from type Foo to type
// SuperclassOfFoo), it's fine to use implicit_cast<>, since upcasts
// always succeed.  When you downcast (that is, cast a pointer from
// type Foo to type SubclassOfFoo), static_cast<> isn't safe, because
// how do you know the pointer is really of type SubclassOfFoo?  It
// could be a bare Foo, or of type DifferentSubclassOfFoo.  Thus,
// when you downcast, you should use this macro.  In debug mode, we
// use dynamic_cast<> to double-check the downcast is legal (we die
// if it's not).  In normal mode, we do the efficient static_cast<>
// instead.  Thus, it's important to test in debug mode to make sure
// the cast is legal!
//    This is the only place in the code we should use dynamic_cast<>.
// In particular, you SHOULDN'T be using dynamic_cast<> in order to
// do RTTI (eg code like this:
//    if (dynamic_cast<Subclass1>(foo)) HandleASubclass1Object(foo);
//    if (dynamic_cast<Subclass2>(foo)) HandleASubclass2Object(foo);
// You should design the code some other way not to need this.

template<typename To, typename From>     // use like this: down_cast<T*>(foo);
inline To down_cast(From* f)             // so we only accept pointers
{
    // Ensures that To is a sub-type of From *.  This test is here only
    // for compile-time type checking, and has no overhead in an
    // optimized build at run-time, as it will be optimized away
    // completely.
    if (false)
    {
        implicit_cast<From*, To>(0);
    }

#if !defined(NDEBUG) && !defined(GOOGLE_PROTOBUF_NO_RTTI)
    assert(f == NULL || dynamic_cast<To>(f) != NULL);  // RTTI: debug mode only!
#endif // NDEBUG || GOOGLE_PROTOBUF_NO_RTTI
    return static_cast<To>(f);
}

} // namespace base

} // namespace tesla

#endif // TESLA_BASE_TYPES_HPP