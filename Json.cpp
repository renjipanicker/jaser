#include "Json.hpp"
#include <fstream>
#include <assert.h>

#ifndef NDEBUG
#define DO_ASSERT
#endif

#ifdef DO_ASSERT
#define ASSERT(e) assert(e)
#else
#define ASSERT(e)
#endif

namespace {
    enum class ParserState {
        EnterValue,
        EnterNumber0,
        EnterNumber1,
        EnterString,
        EnterStringEscape,
        EnterObjectKey,
        EnterObjectKeyString,
        LeaveObjectKeyString,
        LeaveObjectValue,
        EnterArray0,
        EnterArray1,
        TokenTrue_t,
        TokenTrue_tr,
        TokenTrue_tru,
        TokenFalse_f,
        TokenFalse_fa,
        TokenFalse_fal,
        TokenFalse_fals,
        TokenNull_n,
        TokenNull_nu,
        TokenNull_nul,
    };

    struct Tokeniser {
        std::istream& in_;
        const std::string& name_;
        size_t row = 1;
        size_t col = 1;
        char ch_;

        inline std::string pos() const {
            std::ostringstream os;
            os << name_ << "(" << row << "," << col << ")";
            return os.str();
        }

        inline char eof() {
            return in_.eof();
        }

        inline char peek_() {
            auto ch = static_cast<char>(in_.peek());
            return ch;
        }

        inline char peek() {
            ch_ = peek_();
            return ch_;
        }

        inline void next() {
            if(ch_ != peek_()) {
                ASSERT(false);
                throw posdk::RuntimeError("{0}: internal error in json", pos());
            }

            in_.get();
            col++;
            if(ch_ == '\n') {
                row++;
                col = 1;
            }
        }

        inline Tokeniser(std::istream& in, const std::string& name) : in_(in), name_(name) {}
    };

