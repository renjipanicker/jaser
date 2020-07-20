#pragma once
#include "Json.hpp"
#include <assert.h>

namespace posdk {
    /// \brief class representing Json
    namespace Json {
        template <typename ValT>
        inline std::string e2s(const ValT& val);

        template <typename ValT>
        inline ValT s2e(const std::string& val);

        /// \brief internal classes
        namespace Json_ {
            // specializers
            struct specializer_basic {};
            struct specializer_map     : specializer_basic {};
            struct specializer_vector  : specializer_map {};
            struct specializer_variant : specializer_vector {};
            struct specializer_enum    : specializer_variant {};
            struct specializer_class   : specializer_enum {};
            struct specializer         : specializer_class {};

            // used to check if class has method
            template<typename> struct int_ { typedef int type; };

            // struct & classes
            template<typename ValT, typename int_<decltype(&ValT::jsave)>::type = 0>
            inline ValT j2v(const posdk::Json::Tree& jval, const specializer_class&) {
                return ValT(jval);
            }

            template<typename ValT, typename int_<decltype(&ValT::jsave)>::type = 0>
            inline posdk::Json::Tree v2j(const ValT& val, const specializer_class&) {
                posdk::Json::Tree jval(posdk::Json::DataType::Object);
                val.jsave(jval);
                return jval;
            }

            // enums
            template<typename ValT, typename = typename std::enable_if< std::is_enum<ValT>::value, ValT >::type>
            inline ValT j2v(const posdk::Json::Tree& jval, const specializer_enum&) {
                auto sval = jval.getValue<std::string>();
                return s2e<ValT>(sval);
            }

            template<typename ValT, typename = typename std::enable_if< std::is_enum<ValT>::value, ValT >::type>
            inline posdk::Json::Tree v2j(const ValT& val, const specializer_enum&) {
                auto eval = e2s<ValT>(val);
                auto jval = posdk::Json::Tree(eval);
                return jval;
            }

            // variant
            template <typename... ValT>
            inline void j2v_variant(const posdk::Json::Tree& jval, std::variant<ValT...>& val);

            template<typename... ValT>
            inline posdk::Json::Tree v2j_variant(const std::variant<ValT...>& val);

            // check if type is a variant (1)
            template<typename T>
            struct is_variant : std::false_type {};

            template<typename ...Args>
            struct is_variant<std::variant<Args...>> : std::true_type {};

            template<typename ValT, typename = typename std::enable_if< is_variant<ValT>::value, ValT >::type>
            inline ValT j2v(const posdk::Json::Tree& jval, const specializer_variant&){
                ValT val;
                j2v_variant(jval, val);
                return val;
            }

            template<typename ValT, typename = typename std::enable_if< is_variant<ValT>::value, ValT >::type>
            inline posdk::Json::Tree v2j(const ValT& val, const specializer_variant&){
                return v2j_variant(val);
            }

            // vector
            template <typename ValT>
            inline void j2v_vector(const posdk::Json::Tree& jval, std::vector<ValT>& val);

            template<typename ValT>
            inline posdk::Json::Tree v2j_vector(const std::vector<ValT>& val);

            // check if type is a vector (1)
            template<typename T>
            struct is_vector : std::false_type {};

            template<typename Args>
            struct is_vector<std::vector<Args>> : std::true_type {};

            template<typename ValT, typename = typename std::enable_if< is_vector<ValT>::value, ValT >::type>
            inline ValT j2v(const posdk::Json::Tree& jval, const specializer_vector&){
                ValT val;
                j2v_vector(jval, val);
                return val;
            }

            template<typename ValT, typename = typename std::enable_if< is_vector<ValT>::value, ValT >::type>
            inline posdk::Json::Tree v2j(const ValT& val, const specializer_vector&){
                return v2j_vector(val);
            }

            // map
            template <typename KeyT, typename ValT>
            inline void j2v_map(const posdk::Json::Tree& jval, std::map<KeyT,ValT>& val);

            template <typename KeyT, typename ValT>
            inline posdk::Json::Tree v2j_map(const std::map<KeyT,ValT>& val);

            // check if type is a map (1)
            template<typename T>
            struct is_map : std::false_type {};

            template <typename KeyT, typename ValT>
            struct is_map<std::map<KeyT,ValT>> : std::true_type {};

            template<typename ValT, typename = typename std::enable_if< is_map<ValT>::value, ValT >::type>
            inline ValT j2v(const posdk::Json::Tree& jval, const specializer_map&){
                ValT val;
                j2v_map(jval, val);
                return val;
            }

            template<typename ValT, typename = typename std::enable_if< is_map<ValT>::value, ValT >::type>
            inline posdk::Json::Tree v2j(const ValT& val, const specializer_map&){
                return v2j_map(val);
            }

            // basic types
            template <typename ValT>
            inline ValT j2v(const posdk::Json::Tree& jval, const specializer_basic&) {
                return jval.getValue<ValT>();
            }

