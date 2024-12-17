#include "cur.h"

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/timer.h"

#include "display.h"
#include "i2c_lcd.h"

typedef struct flipflop_D
{
    bool Q;
    bool Qn;
    bool D;
    bool FallEdge;
} flipflop_D;

typedef uint32_t T_Cont;   // Variable de tipo contador --> Puede ser cambiada si necesitamos mayor valor.
typedef uint32_t T_FreqIN; // Misma idea pero con frecuencia de entrada.
typedef uint32_t T_FreqOut;

typedef struct DecDivN
{
    T_Cont N;
    T_Cont Cont;
    T_FreqIN Fin;
    T_FreqOut Fout;
    bool RiseEdge;
    bool Q;
} DecDivN;

typedef struct Monoestable
{
    bool Signal;
    bool Salida;
} Monoestable;

typedef struct Contador
{
    uint32_t N;
    uint32_t Cont;
} Contador;

typedef struct cur
{
    // Declaracion de los flip flops tipo D.
    flipflop_D FF_1;
    flipflop_D FF_2;

    // Declaracion de la decada divisora.
    DecDivN Decada_Div;

    // Declaracion de los monosestables. --> Se implementaron como uno solo en una funcion
    // Monoestable MT;
    // Monoestable MB;

    // Contadores
    Contador Nm;
    Contador Nr;
} cur;

/* DECLARACION DE VARIABLES */
cur ContUniRev;
volatile bool flag = false;
repeating_timer_t timer_Fr;

/* PROTOTIPO DE FUNCIONES */
/**
 * @brief Esta función se encarga de replicar el funcionamiento de
 * un flip flop tipo D.
 * @param flipflop_D Flip Flop tipo D.
 * @param D Estado de la entrada D.
 */
static void flipflop(flipflop_D *flipflop, bool D);
/**
 * @brief Inicializa el flip y sus variables internas.
 * @param flipflop_D Flip Flop tipo D.
 * @param Q Salida no negada.
 * @param Qn Salida negada.
 * @param D Entrada tipo D.
 */
static void flipflop_init(flipflop_D *flipflop, bool Q, bool Qn, bool D);
/**
 * @brief Resetea las variables del flipflop.
 * @param flipflop_D Flip Flop tipo D.
 */
static void flipflop_reset(flipflop_D *flipflop);

/**
 * @brief Inicializacion del contador.
 * @param Nx Variable tipo contador.
 */
static void Contador_init(Contador *Nx);
/**
 * @brief Incremento del contador.
 * @param Nx Variable tipo contador.
 */
static void Contador_inc(Contador *Nx);
/**
 * @brief Reseteo del contador.
 * @param Nx Variable de tipo contador.
 */
static void Contador_reset(Contador *Nx);

/**
 * @brief Inicializacion de la decada divisora.
 * @param DecDiv Variable del tipo DecDivN.
 */
static void DecadaDiv_init(DecDivN *DecDiv, T_Cont N, T_FreqIN Fin, T_FreqOut Fout);
/**
 * @brief Divide la frecuencia de entrada N veces.
 * @param DecDiv Variable del tipo DecDivN.
 * @return Estado de la salida.
 */
static void DecadaDiv(DecDivN *DecDiv);

/**
 * @brief Funcion de callback del timer Fr.
 * @return Devuelve el valor en tiempo para el proximo periodo.
 */
bool Temp_Fr_callback(repeating_timer_t *rtimer);
/**
 * @brief Inicializacion de los temporizadores.
 */
static void Temp_init(void);

/**
 * @brief Setea los monoestables.
 * @param Mx Variable del tipo Monoestable.
 */
static void Monoestable_set(void);

/**
 * @brief Rutina de interrupcion de la fe (frecuencia de entrada).
 */
static void isr_fe(uint gpio, uint32_t events);
static void configure_gpio2_rising_edge_fast_slew(void);

/* CUERPO DE FUNCIONES INTERNAS */
static void flipflop_init(flipflop_D *flipflop, bool Q, bool Qn, bool D)
{
    flipflop->D = D;
    flipflop->Q = Q;
    flipflop->Qn = Qn;
    flipflop->FallEdge = false;

    return;
}
/*.....................................................................*/
static void flipflop_reset(flipflop_D *flipflop)
{
    flipflop->D = 0;
    flipflop->Q = 0;
    flipflop->Qn = 1;
    flipflop->FallEdge = false;

    return;
}
/*.....................................................................*/
static void flipflop(flipflop_D *flipflop, bool D)
{
    flipflop->D = D;

    // Detecta el flaco descendente
    if (flipflop->Q == 1 && D == 0)
        flipflop->FallEdge = true;

    flipflop->Q = flipflop->D;
    flipflop->Qn = !flipflop->Q;

    return;
}

