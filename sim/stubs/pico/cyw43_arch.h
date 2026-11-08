#pragma once
// The simulator has no radio; the Pico 2 W's LED calls do nothing.
#define CYW43_WL_GPIO_LED_PIN 0
static inline int cyw43_arch_init(void) { return 0; }
static inline void cyw43_arch_deinit(void) {}
static inline void cyw43_arch_gpio_put(unsigned int pin, bool value) {
    (void)pin;
    (void)value;
}
