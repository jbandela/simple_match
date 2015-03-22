#pragma once
#ifndef SIMPLE_MATCH_SOME_NONE_HPP_JRB_2015_03_21
#define SIMPLE_MATCH_SOME_NONE_HPP_JRB_2015_03_21
#include "simple_match.hpp"
#include <memory>

namespace simple_match {

	namespace detail {

		template<class Type>
		struct pointer_getter {};

		template<class Type>
		struct pointer_getter<Type*> {
			static auto get_pointer(Type* t) {
				return t;
			}
		};

		template<class Type>
		struct pointer_getter<std::shared_ptr<Type>> {
			template<class T>
			static auto get_pointer(T&& t) {
				return t.get();
			}
		};

		template<class Type, class D>
		struct pointer_getter<std::unique_ptr<Type,D>> {
			template<class T>
			static auto get_pointer(T&& t) {
				return t.get();
			}
		};

		template<class Class, class Matcher>
		struct some_t{
			Matcher m_;

			template<class T>
			bool check(T&& t) {
				auto ptr = pointer_getter<std::decay_t<T>>::get_pointer(std::forward<T>(t));
				if (!ptr) {
					return false;
				}
				auto casted_ptr = dynamic_cast<Class*>(ptr);
				if (!casted_ptr) {
					return false;
				}
				return match_check(*casted_ptr, m_);

			}

			template<class T>
			auto get(T&& t) {
				auto casted_ptr = dynamic_cast<Class*>(pointer_getter<std::decay_t<T>>::get_pointer(std::forward<T>(t)));
				return match_get(*casted_ptr, m_);
			}


		};

		template<class Matcher>
		struct some_t<void,Matcher> {
			Matcher m_;

			template<class T>
			bool check(T&& t) {
				auto ptr = pointer_getter<std::decay_t<T>>::get_pointer(std::forward<T>(t));
				if (!ptr) {
					return false;
				}
				return match_check(*ptr, m_);

			}

			template<class T>
			auto get(T&& t) {
				auto ptr = (pointer_getter<std::decay_t<T>>::get_pointer(std::forward<T>(t)));
				return match_get(*ptr, m_);
			}


		};

		template<class Class>
		struct some_t<Class,void> {

			template<class T>
			bool check(T&& t) {
				auto ptr = pointer_getter<std::decay_t<T>>::get_pointer(std::forward<T>(t));
				if (!ptr) {
					return false;
				}
				auto casted_ptr = dynamic_cast<Class*>(ptr);
				if (!casted_ptr) {
					return false;
				}
				return true;

			}

			template<class T>
			auto get(T&& t) {
				auto casted_ptr = dynamic_cast<const Class*>(pointer_getter<std::decay_t<T>>::get_pointer(std::forward<T>(t)));
				return std::tie(*t);
			}


		};

		template<>
		struct some_t<void, void> {

			template<class T>
			bool check(T&& t) {
				auto ptr = pointer_getter<std::decay_t<T>>::get_pointer(std::forward<T>(t));
				if (!ptr) {
					return false;
				}
				return true;

			}

			template<class T>
			auto get(T&& t) {
				auto casted_ptr = pointer_getter<std::decay_t<T>>::get_pointer(std::forward<T>(t));
				return std::tie(*t);
			}


		};


		struct none_t{
			template<class T>
			bool check(T&& t) {
				auto ptr = pointer_getter<std::decay_t<T>>::get_pointer(std::forward<T>(t));
				if (!ptr) {
					return true;
				}
				return false;

			}

			template<class T>
			auto get(T&& t) {
				return std::tie();
			}
		};

	}
	namespace customization {

		template<class Type, class Class, class Matcher>
		struct matcher<Type,detail::some_t<Class,Matcher>> {
			template<class T, class U>
			static bool check(T&& t, U&& u) {
				return u.check(std::forward<T>(t));
			}
			template<class T, class U>
			static auto get(T&& t, U&& u) {
				return u.get(std::forward<T>(t));
			}

		};

		template<class Type>
		struct matcher<Type,detail::none_t> {
			template<class T, class U>
			static bool check(T&& t, U&& u) {
				return u.check(std::forward<T>(t));
			}
			template<class T, class U>
			static auto get(T&& t, U&& u) {
				return u.get(std::forward<T>(t));
			}

		};
	}

	detail::none_t none() { return {}; }

	detail::some_t<void, void> some() { return{}; }

	template<class Matcher>
	detail::some_t<void, Matcher> some(Matcher&& m) { return{ std::forward<Matcher>(m) }; }

	template<class Class, class Matcher>
	detail::some_t<Class, Matcher> some(Matcher&& m) {
		return{ std::forward<Matcher>(m) };
	}
	template<class Class>
	detail::some_t<Class, void> some() {
		return{ };
	}
}



#endif