/*.....................................................................*/
static void DecadaDiv_init(DecDivN *DecDiv, T_Cont N, T_FreqIN Fin, T_FreqOut Fout)
{
    DecDiv->N = N;
    DecDiv->Cont = N;
    DecDiv->Fin = Fin;
    DecDiv->Fout = Fout;
    DecDiv->Q = false;
    DecDiv->RiseEdge = false;

    return;
}
/*.....................................................................*/
static void DecadaDiv(DecDivN *DecDiv)
{
    DecDiv->Cont--;

    if (!DecDiv->Cont) // Cuando cont = 0
    {
        DecDiv->Cont = DecDiv->N; // Reincia

        if (DecDiv->Q == 0)
            DecDiv->RiseEdge = true; // Flanco ascendente habilitado

        DecDiv->Q = !DecDiv->Q; // Togglea la salida

        // printf("Nr:%d\n", ContUniRev.Nr.Cont);
    }

    // /* Ensayo: fr y fn */ // Chequeado
    // if (DecDiv->Q)
    // {
    //     gpio_put(7, true);
    // }
    // else
    // {
    //     gpio_put(7, false);
    // }
    // /*----------------*/

    return;
}
/*.....................................................................*/

static void Contador_init(Contador *Nx)
{
    Nx->Cont = 0;

    return;
}
/*.....................................................................*/
static void Contador_reset(Contador *Nx)
{
    Nx->Cont = 0;

    return;
}
/*.....................................................................*/
static void Contador_inc(Contador *Nx)
{
    Nx->Cont++;

    return;
}
/*.....................................................................*/

#define FREQ_Fr 100000

static void Temp_init(void)
{
    uint16_t delay_us = FREQ_Fr / 1000;
    bool status = add_repeating_timer_us(10, Temp_Fr_callback, NULL, &timer_Fr);

    // printf("Estado del temporizador: %s\n", status ? "Exitoso" : "Fallido");

    return;
}
/*.....................................................................*/
bool Temp_Fr_callback(repeating_timer_t *rtimer)
{
    DecadaDiv(&ContUniRev.Decada_Div);

    if (ContUniRev.FF_2.Q) // AND nro 2
    // if (ContUniRev.FF_1.Q) // Para probar
    {
        Contador_inc(&ContUniRev.Nr); // Cuentas de Nr
    }

    // /* Ensayo: verificar fr */
    // gpio_put(8, !gpio_get(8)); // Chequeado
    // /*----------------------*/

    if (ContUniRev.Decada_Div.RiseEdge)
    {
        flipflop(&ContUniRev.FF_1, ContUniRev.FF_1.Qn); // D se conecta con Qn

        ContUniRev.Decada_Div.RiseEdge = false; // Deshabilita el flanco
    }

    if (ContUniRev.FF_1.Q)
    {
        /* Ensayo: salida del ff1 */ // Chequeado
        gpio_put(7, true);
        /*------------------------*/
    }
    else
    {
        /* Ensayo: salida del ff1 */
        gpio_put(7, false);
        /*------------------------*/
    }

    // DecadaDiv(&ContUniRev.Decada_Div);

    return true;
}
/*.....................................................................*/
static void isr_fe(uint gpio, uint32_t events)
{
    if (gpio == 2 && (events & GPIO_IRQ_EDGE_RISE))
    {
        // (ContUniRev.FF_1.Q) ? printf("FF1 Q high\n"): printf("FF1 Q low\n");

        flipflop(&ContUniRev.FF_2, ContUniRev.FF_1.Q); // Q1 se conecta a D2

        if (ContUniRev.FF_2.Q) // AND 2
        // if (ContUniRev.FF_1.Q) // Para probar!!
        {
            /* Ensayo: salida de ff2 */
            gpio_put(8, true);
            /*-----------------------*/

            Contador_inc(&ContUniRev.Nm); // Incremento de Nm
            //  printf("Nm:%d",ContUniRev.Nm.Cont);
        }

        if (ContUniRev.FF_2.FallEdge)
        {
            /* Ensayo: salida de ff2 */
            gpio_put(8, false);
            /*-----------------------*/

            Monoestable_set(); // Resetea las variables

            ContUniRev.FF_2.FallEdge = false; // Limpia el flanco descendente
            // printf("Toggle FF2\n");
        }

        // printf("Fe\n");
    }

    return;
}
/*.....................................................................*/
void configure_gpio2_rising_edge_fast_slew(void)
{
    // Selecciona el pin GPIO 2
    uint gpio_pin = 2;

    // Configura el GPIO 2 como entrada
    gpio_init(gpio_pin);
    gpio_set_dir(gpio_pin, GPIO_IN);

    // Habilita la detección de flancos ascendentes
    gpio_set_irq_enabled_with_callback(2, GPIO_IRQ_EDGE_RISE, true, &isr_fe);
    // gpio_set_irq_enabled(gpio_pin, GPIO_IRQ_EDGE_RISE, true);

    // Configura el slew rate a rápido
    gpio_set_slew_rate(gpio_pin, GPIO_SLEW_RATE_FAST);

    // Opcional: Configura el pull resistor si es necesario
    // gpio_pull_up(gpio_pin); // Habilita un pull-up interno
    // gpio_pull_down(gpio_pin); // Alternativamente, habilita un pull-down interno

    // Nota: Agrega un handler de interrupción si planeas utilizar interrupciones.
}
/*.....................................................................*/