    void parseValue(Tokeniser& in, posdk::Json::Tree& tree) {
        auto s = ParserState::EnterValue;
        std::string n;
        std::string k;
        while(!in.eof()) {
            char ch = in.peek();
            // std::cout << in.pos() << ":" << ch << ":" << static_cast<int>(ch) << ":" << static_cast<int>(s) << std::endl;

            if(s != ParserState::EnterString){
                switch(ch) {
                    case 0:
                    case ' ':
                    case '\t':
                    case '\r':
                    case '\n':
                        in.next();
                        continue;
                }
            }

            posdk::Json::Tree j(posdk::Json::DataType::Value);
            switch(s) {
                case ParserState::EnterValue:
                    switch(ch) {
                        case '{':
                            in.next();
                            s = ParserState::EnterObjectKey;
                            tree = posdk::Json::Tree(posdk::Json::DataType::Object);
                            break;

                        case '[':
                            in.next();
                            s = ParserState::EnterArray0;
                            tree = posdk::Json::Tree(posdk::Json::DataType::Array);
                            break;

                        case 't':
                            in.next();
                            s = ParserState::TokenTrue_t;
                            break;

                        case 'f':
                            in.next();
                            s = ParserState::TokenFalse_f;
                            break;

                        case 'n':
                            in.next();
                            s = ParserState::TokenNull_n;
                            break;

                        case '-':
                        case '+':
                        case '0':
                        case '1':
                        case '2':
                        case '3':
                        case '4':
                        case '5':
                        case '6':
                        case '7':
                        case '8':
                        case '9':
                            // add to number
                            n += ch;
                            in.next();
                            s = ParserState::EnterNumber0;
                            break;

                        case '"':
                            in.next();
                            s = ParserState::EnterString;
                            break;

                        default:
                            ASSERT(false);
                            throw posdk::RuntimeError("{0}: invalid char in json", in.pos());
                    }
                    break;

                case ParserState::EnterNumber0:
                    switch(ch) {
                        case '0':
                        case '1':
                        case '2':
                        case '3':
                        case '4':
                        case '5':
                        case '6':
                        case '7':
                        case '8':
                        case '9':
                            // add to number
                            n += ch;
                            in.next();
                            break;

                        case '.':
                            n += ch;
                            in.next();
                            s = ParserState::EnterNumber1;
                            break;

                        default:
                            tree = posdk::Json::Tree(static_cast<int64_t>(std::atol(n.c_str())));
                            return;
                    }
                    break;

                case ParserState::EnterNumber1:
                    switch(ch) {
                        case '0':
                        case '1':
                        case '2':
                        case '3':
                        case '4':
                        case '5':
                        case '6':
                        case '7':
                        case '8':
                        case '9':
                            // add to number
                            n += ch;
                            in.next();
                            break;

                        default:
                            tree = posdk::Json::Tree(static_cast<float>(std::atof(n.c_str())));
                            return;
                    }
                    break;
                    
                case ParserState::EnterString:
                    switch(ch) {
                        case '"':
                            in.next();
                            tree = posdk::Json::Tree(n);
                            return;

                        case '\\':
                            in.next();
                            s = ParserState::EnterStringEscape;
                            break;

                        default:
                            // add to string
                            in.next();
                            n += ch;
                            break;
                    }
                    break;
                    
                case ParserState::EnterStringEscape:
                    switch(ch) {
                        case 'r':
                            in.next();
                            break;
                        case 'n':
                            n += '\n';
                            in.next();
                            break;
                        default:
                            in.next();
                            n += '\\';
                            n += ch;
                            break;
                    }
                    s = ParserState::EnterString;
                    break;

                case ParserState::EnterArray0:
                    switch(ch) {
                        case ']':
                            in.next();
                            return;

                        default:
                            parseValue(in, j);
                            tree.add(j);
                            s = ParserState::EnterArray1;
                            break;
                    }
                    break;
                case ParserState::EnterArray1:
                    switch(ch) {
                        case ',':
                            in.next();
                            parseValue(in, j);
                            tree.add(j);
                            break;

                        case ']':
                            in.next();
                            return;

                        default:
                            ASSERT(false);
                            throw posdk::RuntimeError("{0}: invalid char in json", in.pos());
                    }
                    break;

                case ParserState::EnterObjectKey:
                    switch(ch) {
                        case '"':
                            in.next();
                            k = "";
                            s = ParserState::EnterObjectKeyString;
                            break;

                        case '}':
                            in.next();
                            return;

                        default:
                            ASSERT(false);
                            throw posdk::RuntimeError("{0}: invalid char in json", in.pos());
                    }
                    break;

                case ParserState::EnterObjectKeyString:
                    switch(ch) {
                        case '"':
                            in.next();
                            s = ParserState::LeaveObjectKeyString;
                            break;

                        default:
                            // add to key string
                            k += ch;
                            in.next();
                            break;
                    }
                    break;
                    
                case ParserState::LeaveObjectKeyString:
                    switch(ch) {
                        case ':':
                            in.next();
                            parseValue(in, j);
                            tree.add(k, j);
                            k = "";
                            s = ParserState::LeaveObjectValue;
                            break;
                            
                        default:
                            ASSERT(false);
                            throw posdk::RuntimeError("{0}: invalid char in json", in.pos());
                    }
                    break;

                case ParserState::LeaveObjectValue:
                    switch(ch) {
                        case '}':
                            in.next();
                            return;

                        case ',':
                            in.next();
                            s = ParserState::EnterObjectKey;
                            break;

                        default:
                            ASSERT(false);
                            throw posdk::RuntimeError("{0}: invalid char in json", in.pos());
                    }
                    break;
                case ParserState::TokenTrue_t:
                    if(ch == 'r'){
                        in.next();
                        s = ParserState::TokenTrue_tr;
                        break;
                    }
                    ASSERT(false);
                    throw posdk::RuntimeError("{0}: invalid char in json", in.pos());
                case ParserState::TokenTrue_tr:
                    if(ch == 'u'){
                        in.next();
                        s = ParserState::TokenTrue_tru;
                        break;
                    }
                    ASSERT(false);
                    throw posdk::RuntimeError("{0}: invalid char in json", in.pos());
                case ParserState::TokenTrue_tru:
                    if(ch == 'e'){
                        in.next();
                        tree = posdk::Json::Tree(true);
                        return;
                    }
                    ASSERT(false);
                    throw posdk::RuntimeError("{0}: invalid char in json", in.pos());
                case ParserState::TokenFalse_f:
                    if(ch == 'a'){
                        in.next();
                        s = ParserState::TokenFalse_fa;
                        break;
                    }
                    ASSERT(false);
                    throw posdk::RuntimeError("{0}: invalid char in json", in.pos());
                case ParserState::TokenFalse_fa:
                    if(ch == 'l'){
                        in.next();
                        s = ParserState::TokenFalse_fal;
                        break;
                    }
                    ASSERT(false);
                    throw posdk::RuntimeError("{0}: invalid char in json", in.pos());
                case ParserState::TokenFalse_fal:
                    if(ch == 's'){
                        in.next();
                        s = ParserState::TokenFalse_fals;
                        break;
                    }
                    ASSERT(false);
                    throw posdk::RuntimeError("{0}: invalid char in json", in.pos());
                case ParserState::TokenFalse_fals:
                    if(ch == 'e'){
                        in.next();
                        tree = posdk::Json::Tree(false);
                        return;
                    }
                    ASSERT(false);
                    throw posdk::RuntimeError("{0}: invalid char in json", in.pos());
                case ParserState::TokenNull_n:
                    if(ch == 'u'){
                        in.next();
                        s = ParserState::TokenNull_nu;
                        break;
                    }
                    ASSERT(false);
                    throw posdk::RuntimeError("{0}: invalid char in json", in.pos());
                case ParserState::TokenNull_nu:
                    if(ch == 'l'){
                        in.next();
                        s = ParserState::TokenNull_nul;
                        break;
                    }
                    ASSERT(false);
                    throw posdk::RuntimeError("{0}: invalid char in json", in.pos());
                case ParserState::TokenNull_nul:
                    if(ch == 'l'){
                        in.next();
                        tree = posdk::Json::Tree(posdk::Json::DataType::Value);
                        return;
                    }
                    ASSERT(false);
                    throw posdk::RuntimeError("{0}: invalid char in json", in.pos());
            }
        }

        ASSERT(false);
        throw posdk::RuntimeError("{0}: unexpected EOF in json", in.pos());
    }

