#pragma once

#include <iostream>
#include <sstream>
#include <list>
#include <vector>
#include <cstddef>
#include <map>
#include <variant>

namespace posdk {
    class RuntimeError : public std::runtime_error {
    public:
        inline RuntimeError(const std::string& format) : runtime_error(format) {}
        inline RuntimeError(const std::string& format, const std::string& p1) : runtime_error(format + p1) {}
    };

    /// \brief class representing an Json parser
    namespace Json {
        typedef std::nullptr_t null_t;
        typedef bool bool_t;
        typedef int64_t integer_t;
        typedef float float_t;
        typedef std::string string_t;

        typedef std::variant<
            null_t,
            bool_t,
            integer_t,
            float_t,
            string_t
        > Value_t;

        enum class DataType {
            Value,
            Array,
            Object,
        };

        class Tree {
            DataType dataType_;
            Value_t value;
            std::list<std::pair<std::string, Tree>> items_;
            std::map<std::string, Tree*> names;

        public:
            typedef std::list<std::pair<std::string, Tree>>::const_iterator iterator;
            inline Tree(const DataType& dataType = DataType::Value) : dataType_(dataType), value(nullptr) {}

            explicit inline Tree(const bool_t& val) : dataType_(DataType::Value), value(val) {}
            explicit inline Tree(const integer_t& val) : dataType_(DataType::Value), value(val) {}
            explicit inline Tree(const float_t& val) : dataType_(DataType::Value), value(val) {}
            explicit inline Tree(const string_t& val) : dataType_(DataType::Value), value(val) {}

            // explicit inline Tree(const int& val) : dataType_(DataType::Value), value(static_cast<int64_t>(val)) {}
            // explicit inline Tree(const uint64_t& val) : dataType_(DataType::Value), value(static_cast<int64_t>(val)) {}
            // explicit inline Tree(const size_t& val) : dataType_(DataType::Value), value(static_cast<int64_t>(val)) {}

            inline Tree(const Tree& src) : dataType_(src.dataType_), value(src.value),  items_(src.items_) {
                if(!src.isArray()){
                    for(auto& p : items_){
                        names[p.first] = &p.second;
                    }
                }
            }

            inline Tree& operator=(const Tree& src) {
                dataType_ = src.dataType_;
                value = src.value;
                items_ = src.items_;
                if(!src.isArray()){
                    for(auto& p : items_){
                        names[p.first] = &p.second;
                    }
                }
                return *this;
            }

            template <typename ValT>
            inline auto isValue() const {
                return ((dataType_ == DataType::Value) && std::holds_alternative<ValT>(value));
            }

            inline bool isNull() const {
                return isValue<std::nullptr_t>();
            }

            inline bool isValueType() const {
                return (dataType_ == DataType::Value);
            }

            inline bool isArray() const {
                return (dataType_ == DataType::Array);
            }

            inline bool isObject() const {
                return (dataType_ == DataType::Object);
            }

            inline bool isContainer() const {
                return (isObject() || isArray());
            }

            inline iterator begin() const {
                if(!isContainer()) {
                    throw posdk::RuntimeError("attempting to iterate-begin on non-container");
                }
                return items_.begin();
            }

            inline iterator end() const {
                if(!isContainer()) {
                    throw posdk::RuntimeError("attempting to iterate-end on non-container");
                }
                return items_.end();
            }

            inline auto size() const {
                if(!isContainer()) {
                    throw posdk::RuntimeError("attempting to get size on non-container");
                }
                return items_.size();
            }

            inline auto& items() const {
                if(!isContainer()) {
                    throw posdk::RuntimeError("attempting to get items on non-container");
                }
                return items_;
            }

            template <typename ValT>
            inline auto getValue() const {
                if(dataType_ != DataType::Value) {
                    throw posdk::RuntimeError("attempting to get value on non-value");
                }
                if(!isValue<ValT>()){
                    throw posdk::RuntimeError("unexpected value type in JSON node");
                }
                return std::get<ValT>(value);
            }

            std::map<std::string, Tree*>::const_iterator find(const std::string& key) const;

            template <typename ValT>
            inline ValT get(const std::string& key) const {
                if(key.size() == 0) {
                    throw posdk::RuntimeError("key length is zero");
                }
                if(dataType_ != DataType::Object) {
                    throw posdk::RuntimeError("attempting to get key {0} on non-object", key);
                }
                auto vit = find(key);
                return vit->second->getValue<ValT>();
            }

