// #include <stdio.h>
// #include "pico/stdlib.h"


// int main()
// {
//     stdio_init_all();

//     while (true) {
//         printf("Hello, world!\n");
//         sleep_ms(1000);
//     }
// }

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

/**
 * @brief Función callback de fe.
 */
void callback_fe(uint gpio, uint32_t events);

void callback_fr(uint gpio, uint32_t events);

// Definir el pin que usaremos
#define PIN_BUTTON 2 

// Callback para manejar la interrupción
void callback_fe(uint gpio, uint32_t events) {
    if (events & GPIO_IRQ_EDGE_RISE) {
        printf("Flanco ascendente detectado en el pin %d\n", gpio);
    }
}

cur_init(void) {
    // Inicializa la comunicación serie para la depuración
    stdio_init_all();

    // Configura el GPIO
    gpio_init(PIN_BUTTON);                  // Inicializa el pin
    gpio_set_dir(PIN_BUTTON, GPIO_IN);      // Configura como entrada
    gpio_pull_down(PIN_BUTTON);             // Habilita el pull-down interno

    // Configura la interrupción para el flanco ascendente
    gpio_set_irq_enabled_with_callback(
        PIN_BUTTON,                         // Pin
        GPIO_IRQ_EDGE_RISE,                 // Detectar flanco ascendente
        true,                               // Habilitar la interrupción
        &callback_fe                        // Callback de fe
    );

    gpio_set_irq_enabled_with_callback(
        PIN_BUTTON,                         // Pin
        GPIO_IRQ_EDGE_RISE,                 // Detectar flanco ascendente
        true,                               // Habilitar la interrupción
        &callback_fr                        // Callback de fr
    );

    while (true) {
        // Bucle principal
        tight_loop_contents();
    }

    return 0;
}
