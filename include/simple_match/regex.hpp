#pragma once
#include <array>
#include <regex>

#include <boost/utility/string_ref.hpp>

#include "simple_match/simple_match.hpp"

namespace simple_match
{
namespace detail
{
// Reuse our apply_impl for array. We can do this because std::get is
// specialized for array as well as tuple
template<typename F, typename T, std::size_t n>
auto apply_array(F&& f, const std::array<T, n>& t) -> decltype(auto)
{
  return apply_impl(std::forward<F>(f), t, std::make_index_sequence<n> {});
}

template<class S>
struct regex_match_t
{
  S s;

  template<class Iter1, class Iter2, class Results>
  auto match_search(Iter1 b, Iter2 e, Results& results) -> bool
  {
    std::basic_regex<typename S::value_type> r {s.data(), s.size()};
    return std::regex_match(b, e, results, r);
  }

  regex_match_t(S&& s)
      : s {std::forward<S>(s)}
  {
  }
};
template<class C, class T>
struct regex_match_t<std::basic_regex<C, T>>
{
  using r_t = std::basic_regex<C, T>;

  const r_t* r;

  template<class Iter1, class Iter2, class Results>
  auto match_search(Iter1 b, Iter2 e, Results& results) -> bool
  {
    return std::regex_match(b, e, results, *r);
  }

  regex_match_t(const r_t& r)
      : r {&r}
  {
  }
};

template<class S>
struct regex_search_t
{
  S s;

  template<class Iter1, class Iter2, class Results>
  auto match_search(Iter1 b, Iter2 e, Results& results) -> bool
  {
    std::basic_regex<typename S::value_type> r {s.data(), s.size()};
    return std::regex_search(b, e, results, r);
  }

  regex_search_t(S&& s)
      : s {std::forward<S>(s)}
  {
  }
};
template<class C, class T>
struct regex_search_t<std::basic_regex<C, T>>
{
  using r_t = std::basic_regex<C, T>;

  const r_t* r;

  template<class Iter1, class Iter2, class Results>
  auto match_search(Iter1 b, Iter2 e, Results& results) -> bool
  {
    return std::regex_search(b, e, results, *r);
  }

  regex_search_t(const r_t& r)
      : r {&r}
  {
  }
};

template<class R, class S, class... Ms>
struct regex_t : private R
{
  std::tuple<Ms..., detail::tuple_ignorer> t;
  enum
  {
    len = sizeof...(Ms)
  };
  std::array<S, len> ar;

  template<class T, class... M>
  regex_t(T&& s, M&&... m)
      : R {std::forward<T>(s)}
      , t {std::forward<M>(m)..., detail::tuple_ignorer {}}
  {
  }

  auto get_tuple()
  {
    return detail::apply_array([](auto&... a) { return std::tie(a...); }, ar);
  }
  template<class U>
  auto check(U&& u) -> bool
  {
    using std::begin;
    std::match_results<decltype(begin(u))> results;

    if (this->match_search(u.begin(), u.end(), results)) {
      // The first sub_match is the whole string; the next
      // sub_match is the first parenthesized expression.
      if (results.size() != len + 1) {
        return false;
      }
      for (std::size_t i = 1; i <= ar.size(); ++i) {
        auto pos = results.position(i);
        auto len = results.length(i);
        ar[i - 1] = S(u.data() + pos, len);
      }
      return match_check(get_tuple(), u);
    }
    return false;
  }

  template<class T>
  auto get(T&& /*unused*/)
  {
    return match_get(get_tuple(), t);
  }
};

}  // namespace detail

namespace customization
{
template<class Type, class R, class S, class... M>
struct matcher<Type, detail::regex_t<R, S, M...>>
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

template<class Char, class... M>
detail::regex_t<detail::regex_match_t<boost::basic_string_ref<Char>>,
                boost::basic_string_ref<Char>,
                M...>
rex_match(boost::basic_string_ref<Char> s, M&&... m)
{
  return {std::move(s), std::forward<M>(m)...};
}

template<class Char, class... M>
detail::regex_t<detail::regex_match_t<boost::basic_string_ref<Char>>,
                boost::basic_string_ref<Char>,
                M...>
rex_match(const Char* s, M&&... m)
{
  return {s, std::forward<M>(m)...};
}

template<class Char, class T, class... M>
detail::regex_t<detail::regex_match_t<std::basic_regex<Char, T>>,
                boost::basic_string_ref<Char>,
                M...>
rex_match(std::basic_regex<Char, T>& r, M&&... m)
{
  return {r, std::forward<M>(m)...};
}

template<class Char, class... M>
detail::regex_t<detail::regex_search_t<boost::basic_string_ref<Char>>,
                boost::basic_string_ref<Char>,
                M...>
rex_search(boost::basic_string_ref<Char> s, M&&... m)
{
  return {std::move(s), std::forward<M>(m)...};
}

template<class Char, class... M>
detail::regex_t<detail::regex_search_t<boost::basic_string_ref<Char>>,
                boost::basic_string_ref<Char>,
                M...>
rex_search(const Char* s, M&&... m)
{
  return {s, std::forward<M>(m)...};
}

template<class Char, class T, class... M>
detail::regex_t<detail::regex_search_t<std::basic_regex<Char, T>>,
                boost::basic_string_ref<Char>,
                M...>
rex_search(std::basic_regex<Char, T>& r, M&&... m)
{
  return {r, std::forward<M>(m)...};
}

}  // namespace simple_match
