#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <tuple>
#include <utility>

#include "simple_match/boost_support/any.hpp"
#include "simple_match/boost_support/lexical_cast.hpp"
#include "simple_match/boost_support/optional.hpp"
#include "simple_match/boost_support/variant.hpp"
#include "simple_match/regex.hpp"
#include "simple_match/simple_match.hpp"
#include "simple_match/utility.hpp"

struct point
{
  int x;
  int y;
  point(int x_, int y_)
      : x(x_)
      , y(y_)
  {
  }
};

auto simple_match_get_tuple(const point& p)
{
  return std::tie(p.x, p.y);
}

struct holder
{
  virtual ~holder() {}
};
template<class T>
struct holder_t : holder
{
  T value_;

  holder_t(T v)
      : value_ {std::move(v)}
  {
  }
};

template<class T>
auto simple_match_get_tuple(const holder_t<T>& h)
{
  return std::tie(h.value_);
}

template<class T>
std::unique_ptr<holder> make_holder(T&& t)
{
  return std::make_unique<holder_t<std::decay_t<T>>>(std::forward<T>(t));
}

void test_holder()
{
  using namespace simple_match;
  using namespace simple_match::placeholders;

  auto m = [](auto&& v)
  {
    match(
        v,
        some<holder_t<int>>(ds(5)),
        []() { std::cout << "Got five\n"; },
        some<holder_t<int>>(ds(_x)),
        [](auto x) { std::cout << "Got int " << x << "\n"; },
        some(),
        [](auto&) { std::cout << "Got some other type of holder\n"; },
        none(),
        []() { std::cout << "Got nullptr\n"; });
  };
  auto five = make_holder(5);
  auto ten = make_holder(10);
  auto pi = make_holder(3.14);
  std::unique_ptr<holder> nothing;
  m(five);
  m(ten);
  m(pi);
  m(nothing);
}

void test_any()
{
  using namespace simple_match;
  using namespace simple_match::placeholders;

  auto m = [](auto&& v)
  {
    match(
        v,
        some<int>(5),
        []() { std::cout << "Got five\n"; },
        some<int>(),
        [](auto x) { std::cout << "Got int " << x << "\n"; },
        none(),
        []() { std::cout << "Got nullptr\n"; },
        _,
        []() { std::cout << "Got some other type of any\n"; });
  };
  auto five = boost::any {5};
  auto ten = boost::any {10};
  auto pi = boost::any {3.14};
  boost::any nothing;
  m(five);
  m(ten);
  m(pi);
  m(nothing);
}

void test_some_none()
{
  using namespace simple_match;
  using namespace simple_match::placeholders;

  std::unique_ptr<int> nothing;
  auto five = std::make_unique<int>(5);
  auto ten = std::make_unique<int>(10);
  auto twelve = std::make_unique<int>(12);

  auto m = [](auto&& v)
  {
    match(
        v,
        some(5),
        []() { std::cout << "five\n"; },
        some(11 <= _x <= 20),
        [](int x) { std::cout << x << " is on the range [11,20] \n"; },
        some(),
        [](int x) { std::cout << x << "\n"; },
        none(),
        []() { std::cout << "Nothing\n"; });
  };

  m(nothing.get());
  m(five.get());
  m(ten.get());
  m(twelve.get());

  m(nothing);
  m(five);
  m(ten);
  m(twelve);
}

void test_ds()
{
  using namespace simple_match;
  using namespace simple_match::placeholders;

  auto m = [](auto&& v)
  {
    match(
        v,
        some(ds(1, 2)),
        []() { std::cout << "one,two\n"; },
        some(ds(_x, _y)),
        [](int x, int y) { std::cout << x << " " << y << "\n"; },
        none(),
        []() { std::cout << "Nothing\n"; });
  };

  auto tup_12 = std::make_unique<std::tuple<int, int>>(1, 2);
  auto point_12 = std::make_unique<point>(point {1, 2});
  auto point_34 = std::make_unique<point>(point {3, 4});

  m(tup_12);
  m(point_12);
  m(point_34);
}