    struct Print {
        std::ostream& os_;
        inline Print(std::ostream& os) : os_(os) {}
        inline void operator()(const std::nullptr_t&){
            os_ << "null";
        }
        inline void operator()(const bool& val){
            os_ << (val?"true":"false");
        }
        inline void operator()(const int64_t& val){
            os_ << val;
        }
        inline void operator()(const float& val){
            std::ios_base::fmtflags f(os_.flags());
            os_.precision(8);
            os_ << std::fixed;
            os_ << val;
            os_.flags(f);
        }
        inline void operator()(const std::string& val){
            os_ << "\"";
            for(auto& ch : val) {
                switch(ch) {
                case '\n':
                    os_ << "\\n";
                    break;
                default:
                    os_ << ch;
                    break;
                }
            }
            os_ << "\"";
        }
    };
}

std::map<std::string, posdk::Json::Tree*>::const_iterator posdk::Json::Tree::find(const std::string& key) const {
    auto vit = names.find(key);
    if(vit == names.end()){
        throw posdk::RuntimeError("json key not found:" + key);
    }
    return vit;
}

void posdk::Json::Tree::print(std::ostream& os, const size_t& lvl, const size_t& indent) const {
    std::string sep;
    std::string nsep = ",\n";
    std::string nl = "\n";
    std::string indent1(lvl*2, ' ');
    std::string indent2((lvl+1)*2, ' ');

    if(indent == 0){
        indent1 = "";
        indent2 = "";
        nsep = ",";
        nl = "";
    }
    switch(dataType_){
    case DataType::Object:
        if(items_.size() == 0){
            os << "{}";
            break;
        }
        os << "{" << nl;
        for(auto& p : items_){
            os << sep << indent2;
            os << "\"" << p.first << "\":";
            p.second.print(os, lvl + 1, indent);
            sep = nsep;
        }
        os << nl << indent1 << "}";
        break;
    case DataType::Array:
        if(items_.size() == 0){
            os << "[]";
            break;
        }
        os << "[" << nl;
        for(auto& p : items_){
            os << sep << indent2;
            p.second.print(os, lvl + 1, indent);
            sep = nsep;
        }
        os << nl << indent1 << "]";
        break;
    case DataType::Value:
        std::visit(Print(os), value);
        break;
    }
}

