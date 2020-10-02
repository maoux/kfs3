#include <kfs/io.h>
#include <kfs/ps2_controller.h>


extern uint8_t		wait_ps2_to_write(void)
{
	uint8_t		status;

	//wait for bit 1 to be clear
	do {
		status = inb(0x64);
	} while (status & 0x02);
	return (status);
}

extern uint8_t		wait_ps2_to_read(void)
{
	uint8_t		status;

	//wait for bit 0 to be set
	do {
		status = inb(0x64);
	} while (!(status & 0x01));
	return (status);
}

extern uint8_t		send_command(uint8_t port, uint8_t cmd,
								 uint8_t data, uint8_t send_data,
								 uint8_t await_response)
{
	uint8_t		response = 0x0;

	wait_ps2_to_write();
	//send command code to controller
	outb(port, cmd);
	if (send_data) {
		wait_ps2_to_write();
		//send data byte to data port
		outb(0x60, data);
	}
	if (await_response) {
		wait_ps2_to_read();
		//read response from data port
		response = inb(0x60);
	}
	return (response);
}