void test_string()
{
  std::string s = "Test";

  using namespace simple_match;
  using namespace simple_match::placeholders;
  match(
      s,

      "One",
      []() { std::cout << "one\n"; },
      "Test",
      []() { std::cout << "two \n"; },
      otherwise,
      []() { std::cout << "did not match \n"; });
}

struct add;
struct sub;
struct neg;
struct mul;

using math_variant_t = boost::variant<boost::recursive_wrapper<add>,
                                      boost::recursive_wrapper<sub>,
                                      boost::recursive_wrapper<neg>,
                                      boost::recursive_wrapper<mul>,
                                      int>;

struct add
{
  math_variant_t left;
  math_variant_t right;
};
struct sub
{
  math_variant_t left;
  math_variant_t right;
};
struct neg
{
  math_variant_t value;
};
struct mul
{
  math_variant_t left;
  math_variant_t right;
};

struct eval_t
{
};
using eval_any = simple_match::tagged_any<eval_t>;

struct add_tag
{
};
struct sub_tag
{
};
struct mul_tag
{
};
struct neg_tag
{
};

using add2 = simple_match::tagged_tuple<add_tag, eval_any, eval_any>;
using sub2 = simple_match::tagged_tuple<sub_tag, eval_any, eval_any>;
using mul2 = simple_match::tagged_tuple<mul_tag, eval_any, eval_any>;
using neg2 = simple_match::tagged_tuple<neg_tag, eval_any>;

struct add3;
struct sub3;
struct neg3;
struct mul3;

using math_variant2_t = boost::variant<boost::recursive_wrapper<add3>,
                                       boost::recursive_wrapper<sub3>,
                                       boost::recursive_wrapper<neg3>,
                                       boost::recursive_wrapper<mul3>,
                                       int>;

struct add3 : std::tuple<math_variant2_t, math_variant2_t>
{
  using tuple::tuple;
};
struct sub3 : std::tuple<math_variant2_t, math_variant2_t>
{
  using tuple::tuple;
};
struct mul3 : std::tuple<math_variant2_t, math_variant2_t>
{
  using tuple::tuple;
};
struct neg3 : std::tuple<math_variant2_t>
{
  using tuple::tuple;
};

int eval(const math_variant_t& m)
{
  using namespace simple_match;
  using namespace simple_match::placeholders;

  return simple_match::match(
      m,
      some<add>(),
      [](auto&& a) { return eval(a.left) + eval(a.right); },
      some<sub>(),
      [](auto&& a) { return eval(a.left) - eval(a.right); },
      some<neg>(),
      [](auto&& a) { return -eval(a.value); },
      some<mul>(),
      [](auto&& a) { return eval(a.left) * eval(a.right); },
      some<int>(),
      [](auto a) { return a; }

  );
}
namespace simple_match
{
namespace customization
{
template<>
struct exhaustiveness_checker<eval_any>
{
  using type = some_exhaustiveness<add2, sub2, mul2, neg2, int>;
};
}  // namespace customization
}  // namespace simple_match

int eval(const eval_any& m)
{
  using namespace simple_match;
  using namespace simple_match::placeholders;

  return simple_match::match(
      m,
      some<add2>(ds(_x, _y)),
      [](auto&& x, auto&& y) { return eval(x) + eval(y); },
      some<sub2>(ds(_x, _y)),
      [](auto&& x, auto&& y) { return eval(x) - eval(y); },
      some<neg2>(ds(_x)),
      [](auto&& x) { return -eval(x); },
      some<mul2>(ds(_x, _y)),
      [](auto&& x, auto&& y) { return eval(x) * eval(y); },
      some<int>(),
      [](auto x) { return x; }

  );
}

