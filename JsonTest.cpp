#include "JsonSerialiser.hpp"
#include <assert.h>

int test_basic() {
    struct test1 {
      char c1 = 1;
      unsigned char uc1 = 2;
      int i1 = 3;
      unsigned int ui1 = 4;
      long l1 = 5;
      unsigned long ul1 = 6;
      long long ll1 = 7;
      unsigned long long ull1 = 8;
      size_t z1 = 9;
      float f1 = 12.3;
      double d1 = 45.6;
      std::string s1 = "abc";

      inline test1(){}

      inline test1(const posdk::Json::Tree& jobj)
      : c1(jget(jobj,"c1",c1))
      , uc1(jget(jobj,"uc1",uc1))
      , i1(jget(jobj,"i1",i1))
      , ui1(jget(jobj,"ui1",ui1))
      , l1(jget(jobj,"l1",l1))
      , ul1(jget(jobj,"ul1",ul1))
      , ll1(jget(jobj,"ll1",ll1))
      , ull1(jget(jobj,"ull1",ull1))
      , z1(jget(jobj,"z1",z1))
      , f1(jget(jobj,"f1",f1))
      , d1(jget(jobj,"d1",d1))
      , s1(jget(jobj,"s1",s1))
      {}

      inline void jsave(posdk::Json::Tree& jobj) const {
        jset(jobj, "c1", c1);
        jset(jobj, "uc1", uc1);
        jset(jobj, "i1", i1);
        jset(jobj, "ui1", ui1);
        jset(jobj, "l1", l1);
        jset(jobj, "ul1", ul1);
        jset(jobj, "ll1", ll1);
        jset(jobj, "ull1", ull1);
        jset(jobj, "z1", z1);
        jset(jobj, "f1", f1);
        jset(jobj, "d1", d1);
        jset(jobj, "s1", s1);
      }
    };

    test1 t1;
    posdk::Json::Tree jobj(posdk::Json::DataType::Object);
    t1.jsave(jobj);

    auto x = posdk::Json::saveToString(jobj);
    std::cout << "x:" << x << std::endl;

    test1 t2(jobj);

    return 0;
}

int test_inherit() {
    struct test1 {
      int i1;
      std::string s1;
      inline test1(const int& pi1, const std::string& ps1) : i1(pi1), s1(ps1){}

      inline test1(const posdk::Json::Tree& jobj)
      : i1(jget(jobj,"i1",i1))
      , s1(jget(jobj,"s1",s1))
      {}

      inline void jsave(posdk::Json::Tree& jobj) const {
        jset(jobj, "i1", i1);
        jset(jobj, "s1", s1);
      }
    };

    struct test2 {
      long l1 = 5;
      test1 t1;
      inline test2(const long& pl1, const int& pi1, const std::string& ps1) : l1(pl1), t1(pi1, ps1){}

      inline test2(const posdk::Json::Tree& jobj)
      : l1(jget(jobj,"l1",l1))
      , t1(jget(jobj,"t1",t1))
      {}

      inline void jsave(posdk::Json::Tree& jobj) const {
        jset(jobj, "l1", l1);
        jset(jobj, "t1", t1);
      }
    };

    struct test3 : public test2 {
      float f1 = 45.6;
      posdk::Json::Tree j1;
      inline test3(const long& pl1, const int& pi1, const std::string& ps1, const float& pf1) : test2(pl1, pi1, ps1), f1(pf1){}

      inline test3(const posdk::Json::Tree& jobj)
      : test2(jobj)
      , f1(jget(jobj,"f1",f1))
      , j1(jget(jobj,"j1",j1))
      {}

      inline void jsave(posdk::Json::Tree& jobj) const {
        test2::jsave(jobj);
        jset(jobj, "f1", f1);
        jset(jobj, "j1", j1);
      }
    };

    test3 t1(0l, 1, "rp", 2.34);
    posdk::Json::Tree jobj1(posdk::Json::DataType::Object);
    t1.jsave(jobj1);

    auto x1 = posdk::Json::saveToString(jobj1);
    std::cout << "x1:" << x1 << std::endl;

    test3 t2(jobj1);

    posdk::Json::Tree jobj2(posdk::Json::DataType::Object);
    t2.jsave(jobj2);

    auto x2 = posdk::Json::saveToString(jobj2);
    std::cout << "x2:" << x2 << std::endl;

    return 0;
}

