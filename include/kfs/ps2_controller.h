#ifndef __PS2_CONTROLLER_H__
# define __PS2_CONTROLLER_H__

#include <stdint.h>


extern uint8_t		wait_ps2_to_write(void);
extern uint8_t		wait_ps2_to_read(void);

extern uint8_t	send_command(uint8_t port, uint8_t cmd,
							 uint8_t data, uint8_t send_data,
							 uint8_t await_response);

#endif