int eval(const math_variant2_t& m)
{
  using namespace simple_match;
  using namespace simple_match::placeholders;

  return simple_match::match(
      m,
      some<add3>(
          ds(some<int>(_x), some<mul3>(ds(some<int>(_y), some<int>(_z))))),
      [](int x, int y, int z)
      {
        std::cout << "Fused add-mul\n";
        return x + y * z;
      },
      some<add3>(ds(_x, _y)),
      [](auto&& x, auto&& y) { return eval(x) + eval(y); },
      some<sub3>(ds(_x, _y)),
      [](auto&& x, auto&& y) { return eval(x) - eval(y); },
      some<mul3>(ds(_x, _y)),
      [](auto&& x, auto&& y) { return eval(x) * eval(y); },
      some<neg3>(ds(_x)),
      [](auto&& x) { return -eval(x); },
      some<int>(),
      [](auto x) { return x; });
}

struct eval_base_t
{
  virtual ~eval_base_t() {}
};
struct int_holder_tag
{
};

using add4 =
    simple_match::inheriting_tagged_tuple<eval_base_t,
                                          add_tag,
                                          std::unique_ptr<eval_base_t>,
                                          std::unique_ptr<eval_base_t>>;
using sub4 =
    simple_match::inheriting_tagged_tuple<eval_base_t,
                                          sub_tag,
                                          std::unique_ptr<eval_base_t>,
                                          std::unique_ptr<eval_base_t>>;
using mul4 =
    simple_match::inheriting_tagged_tuple<eval_base_t,
                                          mul_tag,
                                          std::unique_ptr<eval_base_t>,
                                          std::unique_ptr<eval_base_t>>;
using neg4 = simple_match::
    inheriting_tagged_tuple<eval_base_t, neg_tag, std::unique_ptr<eval_base_t>>;
using int_holder =
    simple_match::inheriting_tagged_tuple<eval_base_t, int_holder_tag, int>;

namespace simple_match
{
namespace customization
{
template<>
struct exhaustiveness_checker<std::unique_ptr<eval_base_t>>
{
  using type = some_exhaustiveness<add4, sub4, mul4, neg4, int_holder>;
};
}  // namespace customization
}  // namespace simple_match

auto eval(const std::unique_ptr<eval_base_t>& m) -> int
{
  using namespace simple_match;
  using namespace simple_match::placeholders;

  return simple_match::match(
      m,
      some<add4>(ds(_x, _y)),
      [](auto&& x, auto&& y) { return eval(x) + eval(y); },
      some<sub4>(ds(_x, _y)),
      [](auto&& x, auto&& y) { return eval(x) - eval(y); },
      some<mul4>(ds(_x, _y)),
      [](auto&& x, auto&& y) { return eval(x) * eval(y); },
      some<neg4>(ds(_x)),
      [](auto&& x) { return -eval(x); },
      some<int_holder>(ds(_x)),
      [](auto x) { return x; });
}

void FizzBuzz()
{
  using namespace simple_match;
  using namespace simple_match::placeholders;
  for (int i = 1; i <= 100; ++i) {
    match(
        std::make_tuple(i % 3, i % 5),
        ds(0, 0),
        []() { std::cout << "FizzBuzz\n"; },
        ds(0, _),
        []() { std::cout << "Fizz\n"; },
        ds(_, 0),
        []() { std::cout << "Buzz\n"; },
        _,
        [i]() { std::cout << i << "\n"; });
  }
}

// Adapted from
// https://github.com/solodon4/Mach7/blob/master/code/test/unit/cppcon-matching.cpp

struct VarExp;
struct NotExp;
struct AndExp;
struct OrExp;

using BoolExp = boost::variant<boost::recursive_wrapper<VarExp>,
                               bool,
                               boost::recursive_wrapper<NotExp>,
                               boost::recursive_wrapper<AndExp>,
                               boost::recursive_wrapper<OrExp>>;

