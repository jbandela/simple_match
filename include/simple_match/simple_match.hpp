// Copyright 2015 John R. Bandela
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#ifndef SIMPLE_MATCH_HPP_JRB_2015_03_21
#define SIMPLE_MATCH_HPP_JRB_2015_03_21

#include <stdexcept>
#include <tuple>

namespace simple_match {
	using std::size_t;
	namespace customization {
		template<class T, class U>
		struct matcher;

	}	
	
	namespace detail {
		template<typename F, typename Tuple, size_t... I>
		auto
			apply_(F&& f, Tuple&& args, std::integer_sequence<size_t, I...>)

		{
			return std::forward<F>(f)(std::get<I>(std::forward<Tuple>(args))...);
		}

		template<typename F, typename Tuple>
		auto
			apply(F&& f, Tuple&& args)

		{
			return apply_(std::forward<F>(f), std::forward<Tuple>(args), std::make_index_sequence<std::tuple_size<typename std::remove_reference<Tuple>::type>::value >());
		}



	}

	template<class T, class U>
	bool match_check(T&& t, U&& u) {
		using namespace customization;
		using m = matcher<std::decay_t<T>, std::decay_t<U>>;
		return m::check(std::forward<T>(t), std::forward<U>(u));
	}


	template<class T, class U>
	auto match_get(T&& t, U&& u) {
		using namespace customization;
		using m = matcher<std::decay_t<T>, std::decay_t<U>>;
		return m::get(std::forward<T>(t), std::forward<U>(u));
	}

	template<class T, class A1, class F1>
	auto match(T&& t, A1&& a, F1&& f) {
		if (match_check(std::forward<T>(t), std::forward<A1>(a))) {
			return detail::apply(f, match_get(std::forward<T>(t), std::forward<A1>(a)));
		}
		else {
			throw std::logic_error("No match");
		}
	}


	template<class T, class A1, class F1, class A2, class F2, class... Args>
	auto match(T&& t, A1&& a, F1&& f, A2&& a2, F2&& f2, Args&&... args) {
		if (match_check(t, a)) {
			return detail::apply(f, match_get(std::forward<T>(t), std::forward<A1>(a)));
		}
		else {
			return match(t, std::forward<A2>(a2), std::forward<F2>(f2), std::forward<Args>(args)...);
		}
	}




	struct otherwise_t {};

	namespace placeholders {
		const otherwise_t otherwise{};
		const otherwise_t _{};
	}

	namespace customization {

		// Match same type
		template<class T>
		struct matcher<T, T> {
			static bool check(const T& t, const T& v) {
				return t == v;
			}
			static auto get(const T&, const T&) {
				return std::tie();
			}

		};
		// Match string literals
		template<class T>
		struct matcher<T, const char*> {
			static bool check(const T& t, const char* str) {
				return t == str;
			}
			static auto get(const T&, const T&) {
				return std::tie();
			}
		};




		// Match otherwise
		template<class Type>
		struct matcher<Type, otherwise_t> {
			template<class T>
			static bool check(T&&, otherwise_t) {
				return true;
			}
			template<class T>
			static auto get(T&&, otherwise_t) {
				return std::tie();
			}

		};



	}
	template<class F>
	struct matcher_predicate {
		F f_;
	};

	template<class F>
	matcher_predicate<F> make_matcher_predicate(F&& f) {
		return matcher_predicate<F>{std::forward<F>(f)};
	}


	namespace customization {


		template<class Type, class F>
		struct matcher<Type, matcher_predicate<F>> {
			template<class T, class U>
			static bool check(T&& t, U&& u) {
				return u.f_(std::forward<T>(t));
			}
			template<class T, class U>
			static auto get(T&& t, U&&) {
				return std::tie(std::forward<T>(t));
			}

		};

	}

	namespace placeholders {
		const auto _x = make_matcher_predicate([](auto&&) {return true; });

		// relational operators
		template<class F, class T>
		auto operator==(const matcher_predicate<F>& m, const T& t) {
			return make_matcher_predicate([m, &t](const auto& x) {return m.f_(x) && x == t; });
		}

		template<class F, class T>
		auto operator!=(const matcher_predicate<F>& m, const T& t) {
			return make_matcher_predicate([m, &t](const auto& x) {return m.f_(x) && x != t; });
		}

