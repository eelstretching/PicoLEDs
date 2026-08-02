#include "ClockWeatherAnimation.h"

#include "pico/aon_timer.h"
#include "pico/printf.h"
#include "pico/time.h"

#define S_IN_US 1000000ull

ClockWeatherAnimation::ClockWeatherAnimation(Canvas* canvas, Font* font)
    : Animation(canvas, nullptr), font(font) {}

void ClockWeatherAnimation::init() {
    lastRenderUs = time_us_64();
    render();
}

bool ClockWeatherAnimation::step() {
    uint64_t now = time_us_64();
    if (now - lastRenderUs >= S_IN_US) {
        lastRenderUs = now;
        render();
    }
    return true;
}

void ClockWeatherAnimation::setWeather(const WeatherInfo& info) {
    weather = info;
    render();
}

void ClockWeatherAnimation::setShowWeather(bool show) {
    showWeather = show;
    render();
}

const char* ClockWeatherAnimation::conditionText(WeatherCondition condition) {
    switch (condition) {
        case WeatherCondition::Sunny:
            return "SUNNY";
        case WeatherCondition::Cloudy:
            return "CLOUDY";
        case WeatherCondition::PartlyCloudy:
            return "P.CLOUDY";
        case WeatherCondition::Rain:
            return "RAIN";
        case WeatherCondition::Snow:
            return "SNOW";
    }
    return "";
}

void ClockWeatherAnimation::render() {
    struct tm tm = {};
    aon_timer_get_time_calendar(&tm);
    snprintf(timeBuf, sizeof(timeBuf), "%02d/%02d  %02d:%02d:%02d", tm.tm_mon + 1, tm.tm_mday,
             tm.tm_hour, tm.tm_min, tm.tm_sec);

    canvas->clear();
    font->render(canvas, timeBuf, 2, 10, RGB::White);

    if (showWeather && weather.valid) {
        snprintf(weatherBuf, sizeof(weatherBuf), "%s %dF (H%d L%d)", conditionText(weather.condition),
                 weather.currentTempF, weather.highTempF, weather.lowTempF);
        font->render(canvas, weatherBuf, 2, 0, RGB::Cyan);
    }
}
