#include "ModeController.h"

#include "pico/aon_timer.h"
#include "pico/time.h"

ModeController::ModeController(Canvas* canvas, Font* font, RandomAnimator* vibeAnimator,
                                Mailbox* mailbox)
    : clockAnimation(canvas, font),
      meetingAnimation(canvas, font),
      darkAnimation(canvas),
      clockAnimator(canvas),
      meetingAnimator(canvas),
      darkAnimator(canvas),
      vibeAnimator(vibeAnimator),
      mailbox(mailbox) {
    clockAnimator.add(&clockAnimation);
    meetingAnimator.add(&meetingAnimation);
    darkAnimator.add(&darkAnimation);
}

void ModeController::handleMessage(const SignMessage& msg) {
    switch (msg.type) {
        case MessageType::SetTime: {
            struct timespec ts;
            ts.tv_sec = (time_t)msg.localEpochSeconds;
            ts.tv_nsec = 0;
            aon_timer_set_time(&ts);
            break;
        }

        case MessageType::Weather: {
            WeatherInfo info;
            info.condition = msg.weatherCondition;
            info.currentTempF = msg.weatherCurrentTempF;
            info.highTempF = msg.weatherHighTempF;
            info.lowTempF = msg.weatherLowTempF;
            info.valid = true;
            clockAnimation.setWeather(info);
            break;
        }

        case MessageType::ClockMode:
            clockAnimation.setShowWeather(msg.clockShowWeather);
            setMode(SignMode::Clock);
            break;

        case MessageType::MeetingMode:
            meetingAnimation.configure(msg.meetingDurationMinutes, msg.meetingParticipantName,
                                        msg.meetingTzOffsetMinutes, msg.meetingStyle);
            meetingEndTimeUs =
                time_us_64() + (uint64_t)msg.meetingDurationMinutes * 60ull * 1000000ull;
            setMode(SignMode::Meeting);
            break;

        case MessageType::VibeMode:
            setMode(SignMode::Vibe);
            break;

        case MessageType::GoDark:
            setMode(SignMode::Dark);
            break;
    }
}

void ModeController::step() {
    if (mode == SignMode::Meeting && time_us_64() >= meetingEndTimeUs) {
        // The meeting's stated duration has elapsed with no follow-up
        // message; fall back to the clock rather than sitting on stale
        // meeting info indefinitely.
        setMode(SignMode::Clock);
    }

    if (mode == SignMode::Vibe && !mailbox->isConnected() &&
        mailbox->microsSinceDisconnected() >= kVibeFallbackAfterDisconnectUs) {
        // Nobody's been around to change it for a while; fall back to the
        // clock rather than looping Vibe Mode's animations indefinitely with
        // no laptop attached. Deliberately doesn't touch Dark Mode -- "Go
        // Dark" is an explicit instruction, and a BLE hiccup (or the laptop
        // just being asleep) shouldn't quietly turn the lights back on.
        setMode(SignMode::Clock);
    }

    switch (mode) {
        case SignMode::Clock:
            clockAnimator.step();
            break;
        case SignMode::Meeting:
            meetingAnimator.step();
            break;
        case SignMode::Vibe:
            vibeAnimator->step();
            break;
        case SignMode::Dark:
            darkAnimator.step();
            break;
    }
}

void ModeController::setMode(SignMode newMode) {
    if (newMode == mode) {
        return;
    }
    mode = newMode;
    switch (mode) {
        case SignMode::Clock:
            clockAnimator.init();
            break;
        case SignMode::Meeting:
            meetingAnimator.init();
            break;
        case SignMode::Vibe:
            vibeAnimator->init();
            break;
        case SignMode::Dark:
            darkAnimator.init();
            break;
    }
}
