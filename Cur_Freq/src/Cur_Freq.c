#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"
#include "pico/multicore.h"
#include "cur.h"
#include "display.h"

// Entrada de frecuencia
#define LED_BLUE    2

/* PROTOTIPOS DE FUNCIONES */
static void ledBlink_init(void);
static void ledBlink(void);
/*..................................................................*/

/* CUERPO DE FUNCIONES */
int main()
{
    stdio_init_all();
    sleep_ms(1000); // Espera 2 segundos

    display_init();
    ledBlink_init();
    cur_init();

    // Lanza programa en el otro nucleo
    multicore_launch_core1(core1_entry);

    while (true)
    {
        ledBlink();
        // printf("Pico W");
    }

    return 0;
}
/*..................................................................*/
static void ledBlink_init(void)
{
    gpio_init(LED_BLUE);
    gpio_set_dir(LED_BLUE, GPIO_OUT);
    gpio_put(LED_BLUE, true);

    return;
}
static void ledBlink(void)
{
    gpio_put(LED_BLUE, !gpio_get(LED_BLUE));
    sleep_ms(500);

    return;
}