enum class enumx1 {
    val1,
    val2,
};

template <>
inline std::string posdk::Json::e2s(const enumx1& val) {
    switch(val) {
    case enumx1::val1:
      return "val1";
    case enumx1::val2:
      return "val2";
    }
    return "";
}

template <>
inline enumx1 posdk::Json::s2e(const std::string& val) {
    if(val == "val1") {
      return enumx1::val1;
    }
    if(val == "val2") {
      return enumx1::val2;
    }
    return enumx1::val2;
}

int test_enum() {
    struct test1 {
      enumx1 ex1 = enumx1::val1;
      inline test1() {}

      inline test1(const posdk::Json::Tree& jobj)
      : ex1(jget(jobj,"ex1",ex1))
      {}

      inline void jsave(posdk::Json::Tree& jobj) const {
        jset(jobj, "ex1", ex1);
      }
    };

    test1 t1;
    posdk::Json::Tree jobj(posdk::Json::DataType::Object);
    t1.jsave(jobj);

    auto x = posdk::Json::saveToString(jobj);
    std::cout << "x:" << x << std::endl;

    test1 t2(jobj);
    assert(t2.ex1 == enumx1::val1);

    return 0;
}

int test_variant() {
    struct test1 {
      std::variant<posdk::Json::string_t, int, float> v1;
      inline test1() {}

      inline test1(const posdk::Json::Tree& jobj)
      : v1(jget(jobj,"v1",v1))
      {}

      inline void jsave(posdk::Json::Tree& jobj) const {
        jset(jobj, "v1", v1);
      }
    };

    test1 t1;
    t1.v1 = 12;
    posdk::Json::Tree jobj(posdk::Json::DataType::Object);
    t1.jsave(jobj);

    auto x = posdk::Json::saveToString(jobj);
    std::cout << "x1:" << x << std::endl;

    t1.v1 = "hello";
    jobj = posdk::Json::Tree(posdk::Json::DataType::Object);
    t1.jsave(jobj);

    x = posdk::Json::saveToString(jobj);
    std::cout << "x2:" << x << std::endl;

    return 0;
}

int test_vector() {
    struct test1 {
      std::vector<int> v1;
      inline test1() {}

      inline test1(const posdk::Json::Tree& jobj)
      : v1(jget(jobj,"v1",v1))
      {}

      inline void jsave(posdk::Json::Tree& jobj) const {
        jset(jobj, "v1", v1);
      }
    };

    test1 t1;
    t1.v1.push_back(1);
    posdk::Json::Tree jobj(posdk::Json::DataType::Object);
    t1.jsave(jobj);

    auto x = posdk::Json::saveToString(jobj);
    std::cout << "x:" << x << std::endl;

    return 0;
}

int test_map() {
    struct test1 {
      std::map<std::string,float> v1;
      inline test1() {}

      inline test1(const posdk::Json::Tree& jobj)
      : v1(jget(jobj,"v1",v1))
      {}

      inline void jsave(posdk::Json::Tree& jobj) const {
        jset(jobj, "v1", v1);
      }
    };

    test1 t1;
    t1.v1["count"] = 12.3;
    posdk::Json::Tree jobj(posdk::Json::DataType::Object);
    t1.jsave(jobj);

    auto x = posdk::Json::saveToString(jobj);
    std::cout << "x:" << x << std::endl;

    return 0;
}

int main(int argc, char* argv[]) {
    test_basic();
    test_inherit();
    test_enum();
    test_variant();
    test_vector();
    test_map();
    return 0;
}
