#ifndef __DATALINKEMITTER__
#define __DATALINKEMITTER__

#include "common.h"

/**
 * @brief Fill buffer with the content of the frame
 * 
 * @param buffer Buffer to be filled
 * @param buffer_size Size of the buffer
 * @param fc Content of the frame
 * @return int 0 if success, -1 if failure
 */
int frame_to_bytes(uint8_t *buffer, size_t buffer_size, framecontent *fc);

/**
 * @brief Write frame content to file descriptor
 * 
 * @param fd File descriptor
 * @param frame Frame to be written
 * @param frame_size Size of frame
 * @return int 0 if success, -1 if failure
 */
int send_bytes(int fd, uint8_t *frame, size_t frame_size);

/**
 * @brief Send frame
 * 
 * @param fd File descriptor
 * @param fc Frame 
 * @return int 0 if success, -1 if failure
 */
int emit_frame(int fd, framecontent *fc);

/**
 * @brief Send frame until there is a response
 * 
 * @param fd File descriptor
 * @param fc Frame
 * @param expected_response Expected response from receiver
 * @return int 0 if success, 1 if max attempts reached, -1 if failure
 */
int emit_frame_until_response(int fd, framecontent *fc, uint8_t expected_response);

#endif