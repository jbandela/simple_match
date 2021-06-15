#pragma once
#include <sstream>

#include <boost/lexical_cast.hpp>

#include "simple_match/simple_match.hpp"
namespace simple_match
{
namespace detail
{
template<class Type, class Matcher>
struct lexical_cast_t
{
  Type t;
  Matcher m;

  template<class U>
  auto check(U&& u) -> bool
  {
    return boost::conversion::try_lexical_convert(std::forward<U>(u), t);
  }

  template<class T>
  auto get(T&& /*unused*/)
  {
    return match_get(t, m);
  }

  lexical_cast_t(Matcher m)
      : m {std::move(m)}
  {
  }
};
template<class Type>
struct lexical_cast_t<Type, void>
{
  Type t;

  template<class U>
  auto check(U&& u) -> bool
  {
    return boost::conversion::try_lexical_convert(std::forward<U>(u), t);
  }

  template<class T>
  auto get(T&& /*unused*/)
  {
    return std::tie();
  }
};

}  // namespace detail

namespace customization
{
template<class Type, class Type2, class M>
struct matcher<Type, detail::lexical_cast_t<Type2, M>>
{
  template<class T, class U>
  static auto check(T&& t, U&& u) -> bool
  {
    return u.check(std::forward<T>(t));
  }

  template<class T, class U>
  static auto get(T&& t, U&& u)
  {
    return u.get(std::forward<T>(t));
  }
};
}  // namespace customization

template<class T>
auto lexical_cast() -> detail::lexical_cast_t<T, void>
{
  return {};
}

template<class T, class Matcher>
auto lexical_cast(Matcher&& m) -> detail::lexical_cast_t<T, Matcher>
{
  return {std::forward<Matcher>(m)};
}
}  // namespace simple_match