static void Monoestable_set(void)
{
    // Desactiva el temporizador de fr
    cancel_repeating_timer(&timer_Fr);

    // Desactiva la interrupcion de fe
    gpio_set_irq_enabled_with_callback(2, GPIO_IRQ_EDGE_RISE, false, &isr_fe);

    // printf("ISR desactivados");

    // Transferencia de informacion a la pantalla
    // display_freq();
    cur_freq();

    // Reseteo de objetos flipflops
    flipflop_reset(&ContUniRev.FF_1);
    flipflop_reset(&ContUniRev.FF_2);
    Contador_reset(&ContUniRev.Nm);
    Contador_reset(&ContUniRev.Nr);

    // Activo el temporizador fr
    Temp_init();

    // Activa la interrupcion de fe
    gpio_set_irq_enabled_with_callback(2, GPIO_IRQ_EDGE_RISE, true, &isr_fe);

    // printf("ISR activados");

    return;
}
/*.....................................................................*/

/* CUERPO DE FUNCIONES EXTERNAS */
/*.....................................................................*/
// #define DECADA_DIV_N 10000
// #define DECADA_DIV_N 50000
#define DECADA_DIV_N 5000
#define DECADA_DIV_FIN 100000
#define DECADA_DIV_FOUT 10

extern void cur_init(void)
{
    // Configura el pin de entrada
    configure_gpio2_rising_edge_fast_slew();

    // Inicializacion de los flipflops
    flipflop_init(&ContUniRev.FF_1, false, true, false);
    flipflop_init(&ContUniRev.FF_2, false, true, false);

    // Inicializacion de los contadores
    Contador_init(&ContUniRev.Nm);
    Contador_init(&ContUniRev.Nr);

    // Inicializacion de la decada divisora
    DecadaDiv_init(&ContUniRev.Decada_Div, DECADA_DIV_N, DECADA_DIV_FIN, DECADA_DIV_FOUT);

    // Inicialzacion de temporizadores
    Temp_init();

    gpio_init(8);
    gpio_set_dir(8, GPIO_OUT);
    gpio_set_slew_rate(8, GPIO_SLEW_RATE_FAST);
    gpio_init(7);
    gpio_set_dir(7, GPIO_OUT);

    return;
}
/*.....................................................................*/
extern float cur_freq(void)
{
    // return 124.3;
    float resultado = (float)((ContUniRev.Nm.Cont * 1.0) / ContUniRev.Nr.Cont);

    printf("Nm:%d\t\tNr:%d\n", ContUniRev.Nm.Cont, ContUniRev.Nr.Cont);
    // printf("Nr:%d\n", ContUniRev.Nr.Cont);

    // printf("%.2f\n", resultado);

    return resultado;
}
/*.....................................................................*/