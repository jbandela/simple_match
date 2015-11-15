#pragma once
#include "../simple_match.hpp"

#include <sstream>

#include <boost/lexical_cast.hpp>
namespace simple_match {

    namespace detail {

        template<class Type, class Matcher>
        struct lexical_cast_t {
            Type t_;
            Matcher m_;

            template<class T>
            bool check(T&& t) {
				return boost::conversion::try_lexical_convert(std::forward<T>(t), t_);
            }

            template<class T>
            auto get(T&& ) {
                return match_get(t_, m_);
            }

            lexical_cast_t(Matcher m) :m_{ std::move(m) } {}
        };
        template<class Type>
        struct lexical_cast_t<Type,void> {
            Type t_;

            template<class T>
            bool check(T&& t) {
				return boost::conversion::try_lexical_convert(std::forward<T>(t), t_);
            }

            template<class T>
            auto get(T&& ) {
                return std::tie();
            }
        };


    }

    namespace customization {
        template<class Type,class Type2,class M>
        struct matcher<Type, detail::lexical_cast_t<Type2,M>> {
            template<class T, class U>
            static bool check(T&& t, U&& u) {
                return u.check(std::forward<T>(t));
            }

            template<class T, class U>
            static auto get(T&& t, U&& u ) {
                return u.get(std::forward<T>(t));
            }
        };
    }

    template<class T>
    detail::lexical_cast_t<T, void> lexical_cast() {
        return {};
    }

    template<class T, class Matcher>
    detail::lexical_cast_t<T, Matcher> lexical_cast(Matcher&& m) {
        return{ std::forward<Matcher>(m) };
    }
};
