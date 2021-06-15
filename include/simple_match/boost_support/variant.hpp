// Copyright 2015 John R. Bandela
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#ifndef SIMPLE_MATCH_VARIANT_HPP_JRB_2015_09_11
#  define SIMPLE_MATCH_VARIANT_HPP_JRB_2015_09_11
#  include <boost/variant.hpp>

#  include "simple_match/simple_match.hpp"
namespace simple_match
{
namespace detail
{
template<class Variant>
struct extract_variant_types
{
};

template<class T>
struct extract_single_type
{
  using type = std::tuple<T>;
};

template<class T>
struct extract_single_type<boost::recursive_wrapper<T>>
{
  using type = std::tuple<T>;
};

// NB: dependence on boost::variant implementation detail
template<>
struct extract_single_type<boost::detail::variant::void_>
{
  using type = std::tuple<>;
};

template<class... T>
struct extract_variant_types_helper
{
};

template<class First, class... Rest>
struct extract_variant_types_helper<First, Rest...>
{
  using type =
      cat_tuple_t<typename extract_single_type<First>::type,
                  typename extract_variant_types_helper<Rest...>::type>;
};

template<>
struct extract_variant_types_helper<>
{
  using type = std::tuple<>;
};

template<class Tuple>
struct some_exhaustiveness_variant_generator
{
};

template<class... T>
struct some_exhaustiveness_variant_generator<std::tuple<T...>>
{
  using type = some_exhaustiveness<T...>;
};

template<class... T>
struct extract_variant_types<boost::variant<T...>>
{
  using type = typename extract_variant_types_helper<T...>::type;
};

}  // namespace detail

namespace customization
{
template<class... A>
struct pointer_getter<boost::variant<A...>>
{
  template<class To, class T>
  static auto get_pointer(T&& t)
  {
    return boost::get<To>(&t);
  }
};

template<class... T>
struct exhaustiveness_checker<boost::variant<T...>>
{
  using vtypes =
      typename detail::extract_variant_types<boost::variant<T...>>::type;
  using type =
      typename detail::some_exhaustiveness_variant_generator<vtypes>::type;
};

}  // namespace customization

}  // namespace simple_match

#endif
