#include "appIO.h"

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h"

#define LEVEL_SENSOR_GPIO GPIO_NUM_12
#define RELAY_GPIO GPIO_NUM_13

uint8_t relayState = 1;

void IO_peripheralsInit(void) {
    gpio_pad_select_gpio(LEVEL_SENSOR_GPIO);
    gpio_set_direction(LEVEL_SENSOR_GPIO, GPIO_MODE_INPUT);
    // gpio_pullup_en(LEVEL_SENSOR_GPIO);
    gpio_set_pull_mode(LEVEL_SENSOR_GPIO, GPIO_PULLUP_ONLY);

    gpio_pad_select_gpio(RELAY_GPIO);
    gpio_set_direction(RELAY_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(RELAY_GPIO, 1);
}

uint8_t IO_relayToggle(void)
{
    relayState = !relayState;
    gpio_set_level(RELAY_GPIO, relayState);

    return relayState;
}

uint8_t IO_setStateRelay(uint8_t state)
{
    relayState = state;
    return gpio_set_level(RELAY_GPIO, state);
}

uint8_t IO_getStateRelay(void)
{
    return relayState;
}

uint8_t IO_getStateLevelSensor(void)
{
    return gpio_get_level(LEVEL_SENSOR_GPIO);
}
