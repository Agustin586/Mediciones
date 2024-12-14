#include "display.h"

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"

#include "cur.h"
#include "i2c_lcd.h"

// I2C defines
#define I2C_PORT i2c0
#define I2C_SDA 4
#define I2C_SCL 5

/* CUERPO DE FUNCIONES EXTERNAS */
extern void display_init(void)
{
#if !defined(i2c_default) || !defined(PICO_DEFAULT_I2C_SDA_PIN) || !defined(PICO_DEFAULT_I2C_SCL_PIN)
#warning i2c/lcd_1602_i2c example requires a board with I2C pins
#else
    // This example will use I2C0 on the default SDA and SCL pins (4, 5 on a Pico)
    i2c_init(i2c_default, 100 * 1000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
    // Make the I2C pins available to picotool
    bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));

    lcd_init();

    sleep_ms(1000);

    lcd_set_cursor(2, 2);
    lcd_string("Medidor de AWG.");
    lcd_set_cursor(0, 0);
    lcd_string("Mediciones II");

    sleep_ms(2000);

    lcd_clear();

    display_freq();
#endif

    return;
}
/*...................................................*/
extern void display_freq(void)
{
    char buffer[20];

    // Limpia la pantalla
    // lcd_clear();

    // Pantalla principal
    // lcd_set_cursor(0, 0);
    // lcd_string("Freq:");

    // Carga valor de frecuencia
    // lcd_set_cursor(1, 0);
    // sprintf(buffer, "%.5f Hz", cur_freq());
    // lcd_string(buffer);

    return;
}
/*...................................................*/