struct VarExp : std::tuple<std::string>
{
  using tuple::tuple;
};
struct NotExp : std::tuple<BoolExp>
{
  using tuple::tuple;
};
struct AndExp : std::tuple<BoolExp, BoolExp>
{
  using tuple::tuple;
};
struct OrExp : std::tuple<BoolExp, BoolExp>
{
  using tuple::tuple;
};

template<class T, class... U>
auto make_bool_exp(U&&... u) -> BoolExp
{
  return T {std::forward<U>(u)...};
}

void print(const BoolExp& exp)
{
  using namespace simple_match;
  using namespace simple_match::placeholders;

  match(
      exp,
      some<VarExp>(ds(_x)),
      [](auto& x) { std::cout << x; },
      some<bool>(),
      [](auto& x) { std::cout << x; },
      some<NotExp>(ds(_x)),
      [](auto& x)
      {
        std::cout << '!';
        print(x);
      },
      some<AndExp>(ds(_x, _y)),
      [](auto& x, auto& y)
      {
        std::cout << '(';
        print(x);
        std::cout << " & ";
        print(y);
        std::cout << ')';
      },
      some<OrExp>(ds(_x, _y)),
      [](auto& x, auto& y)
      {
        std::cout << '(';
        print(x);
        std::cout << " | ";
        print(y);
        std::cout << ')';
      });
}

auto copy(const BoolExp& exp) -> BoolExp
{
  return exp;
}

#include <map>

template<class K, class T, class C, class A>
auto operator<<(std::ostream& os, const std::map<K, T, C, A>& m)
    -> std::ostream&
{
  for (auto&& p : m) {
    os << p.first << '=' << p.second << std::endl;
  }

  return os;
}
typedef std::map<std::string, bool> Context;

auto eval(const Context& ctx, const BoolExp& exp) -> bool
{
  using namespace simple_match;
  using namespace simple_match::placeholders;

  return match(
      exp,
      some<VarExp>(ds(_x)),
      [&](auto& x)
      {
        auto iter = ctx.find(x);
        return !(iter == ctx.end()) && iter->second;
      },
      some<bool>(),
      [](auto& x) { return x; },
      some<NotExp>(ds(_x)),
      [&](auto& x) { return !eval(ctx, x); },
      some<AndExp>(ds(_x, _y)),
      [&](auto& x, auto& y) { return eval(ctx, x) && eval(ctx, y); },
      some<OrExp>(ds(_x, _y)),
      [&](auto& x, auto& y) { return eval(ctx, x) || eval(ctx, y); });
}

auto replace(const BoolExp& where, const std::string& what, const BoolExp& with)
    -> BoolExp
{
  using namespace simple_match;
  using namespace simple_match::placeholders;

  return match(
      where,
      some<VarExp>(ds(_x)),
      [&](auto& x)
      {
        if (what == x) {
          return copy(with);
        }

        return make_bool_exp<VarExp>(x);
      },
      some<bool>(),
      [&](auto&) { return copy(where); },
      some<NotExp>(ds(_x)),
      [&](auto& x) { return make_bool_exp<NotExp>(replace(x, what, with)); },
      some<AndExp>(ds(_x, _y)),
      [&](auto& x, auto& y)
      {
        return make_bool_exp<AndExp>(replace(x, what, with),
                                     replace(y, what, with));
      },
      some<OrExp>(ds(_x, _y)),
      [&](auto& x, auto& y)
      {
        return make_bool_exp<OrExp>(replace(x, what, with),
                                    replace(y, what, with));
      });
}

