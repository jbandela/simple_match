#pragma once
#include "simple_match.hpp"
#include <regex>
#include <array>

namespace simple_match {

    namespace detail {

        // Reuse our apply_impl for array. We can do this because std::get is specialized for array as well as tuple
        template <typename F, typename T, std::size_t N>
        decltype(auto) apply_array(F&& f, const std::array<T,N>& t) {
            using namespace std;
            using Indices = make_index_sequence<N>;
            return apply_impl(std::forward<F>(f), t, Indices{});
        }

        template<class S,class... Ms>
        struct regex_t {
            S s_;
            std::tuple<Ms...,detail::tuple_ignorer> t_;
            enum { len = sizeof...(Ms) };
            std::array<S, len> ar_;

            template<class... M>
            regex_t(S s, M&&... m) :s_{ std::move(s) }, t_{ std::forward<M>(m)...,detail::tuple_ignorer{} } {}

            auto get_tuple() {
                return detail::apply_array([](auto&... a) {return std::tie(a...);}, ar_);
            }
            template<class T>
            bool check(T&& t) {
                using std::begin;
                std::regex r{ s_ };
                std::match_results<decltype(begin(t))> results;

                if (std::regex_match(std::forward<T>(t), results, r)) {
                    // The first sub_match is the whole string; the next
                    // sub_match is the first parenthesized expression.
                    if (results.size() != len + 1) {
                        return false;
                    }
                    else{
                        for (std::size_t i = 1; i <= ar_.size();++i) {
                            ar_[i - 1] = results[i].str();
                        }
                        return match_check(get_tuple(), t_);
                    }
                }
                else {
                    return false;
                }
            }

            template<class T>
            auto get(T&&) {
                return match_get(get_tuple(), t_);
            }

        };

    }

    namespace customization {
        template<class Type, class S, class... M>
        struct matcher<Type, detail::regex_t<S, M...>> {
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


    template<class... M>
    detail::regex_t<std::string, M...> rex(std::string s, M&&... m) { return{ std::move(s),std::forward<M>(m)... }; }





}
