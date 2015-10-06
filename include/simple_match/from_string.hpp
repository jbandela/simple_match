#pragma once
#include "simple_match.hpp"

#include <sstream>


namespace simple_match {

    namespace detail {

        template<class Type, class Matcher>
        struct from_string_t {
            Type t_;
            Matcher m_;

            template<class T>
            bool check(T&& t) {
                std::stringstream ss(std::forward<T>(t));
                if (ss >> t_) {
                    if (!ss.eof()) return false;
                    return match_check(t_, m_);
                }
                else {
                    return false;
                }
            }

            template<class T>
            auto get(T&& ) {
                return match_get(t_, m_);
            }

            from_string_t(Matcher m) :m_{ std::forward<Matcher>(m) } {}
        };
        template<class Type>
        struct from_string_t<Type,void> {
            Type t_;

            template<class T>
            bool check(T&& t) {
                std::stringstream ss(std::forward<T>(t));
                if (ss >> t_) {
                    if (!ss.eof()) return false;
                    return true;
                }
                else {
                    return false;
                }
            }

            template<class T>
            auto get(T&& ) {
                return std::tie();
            }
        };


    }

    namespace customization {
        template<class Type,class Type2,class M>
        struct matcher<Type, detail::from_string_t<Type2,M>> {
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
    detail::from_string_t<T, void> from_string() {
        return {};
    }

    template<class T, class Matcher>
    detail::from_string_t<T, Matcher> from_string(Matcher&& m) {
        return{ std::forward<Matcher>(m) };
    }
};
