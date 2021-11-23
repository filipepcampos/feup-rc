#ifndef __CORE__
#define __CORE__

#include "common.h"

/**
 * @brief Create a non information frame object (any frame other than INFO)
 * 
 * @param control Frame control byte
 * @param address Address byte
 * @return framecontent filled with the arguments given
 */
framecontent create_non_information_frame(uint8_t control, uint8_t address);

/**
 * @brief Create a information frame object
 * 
 * @param data Pointer to data to be sent
 * @param data_len Data size
 * @param S Sequence bit
 * @param address Address byte
 * @return framecontent filled with the arguments given
 */
framecontent create_information_frame(uint8_t *data, size_t data_len, int S, uint8_t address);

/**
 * @brief Apply byte stuffing to data, note that the data buffer 
 * must have at least 2*data_len of space to avoid any error.
 * @param data Pointer to data
 * @param data_len Data size
 * @return size_t New data size
 */
size_t byte_stuffing(uint8_t *data, size_t data_len);

/**
 * @brief Apply byte destuffing to data * 
 * @param buffer Pointer to buffer
 * @param buf_size Buffer size
 * @return size_t New buffer size
 */
size_t byte_destuffing(uint8_t* buffer, size_t buf_size);

/**
 * @brief Calculate BCC for an array of bytes
 * @param data 
 * @param data_len 
 * @return uint8_t BCC value
 */
uint8_t calculate_bcc(uint8_t *data, size_t data_len);

#endif