auto inplace(BoolExp& where, const std::string& what, const BoolExp& with)
    -> BoolExp&
{
  using namespace simple_match;
  using namespace simple_match::placeholders;

  match(
      where,
      some<VarExp>(ds(_x)),
      [&](auto& x) mutable { x == what ? where = with : where; },
      some<bool>(),
      [&](auto&) mutable {},
      some<NotExp>(ds(_x)),
      [&](auto& x) mutable { inplace(x, what, with); },
      some<AndExp>(ds(_x, _y)),
      [&](auto& x, auto& y) mutable
      {
        inplace(x, what, with);
        inplace(y, what, with);
      },
      some<OrExp>(ds(_x, _y)),
      [&](auto& x, auto& y) mutable
      {
        inplace(x, what, with);
        inplace(y, what, with);
      });
  return where;
}

auto equal(const BoolExp& x1, const BoolExp& x2) -> bool
{
  using namespace simple_match;
  using namespace simple_match::placeholders;

  return match(
      std::tie(x1, x2),
      ds(some<VarExp>(ds(_x)), some<VarExp>(ds(_y))),
      [](auto& x, auto& y) { return x == y; },
      ds(some<bool>(), some<bool>()),
      [](auto& x, auto& y) { return x == y; },
      ds(some<NotExp>(ds(_x)), some<NotExp>(ds(_y))),
      [](auto& x, auto& y) { return equal(x, y); },
      ds(some<AndExp>(ds(_w, _x)), some<AndExp>(ds(_y, _z))),
      [](auto& w, auto& x, auto& y, auto& z)
      { return equal(w, y) && equal(x, z); },
      ds(some<OrExp>(ds(_w, _x)), some<OrExp>(ds(_y, _z))),
      [](auto& w, auto& x, auto& y, auto& z)
      { return equal(w, y) && equal(x, z); },
      _,
      []() { return false; }

  );
}

typedef std::map<std::string, BoolExp> Assignments;

// Other example: unify
auto exp_match(const BoolExp& p, const BoolExp& x, Assignments& ctx) -> bool
{
  using namespace simple_match;
  using namespace simple_match::placeholders;

  return match(
      std::tie(p, x),
      ds(some<VarExp>(ds(_x)), _),
      [&](auto& name) mutable
      {
        if (ctx.count(name) == 0) {
          ctx[name] = x;
          return true;
        } else {
          return equal(ctx[name], x);
        }
      },
      ds(some<bool>(), some<bool>()),
      [&](auto& x, auto& y) mutable { return x == y; },
      ds(some<NotExp>(ds(_x)), some<NotExp>(ds(_y))),
      [&](auto& x, auto& y) mutable { return exp_match(x, y, ctx); },
      ds(some<AndExp>(ds(_w, _x)), some<AndExp>(ds(_y, _z))),
      [&](auto& w, auto& x, auto& y, auto& z) mutable
      { return exp_match(w, y, ctx) && exp_match(x, z, ctx); },
      ds(some<OrExp>(ds(_w, _x)), some<OrExp>(ds(_y, _z))),
      [&](auto& w, auto& x, auto& y, auto& z) mutable
      { return exp_match(w, y, ctx) && exp_match(x, z, ctx); },
      _,
      []() { return false; }

  );
}
void TestBoolExp()
{
  BoolExp exp1 = AndExp(OrExp(VarExp("X"), VarExp("Y")), NotExp(VarExp("Z")));

  std::cout << "exp1 = ";
  print(exp1);
  std::cout << std::endl;

  auto exp2 = copy(exp1);

  std::cout << "exp2 = ";
  print(exp2);
  std::cout << std::endl;

  auto exp3 = replace(exp1, "Z", exp2);

  std::cout << "exp3 = ";
  print(exp3);
  std::cout << std::endl;

  auto& exp4 = inplace(exp1, "Z", exp2);

  std::cout << "exp4 = ";
  print(exp4);
  std::cout << std::endl;
  std::cout << "exp1 = ";
  print(exp1);
  std::cout << " updated! " << std::endl;

  std::cout << (equal(exp1, exp2) ? "exp1 == exp2" : "exp1 <> exp2")
            << std::endl;

  Context ctx;
  ctx["Y"] = true;
  std::cout << eval(ctx, exp1) << std::endl;
  std::cout << eval(ctx, exp2) << std::endl;
  std::cout << eval(ctx, exp3) << std::endl;

  std::cout << ctx << std::endl;

  Assignments ctx2;

  if (exp_match(exp2, exp3, ctx2)) {
    std::cout << "exp2 matches exp3 with assignments: " << std::endl;

    for (Assignments::const_iterator p = ctx2.begin(); p != ctx2.end(); ++p) {
      std::cout << p->first << '=';
      print(p->second);
      std::cout << std::endl;
    }
  }
}

