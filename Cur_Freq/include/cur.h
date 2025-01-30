#ifndef CUR_H_
#define CUR_H_

/**
 * @brief Inicializacion de todos los parametros del cur.
 */
extern void cur_init(void);
/**
 * @brief Obtiene la frecuencia del cur.
 * @return Valor flotante de frecuencia.
 */
extern float cur_freq(void);
/**
 * @brief Programa del nucleo 1.
 */
extern void core1_entry(void);

#endif