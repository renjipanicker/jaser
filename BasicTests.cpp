#include "Json.hpp"

int test1() {
    auto etxtext =
    "{"
        "\\\"value\\\":\\\"qqq1\\n\\\""
    "}";

    std::string str = posdk::Json::decodeString(etxtext);
    auto tx = posdk::Json::loadFromString(str);
    std::cout << "committing-1:" << posdk::Json::saveToString(tx) << std::endl;
    return 0;
}

int test2() {
    auto etxtext =
    "{"
      "\\\"ChannelID\\\":\\\"meghdhoot.segito.net\\\","
      "\\\"Record\\\":{"
        "\\\"Creator\\\":{"
            "\\\"UserID\\\":\\\"qqq@u.segito.net\\\""
          "},"
        "\\\"DocumentID\\\":\\\"75d376cc-e3a5-4daa-89b5-8e3a476e9ec7\\\","
        "\\\"Reply\\\":["
          "{"
            "\\\"value\\\":\\\"qqq1\\n\\\""
          "}"
        "]"
      "}"
    "}";

    std::string str = posdk::Json::decodeString(etxtext);
    auto tx = posdk::Json::loadFromString(str);
    std::cout << "committing-2:" << posdk::Json::saveToString(tx) << std::endl;
    return 0;
}

int test3() {
    auto etxtext =

" {"
"  \"Text\":\"{\\\"Action\\\":\\\"CreateChannel\\\"}\","
"   \\\"value\\\":\\\"qqq1\\n\\\""
"}"
;

    std::string str = posdk::Json::decodeString(etxtext);
    auto tx = posdk::Json::loadFromString(str);
    std::cout << "committing-3:" << posdk::Json::saveToString(tx) << std::endl;
    return 0;
}

int main(int argc, char* argv[]) {
    test1();
    test2();
    test3();
    return 0;
}