struct Base
{
  virtual ~Base() {}
};
struct Paper : Base
{
};
struct Rock : Base
{
};
struct Scissors : Base
{
};

void paper_rock_scissors(const Base* b1, const Base* b2)
{
  using namespace simple_match;
  using namespace simple_match::placeholders;

  match(
      std::tie(b1, b2),
      ds(some<Paper>(), some<Paper>()),
      [](auto&, auto&) { std::cout << "Tie with both Paper\n"; },
      ds(some<Paper>(), some<Rock>()),
      [](auto&, auto&) { std::cout << "Winner 1 - Paper covers Rock\n"; },
      ds(some<Paper>(), some<Scissors>()),
      [](auto&, auto&) { std::cout << "Winner 2 - Scissors cuts Paper\n"; },
      ds(some<Rock>(), some<Paper>()),
      [](auto&, auto&) { std::cout << "Winner 2 - Paper covers Rock\n"; },
      ds(some<Rock>(), some<Rock>()),
      [](auto&, auto&) { std::cout << "Tie with both Rock\n"; },
      ds(some<Rock>(), some<Scissors>()),
      [](auto&, auto&) { std::cout << "Winner 1 - Rock smashes Scissors\n"; },
      ds(some<Scissors>(), some<Paper>()),
      [](auto&, auto&) { std::cout << "Winner 1 - Scissors cuts Paper\n"; },
      ds(some<Scissors>(), some<Rock>()),
      [](auto&, auto&) { std::cout << "Winner 2 - Rock smashes Scissors\n"; },
      ds(some<Scissors>(), some<Scissors>()),
      [](auto&, auto&) { std::cout << "Tie both with Scissors\n"; }

  );
}

auto safe_div(int num, int denom) -> boost::optional<int>
{
  using namespace simple_match;
  using namespace simple_match::placeholders;
  return match(
      std::tie(num, denom),
      ds(_, 0),
      []() { return boost::optional<int> {}; },
      ds(_x, _y),
      [](int x, int y) { return boost::optional<int> {x / y}; });
}

void test_optional()
{
  using namespace simple_match;
  using namespace simple_match::placeholders;
  auto m = [](auto&& v)
  {
    return match(
        v,
        some(),
        [](auto x) { std::cout << "the safe_div answer is " << x << "\n"; },
        none(),
        []() { std::cout << "Tried to divide by 0 in safe_div\n"; });
  };

  m(safe_div(4, 2));
  m(safe_div(4, 0));
}

void test_lexical_cast()
{
  using namespace simple_match;
  using namespace simple_match::placeholders;

  auto m = [](const std::string& s)
  {
    match(
        s,
        lexical_cast<int>(_x),
        [](auto x) { std::cout << "Got int " << x << "\n"; },
        lexical_cast<double>(_x < 123),
        [](auto x) { std::cout << "Got double less than 123.3 " << x << "\n"; },
        lexical_cast<double>(),
        []()
        {
          std::cout << "Matched a double "
                    << "\n";
        },
        _,
        []() { std::cout << "Did not match\n"; }

    );
  };

  m("123");
  m("123.3");
  m("23.3");
  m("");
  m("xyz");
}

