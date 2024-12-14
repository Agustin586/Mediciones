#ifndef I2C_LCD_H_
#define I2C_LCD_H_

#include <stdio.h>
#include "pico/stdlib.h"

/* PROTOTIPOS DE FUNCIONES EXTERNAS */
/**
 * @brief Toggle la posicion.
 */
void lcd_toggle_enable(uint8_t val);
/**
 * @brief Envia un byte.
 */
void lcd_send_byte(uint8_t val, int mode);
/**
 * @brief Limpia la pantalla.
 */
void lcd_clear(void);
/**
 * @brief Setea el cursor en la posicion especificada.
 * @param line Linea del cursor.
 * @param position Columna del cursor.
 */
void lcd_set_cursor(int line, int position);
/**
 * @brief Envia una cadena de caracteres.
 * @param s string.
 */
void lcd_string(const char *s);
/**
 * @brief Inicializa la pantalla lcd.
 */
void lcd_init();

#endif