// Copyright 2015 John R. Bandela
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#ifndef SIMPLE_MATCH_VARIANT_HPP_JRB_2015_09_11
#define SIMPLE_MATCH_VARIANT_HPP_JRB_2015_09_11
#include <boost/variant.hpp>
#include "../simple_match.hpp"
namespace simple_match {
	namespace customization {
		template<class... A>
		struct pointer_getter<boost::variant<A...>> {
			template<class To, class T>
			static auto get_pointer(T&& t) {
				return boost::get<To>(&t);
			}
		};
	}

}


#endif