		template<class F, class T>
		auto operator<=(const matcher_predicate<F>& m, const T& t) {
			return make_matcher_predicate([m, &t](const auto& x) {return m.f_(x) && x <= t; });
		}
		template<class F, class T>
		auto operator>=(const matcher_predicate<F>& m, const T& t) {
			return make_matcher_predicate([m, &t](const auto& x) {return m.f_(x) && x >= t; });
		}
		template<class F, class T>
		auto operator<(const matcher_predicate<F>& m, const T& t) {
			return make_matcher_predicate([m, &t](const auto& x) {return m.f_(x) && x < t; });
		}
		template<class F, class T>
		auto operator>(const matcher_predicate<F>& m, const T& t) {
			return make_matcher_predicate([m, &t](const auto& x) {return m.f_(x) && x > t; });
		}
		template<class F>
		auto operator!(const matcher_predicate<F>& m) {
			return make_matcher_predicate([m](const auto& x) {return !m.f_(x); });
		}

		template<class F, class F2>
		auto operator&&(const matcher_predicate<F>& m, const matcher_predicate<F2>& m2) {
			return make_matcher_predicate([m, m2](const auto& x) {return m.f_(x) && m2.f_(x); });
		}

		template<class F, class F2>
		auto operator||(const matcher_predicate<F>& m, const matcher_predicate<F2>& m2) {
			return make_matcher_predicate([m, m2](const auto& x) {return m.f_(x) || m2.f_(x); });
		}

		template<class F, class T>
		auto operator==(const T& t, const matcher_predicate<F>& m) {
			return make_matcher_predicate([m, &t](const auto& x) {return m.f_(x) && t == x; });
		}

		template<class F, class T>
		auto operator!=(const T& t, const matcher_predicate<F>& m) {
			return make_matcher_predicate([m, &t](const auto& x) {return m.f_(x) && t != x; });
		}

		template<class F, class T>
		auto operator<=(const T& t, const matcher_predicate<F>& m) {
			return make_matcher_predicate([m, &t](const auto& x) {return m.f_(x) && t <= x; });
		}
		template<class F, class T>
		auto operator>=(const T& t, const matcher_predicate<F>& m) {
			return make_matcher_predicate([m, &t](const auto& x) {return m.f_(x) && t >= x; });
		}
		template<class F, class T>
		auto operator<(const T& t, const matcher_predicate<F>& m) {
			return make_matcher_predicate([m, &t](const auto& x) {return m.f_(x) && t < x; });
		}
		template<class F, class T>
		auto operator>(const T& t, const matcher_predicate<F>& m) {
			return make_matcher_predicate([m, &t](const auto& x) {return m.f_(x) && t > x; });
		}

	}

	namespace detail {
		struct tuple_ignorer {};
	}

	namespace customization {

		template<class Type>
		struct tuple_adapter {

			enum { tuple_len = std::tuple_size<Type>::value };

			template<size_t I, class T>
			static decltype(auto) get(T&& t) {
				return std::get<I>(std::forward<T>(t));
			}
		};

		template<class Type, class... Args>
		struct matcher<Type, std::tuple<Args...>> {
			using tu = tuple_adapter<Type>;
			enum { tuple_len = sizeof... (Args) - 1};
			template<size_t pos, size_t last>
			struct helper {
				template<class T, class A>
				static bool check(T&& t, A&& a) {
					return match_check(tu::template get<pos>(std::forward<T>(t)), std::get<pos>(std::forward<A>(a)))
						&& helper<pos + 1, last>::check(std::forward<T>(t), std::forward<A>(a));

				}

				template<class T, class A>
				static auto get(T&& t, A&& a) {
					return std::tuple_cat(match_get(tu::template get<pos>(std::forward<T>(t)), std::get<pos>(std::forward<A>(a))),
						helper<pos + 1, last>::get(std::forward<T>(t), std::forward<A>(a)));

				}
			};

			template<size_t pos>
			struct helper<pos, pos> {
				template<class T, class A>
				static bool check(T&& t, A&& a) {
					return match_check(tu::template get<pos>(std::forward<T>(t)), std::get<pos>(std::forward<A>(a)));

				}
				template<class T, class A>
				static auto get(T&& t, A&& a) {
					return match_get(tu::template get<pos>(std::forward<T>(t)), std::get<pos>(std::forward<A>(a)));

				}
			};


			template<class T, class A>
			static bool check(T&& t, A&& a) {
				return helper<0, tuple_len - 1>::check(std::forward<T>(t), std::forward<A>(a));

			}
			template<class T, class A>
			static auto get(T&& t, A&& a) {
				return helper<0, tuple_len - 1>::get(std::forward<T>(t), std::forward<A>(a));

			}

		};


	}

	template<class... A>
	auto tup(A&& ... a) {
		return std::make_tuple(std::forward<A>(a)..., detail::tuple_ignorer{});
	}
}



#endif
