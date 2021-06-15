// Copyright 2015 John R. Bandela
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#ifndef SIMPLE_MATCH_OPTIONAL_HPP_JRB_2015_09_11
#  define SIMPLE_MATCH_OPTIONAL_HPP_JRB_2015_09_11
#  include <boost/optional.hpp>

#  include "simple_match/simple_match.hpp"
namespace simple_match
{
namespace customization
{
template<class Type>
struct pointer_getter<boost::optional<Type>>
{
  template<class T>
  static auto get_pointer_no_cast(T&& t)
  {
    return t.get_ptr();
  }
  template<class T>
  static auto is_null(T&& t)
  {
    return !t;
  }
};
}  // namespace customization
}  // namespace simple_match

#endif