            template <typename ValT>
            inline posdk::Json::Tree v2j(const ValT& val, const specializer_basic&) {
                return posdk::Json::Tree(val);
            }

            // json::Tree
            template <>
            inline posdk::Json::Tree j2v<posdk::Json::Tree>(const posdk::Json::Tree& jval, const specializer_basic&) {
                return jval;
            }

            template <>
            inline posdk::Json::Tree v2j<posdk::Json::Tree>(const posdk::Json::Tree& val, const specializer_basic&) {
                return val;
            }

            //double
            template <>
            inline double j2v<double>(const posdk::Json::Tree& jval, const specializer_basic&) {
                return static_cast<double>(jval.getValue<posdk::Json::float_t>());
            }

            template <>
            inline posdk::Json::Tree v2j<double>(const double& val, const specializer_basic&) {
                return posdk::Json::Tree(static_cast<posdk::Json::float_t>(val));
            }

            // long long
            template <>
            inline long long j2v<long long>(const posdk::Json::Tree& jval, const specializer_basic&) {
                return static_cast<long long>(jval.getValue<posdk::Json::integer_t>());
            }

            template <>
            inline posdk::Json::Tree v2j<long long>(const long long& val, const specializer_basic&) {
                return posdk::Json::Tree(static_cast<posdk::Json::integer_t>(val));
            }

            // unsigned long long
            template <>
            inline unsigned long long j2v<unsigned long long>(const posdk::Json::Tree& jval, const specializer_basic&) {
                return static_cast<unsigned long long>(jval.getValue<posdk::Json::integer_t>());
            }

            template <>
            inline posdk::Json::Tree v2j<unsigned long long>(const unsigned long long& val, const specializer_basic&) {
                return posdk::Json::Tree(static_cast<posdk::Json::integer_t>(val));
            }

            // int64_t
            template <>
            inline int64_t j2v<int64_t>(const posdk::Json::Tree& jval, const specializer_basic&) {
                return static_cast<int64_t>(jval.getValue<posdk::Json::integer_t>());
            }

            template <>
            inline posdk::Json::Tree v2j<int64_t>(const int64_t& val, const specializer_basic&) {
                return posdk::Json::Tree(static_cast<posdk::Json::integer_t>(val));
            }

            // uint64_t
            template <>
            inline uint64_t j2v<uint64_t>(const posdk::Json::Tree& jval, const specializer_basic&) {
                return static_cast<uint64_t>(jval.getValue<posdk::Json::integer_t>());
            }

            template <>
            inline posdk::Json::Tree v2j<uint64_t>(const uint64_t& val, const specializer_basic&) {
                return posdk::Json::Tree(static_cast<posdk::Json::integer_t>(val));
            }

            // int32_t
            template <>
            inline int32_t j2v<int32_t>(const posdk::Json::Tree& jval, const specializer_basic&) {
                return static_cast<int32_t>(jval.getValue<posdk::Json::integer_t>());
            }

            template <>
            inline posdk::Json::Tree v2j<int32_t>(const int32_t& val, const specializer_basic&) {
                return posdk::Json::Tree(static_cast<posdk::Json::integer_t>(val));
            }

            // uint32_t
            template <>
            inline uint32_t j2v<uint32_t>(const posdk::Json::Tree& jval, const specializer_basic&) {
                return static_cast<uint32_t>(jval.getValue<posdk::Json::integer_t>());
            }

            template <>
            inline posdk::Json::Tree v2j<uint32_t>(const uint32_t& val, const specializer_basic&) {
                return posdk::Json::Tree(static_cast<posdk::Json::integer_t>(val));
            }

            // int16_t
            template <>
            inline int16_t j2v<int16_t>(const posdk::Json::Tree& jval, const specializer_basic&) {
                return static_cast<int16_t>(jval.getValue<posdk::Json::integer_t>());
            }

            template <>
            inline posdk::Json::Tree v2j<int16_t>(const int16_t& val, const specializer_basic&) {
                return posdk::Json::Tree(static_cast<posdk::Json::integer_t>(val));
            }

            // uint16_t
            template <>
            inline uint16_t j2v<uint16_t>(const posdk::Json::Tree& jval, const specializer_basic&) {
                return static_cast<uint16_t>(jval.getValue<posdk::Json::integer_t>());
            }

            template <>
            inline posdk::Json::Tree v2j<uint16_t>(const uint16_t& val, const specializer_basic&) {
                return posdk::Json::Tree(static_cast<posdk::Json::integer_t>(val));
            }

            // int8_t
            template <>
            inline int8_t j2v<int8_t>(const posdk::Json::Tree& jval, const specializer_basic&) {
                return static_cast<int8_t>(jval.getValue<posdk::Json::integer_t>());
            }

            template <>
            inline posdk::Json::Tree v2j<int8_t>(const int8_t& val, const specializer_basic&) {
                return posdk::Json::Tree(static_cast<posdk::Json::integer_t>(val));
            }