#if 0
void test_regex()
{
  using namespace simple_match;
  using namespace simple_match::placeholders;

  auto toll_free = make_matcher_predicate(
      [](boost::string_ref s)
      {
        const static std::vector<std::string> toll_free_nums {
            "800", "888", "877", "866", "855"};
        return std::find(toll_free_nums.begin(), toll_free_nums.end(), s)
            != toll_free_nums.end();
      });

  auto m = [&](const std::string& s)
  {
    match(
        s,
        rex_match("([a-z]+)\\.txt", _x),
        [](auto& x) { std::cout << x << "\n"; },
        rex_match("([0-9]{4})-([0-9]{2})-([0-9]{2})",
                  lexical_cast<int>(_x),
                  lexical_cast<int>(0 < _x <= 12),
                  lexical_cast<int>(0 < _x <= 31)),
        [](auto y, auto m, auto d)
        { std::cout << "Got date " << y << " " << m << " " << d << "\n"; },
        rex_match("([0-9]{3})-([0-9]+)-([0-9]+)", "979", _y, _z),
        [](auto& y, auto& z)
        { std::cout << "Got local phone " << y << "-" << z << "\n"; },
        rex_match("([0-9]{3})-([0-9]+)-([0-9]+)", toll_free, _y, _z),
        [](auto& x, auto& y, auto& z)
        { std::cout << "Got toll free " << x << "-" << y << "-" << z << "\n"; },
        rex_match("([0-9]{3})-([0-9]+)-([0-9]+)", _x, _y, _z),
        [](auto& x, auto& y, auto& z) {
          std::cout << "Got long distance " << x << "-" << y << "-" << z
                    << "\n";
        },
        _x,
        [](auto& x) { std::cout << x << " Did not match a regex\n"; }
    );
  };

  m("foo.txt");
  m("2015-01-22");
  m("2015-13-22");
  m("2015-01-00");
  m("979-123-4567");
  m("877-123-4567");
  m("561-123-4567");
}
#endif

auto main() -> int
{
  FizzBuzz();

  math_variant_t var {add {2, mul {3, neg {2}}}};
  std::cout << eval(var) << "\n";
  eval_any any_var {add2 {2, mul2 {3, neg2 {2}}}};
  std::cout << eval(any_var) << "\n";

  math_variant2_t var2 {add3 {2, mul3 {3, neg3 {2}}}};
  std::cout << eval(var2) << "\n";
  math_variant2_t var_fused {sub3 {10, add3 {2, mul3 {3, 2}}}};
  std::cout << eval(var_fused) << "\n";

  std::unique_ptr<eval_base_t> eval_base = std::make_unique<add4>(
      std::make_unique<int_holder>(2),
      std::make_unique<mul4>(
          std::make_unique<int_holder>(3),
          std::make_unique<neg4>(std::make_unique<int_holder>(2))));

  std::cout << eval(eval_base) << "\n";

  test_string();
  test_some_none();
  test_ds();

  using namespace simple_match;
  using namespace simple_match::placeholders;

  int xs[] = {1, 2, 4, 15, 20, 21};

  for (auto x : xs) {
    match(
        x,
        1,
        []() { std::cout << "The answer is one\n"; },
        2,
        []() { std::cout << "The answer is two\n"; },
        _x < 10,
        [](auto&& a)
        { std::cout << "The answer " << a << " is less than 10\n"; },
        10 < _x < 20,
        [](auto&& a) {
          std::cout << "The answer " << a
                    << " is between 10 and 20 exclusive\n";
        },
        _,
        []() { std::cout << "Did not match\n"; });
  }

  TestBoolExp();

  std::unique_ptr<Base> p1 {new Rock};
  std::unique_ptr<Base> p2 {new Scissors};
  std::unique_ptr<Base> p3 {new Paper};

  paper_rock_scissors(p1.get(), p2.get());
  paper_rock_scissors(p3.get(), p1.get());

  test_holder();
  test_any();
  test_optional();
  test_lexical_cast();
#if 0
  test_regex();
#endif

  return 0;
}
