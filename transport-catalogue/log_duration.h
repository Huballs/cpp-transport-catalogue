// benchmark.hh
#pragma once
#include <chrono>
#include <iostream>
#include <string>
#include <string_view>

#define PROFILE_CONCAT_INTERNAL(X, Y) X ## Y
#define PROFILE_CONCAT(X, Y) PROFILE_CONCAT_INTERNAL(X, Y)
#define UNIQUE_VAR_NAME_PROFILE PROFILE_CONCAT(profileGuard, __LINE__)
#define LOG_DURATION(x) LogDuration UNIQUE_VAR_NAME_PROFILE(x) 

/*Такого трюка для определения UNIQUE_VAR_NAME_PROFILE — макроса, генерирующего уникальное 
имя переменной — требуют довольно запутанные правила раскрытия в C++. Параметры макроса при 
склеивании заменяются на то, что в них было подставлено без изменения. Те параметры, которые 
не склеиваются, раскрываются, то есть полностью подставляются до того момента, пока в них не 
останется макросов. Чтобы достичь желаемого, нужно, чтобы __LINE__ побывал параметром два 
раза: в первый раз он раскроется в номер строки, во второй раз номер строки приклеится к 
имени переменной.*/

class LogDuration {
public:
    using Clock = std::chrono::steady_clock;

    LogDuration(std::string_view id, std::ostream& dst_stream = std::cerr)
        : id_(id)
        , dst_stream_(dst_stream) {
    }

    ~LogDuration() {
        using namespace std::chrono;
        using namespace std::literals;

        const auto end_time = Clock::now();
        const auto dur = end_time - start_time_;
        dst_stream_ << id_ << ": "sv << duration_cast<microseconds>(dur).count() << " u"sv << std::endl;
    }

private:
    const std::string id_;
    const Clock::time_point start_time_ = Clock::now();
    std::ostream& dst_stream_;
}; 