void posdk::Json::load(std::istream& in, const std::string& filename, posdk::Json::Tree& tree) {
    Tokeniser tok(in, filename);
    return parseValue(tok, tree);
}

posdk::Json::Tree posdk::Json::loadFromString(const std::string& str) {
    posdk::Json::Tree tree(posdk::Json::DataType::Value);
    if(str.size() == 0) {
        return tree;
    }

    std::istringstream iss(str);
    load(iss, "<str>", tree);
    return tree;
}

std::string posdk::Json::saveToString(const Tree& tree, const size_t& indent) {
    std::ostringstream oss;
    save(oss, tree, indent);
    return oss.str();
}

posdk::Json::Tree posdk::Json::loadFromFile(const std::string& filename) {
    posdk::Json::Tree tree(posdk::Json::DataType::Value);
    std::ifstream ifs(filename);
    if(!ifs) {
        throw posdk::RuntimeError("file not found:" + filename);
    }
    load(ifs, filename, tree);
    return tree;
}

void posdk::Json::saveToFile(const Tree& tree, const std::string& filename, const size_t& indent) {
    std::ofstream ofs(filename);
    save(ofs, tree, indent);
}

enum class CodecState {
    Init,
    InEscape,
    InString,
    InStringEscape,
};

std::string posdk::Json::decodeString(const std::string& estr) {
    std::string str;
    auto state = CodecState::Init;
    for(auto& ch : estr){
        switch(state) {
        case CodecState::Init:
            switch(ch) {
            case '\\':
                state = CodecState::InEscape;
                break;
            case '\"':
                str += ch;
                state = CodecState::InString;
                break;
            default:
                str += ch;
                break;
            }
            break;
        case CodecState::InEscape:
            switch(ch) {
            case '"':
                str += ch;
                break;
            default:
                str += '\\';
                str += ch;
                break;
            }
            state = CodecState::Init;
            break;
        case CodecState::InString:
            switch(ch) {
            case '\\':
                str += ch;
                state = CodecState::InStringEscape;
                break;
            case '\"':
                str += ch;
                state = CodecState::Init;
                break;
            default:
                str += ch;
                break;
            }
            break;
        case CodecState::InStringEscape:
            str += ch;
            state = CodecState::InString;
            break;
        }
    }
    return str;
}

std::string posdk::Json::encodeString(const std::string& str) {
    std::string estr;
    auto state = CodecState::Init;
    for(auto& ch : str){
        switch(state) {
        case CodecState::Init:
            switch(ch) {
            case '\\':
                estr += ch;
                state = CodecState::InEscape;
                break;
            case '"':
                estr += '\\';
                estr += ch;
                state = CodecState::InString;
                break;
            default:
                estr += ch;
                break;
            }
            break;
        case CodecState::InEscape:
            estr += ch;
            state = CodecState::Init;
            break;
        case CodecState::InString:
            switch(ch) {
            case '\\':
                estr += ch;
                state = CodecState::InStringEscape;
                break;
            case '\r':
                break;
            case '\n':
                estr += "\\n";
                break;
            case '"':
                estr += '\\';
                estr += ch;
                state = CodecState::Init;
                break;
            default:
                estr += ch;
                break;
            }
            break;
        case CodecState::InStringEscape:
            estr += ch;
            state = CodecState::InString;
            break;
        }
    }
    return estr;
}