            inline Tree& getChild(const std::string& key) const {
                if(key.size() == 0) {
                    throw posdk::RuntimeError("key length is zero");
                }
                if(dataType_ != DataType::Object) {
                    throw posdk::RuntimeError("attempting to get child {0} on non-object", key);
                }
                auto vit = find(key);
                return *(vit->second);
            }

            inline const Tree* hasChild(const std::string& key) const {
                if(key.size() == 0) {
                    throw posdk::RuntimeError("key length is zero");
                }
                if(dataType_ != DataType::Object){
                    return nullptr;
                }
                auto vit = names.find(key);
                if(vit == names.end()){
                    return nullptr;
                }
                const Tree& tree = *vit->second;
                return &tree;
            }

            template <typename ValT>
            inline Tree& add(const std::string& key, const ValT& val) {
                if(key.size() == 0) {
                    throw posdk::RuntimeError("key length is zero");
                }
                if(dataType_ != DataType::Object){
                    throw posdk::RuntimeError("attempting to add child {0} on non-object", key);
                }
                items_.push_back(std::make_pair(key, Tree(val)));
                Tree& t = items_.back().second;
                names[key] = &t;
                return t;
            }

            inline Tree& add(const std::string& key, const Tree& val) {
                if(key.size() == 0) {
                    throw posdk::RuntimeError("key length is zero");
                }
                if(dataType_ != DataType::Object){
                    throw posdk::RuntimeError("attempting to add child {0} on non-object", key);
                }
                items_.push_back(std::make_pair(key, val));
                Tree& t = items_.back().second;
                names[key] = &t;
                return t;
            }

            inline auto& add(const std::string& key, const char* val) {
                return add<std::string>(key, std::string(val));
            }

            template <typename ValT>
            inline void set(const std::string& key, const ValT& val) {
                if(dataType_ != DataType::Object){
                    throw posdk::RuntimeError("attempting to erase child {0} on non-object", key);
                }
                for(auto iit = items_.begin(), iite = items_.end(); iit != iite; ++iit){
                    if(iit->first == key){
                        iit->second = Tree(val);
                        return;
                    }
                }
                throw posdk::RuntimeError("attempting to set non-existent key: {0}", key);
            }

            inline void set(const std::string& key, const Tree& val) {
                if(dataType_ != DataType::Object){
                    throw posdk::RuntimeError("attempting to erase child {0} on non-object", key);
                }
                for(auto iit = items_.begin(), iite = items_.end(); iit != iite; ++iit){
                    if(iit->first == key){
                        iit->second = val;
                        return;
                    }
                }
                throw posdk::RuntimeError("attempting to set non-existent key: {0}", key);
            }

            template <typename ValT>
            inline void add(const ValT& val) {
                if(dataType_ != DataType::Array){
                    throw posdk::RuntimeError("attempting to add item on non-array");
                }
                items_.push_back(std::make_pair("", Tree(val)));
            }

            inline void add(const Tree& val) {
                if(dataType_ != DataType::Array){
                    throw posdk::RuntimeError("attempting to add item on non-array");
                }
                items_.push_back(std::make_pair("", val));
            }

            inline void erase(const std::string& key) {
                if(dataType_ != DataType::Object){
                    throw posdk::RuntimeError("attempting to erase child {0} on non-object", key);
                }
                for(auto iit = items_.begin(), iite = items_.end(); iit != iite; ++iit){
                    if(iit->first == key){
                        items_.erase(iit);
                        break;
                    }
                }
                auto vit = names.find(key);
                if(vit != names.end()){
                    names.erase(key);
                }
            }

            void print(std::ostream& os, const size_t& lvl, const size_t& indent) const;

        };

        /// \brief load Json string into posdk::Json::Tree structure
        void load(std::istream& in, const std::string& filename, Tree& tree);
        inline void save(std::ostream& os, const Tree& tree, const size_t& indent = 2){
            tree.print(os, 0, indent);
        }

        Tree loadFromString(const std::string& str);
        std::string saveToString(const Tree& tree, const size_t& indent = 2);

        Tree loadFromFile(const std::string& filename);
        void saveToFile(const Tree& tree, const std::string& filename, const size_t& indent = 2);

        std::string encodeString(const std::string& str);
        std::string decodeString(const std::string& str);
     }

    template <>
    inline auto Json::Tree::getValue<const char*>() const {
        return getValue<std::string>();
    }

    template <>
    inline auto Json::Tree::getValue<size_t>() const {
        return static_cast<size_t>(getValue<int64_t>());
    }

    template <>
    inline auto Json::Tree::getValue<int>() const {
        return getValue<int64_t>();
    }

    template <>
    inline auto Json::Tree::getValue<uint32_t>() const {
        return static_cast<int64_t>(getValue<int64_t>());
    }
}