            // uint8_t
            template <>
            inline uint8_t j2v<uint8_t>(const posdk::Json::Tree& jval, const specializer_basic&) {
                return static_cast<uint8_t>(jval.getValue<posdk::Json::integer_t>());
            }

            template <>
            inline posdk::Json::Tree v2j<uint8_t>(const uint8_t& val, const specializer_basic&) {
                return posdk::Json::Tree(static_cast<posdk::Json::integer_t>(val));
            }

            // char
            template <>
            inline char j2v<char>(const posdk::Json::Tree& jval, const specializer_basic&) {
                return static_cast<char>(jval.getValue<posdk::Json::integer_t>());
            }

            template <>
            inline posdk::Json::Tree v2j<char>(const char& val, const specializer_basic&) {
                return posdk::Json::Tree(static_cast<posdk::Json::integer_t>(val));
            }

            // looper (used in variant converter)
            template<class T, T... inds, class F>
            constexpr void loop_(std::integer_sequence<T, inds...>, F&& f) {
                (f(std::integral_constant<T, inds>{}), ...);// C++17 fold expression
            }

            template<class T, T count, class F>
            constexpr void loop(F&& f) {
                loop_(std::make_integer_sequence<T, count>{}, std::forward<F>(f));
            }

            // variant helpers. these need to be at the bottom of the list
            template <typename... ValT>
            inline void j2v_variant(const posdk::Json::Tree& jval, std::variant<ValT...>& val){
                auto varidx = jval.get<size_t>("__varidx__");
                auto& jdata = jval.getChild("__data__");
                assert(varidx < sizeof...(ValT));

                loop<size_t, sizeof...(ValT)>([&] (auto i) {
                    constexpr size_t idx = i;
                    if(idx == varidx){
                        typedef decltype(std::get<idx>(val)) TypeRef;
                        typedef typename std::remove_reference<TypeRef>::type ConstType;
                        typedef typename std::remove_const<ConstType>::type Type;

                        val = Json_::j2v<Type>(jdata, Json_::specializer());
                    }
                });
            }

            template<typename... ValT>
            inline posdk::Json::Tree v2j_variant(const std::variant<ValT...>& val){
                posdk::Json::Tree jval(posdk::Json::DataType::Object);
                jval.add("__varidx__", static_cast<posdk::Json::integer_t>(val.index()));

                std::visit([&jval](const auto& x){
                    typedef decltype(x) TypeRef;
                    typedef typename std::remove_reference<TypeRef>::type ConstType;
                    typedef typename std::remove_const<ConstType>::type Type;

                    auto jdata = Json_::v2j<Type>(x, Json_::specializer());
                    jval.add("__data__", jdata);
                }, val);

                return jval;
            }

            // vector helpers
            template <typename ValT>
            inline void j2v_vector(const posdk::Json::Tree& jval, std::vector<ValT>& val) {
                for(auto& jdata : jval){
                    auto aval = Json_::j2v<ValT>(jdata.second, specializer());
                    val.push_back(aval);
                }
            }

            template<typename ValT>
            inline posdk::Json::Tree v2j_vector(const std::vector<ValT>& val) {
                posdk::Json::Tree jval(posdk::Json::DataType::Array);
                for(auto& x : val){
                    auto jdata = Json_::v2j<ValT>(x, specializer());
                    jval.add(jdata);
                }
                return jval;
            }

            // map helpers
            template <typename KeyT, typename ValT>
            inline void j2v_map(const posdk::Json::Tree& jval, std::map<KeyT,ValT>& val) {
                for(auto& jdata : jval){
                    auto& jkey = jdata.second.getChild("__key__");
                    auto& jval = jdata.second.getChild("__val__");
                    auto akey = Json_::j2v<KeyT>(jkey, specializer());
                    auto aval = Json_::j2v<ValT>(jval, specializer());
                    val[akey] = aval;
                }
            }

            template <typename KeyT, typename ValT>
            inline posdk::Json::Tree v2j_map(const std::map<KeyT,ValT>& val) {
                posdk::Json::Tree jret(posdk::Json::DataType::Array);
                for(auto& x : val){
                    auto jkey = Json_::v2j<KeyT>(x.first, specializer());
                    auto jval = Json_::v2j<ValT>(x.second, specializer());
                    posdk::Json::Tree jpair(posdk::Json::DataType::Object);
                    jpair.add("__key__", jkey);
                    jpair.add("__val__", jval);
                    jret.add(jpair);
                }
                return jret;
            }
        }

        /// \brief convert from JSON
        template <typename ValT>
        inline ValT jget(const posdk::Json::Tree& jobj, const std::string& key, const ValT&) {
            auto& jval = jobj.getChild(key);
            return Json_::j2v<ValT>(jval, Json_::specializer());
        }

        /// \brief convert to JSON
        template <typename ValT>
        inline void jset(posdk::Json::Tree& jobj, const std::string& key, const ValT& val) {
            auto jval = Json_::v2j<ValT>(val, Json_::specializer());
            jobj.add(key, jval);
        }
    }
}
