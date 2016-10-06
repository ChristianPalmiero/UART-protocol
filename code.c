/* 
 * "Small Hello World" example. 
 * 
 * This example prints 'Hello from Nios II' to the STDOUT stream. It runs on
 * the Nios II 'standard', 'full_featured', 'fast', and 'low_cost' example 
 * designs. It requires a STDOUT  device in your system's hardware. 
 *
 * The purpose of this example is to demonstrate the smallest possible Hello 
 * World application, using the Nios II HAL library.  The memory footprint
 * of this hosted application is ~332 bytes by default using the standard 
 * reference design.  For a more fully featured Hello World application
 * example, see the example titled "Hello World".
 *
 * The memory footprint of this example has been reduced by making the
 * following changes to the normal "Hello World" example.
 * Check in the Nios II Software Developers Manual for a more complete 
 * description.
 * 
 * In the SW Application project (small_hello_world):
 *
 *  - In the C/C++ Build page
 * 
 *    - Set the Optimization Level to -Os
 * 
 * In System Library project (small_hello_world_syslib):
 *  - In the C/C++ Build page
 * 
 *    - Set the Optimization Level to -Os
 * 
 *    - Define the preprocessor option ALT_NO_INSTRUCTION_EMULATION 
 *      This removes software exception handling, which means that you cannot 
 *      run code compiled for Nios II cpu with a hardware multiplier on a core 
 *      without a the multiply unit. Check the Nios II Software Developers 
 *      Manual for more details.
 *
 *  - In the System Library page:
 *    - Set Periodic system timer and Timestamp timer to none
 *      This prevents the automatic inclusion of the timer driver.
 *
 *    - Set Max file descriptors to 4
 *      This reduces the size of the file handle pool.
 *
 *    - Check Main function does not exit
 *    - Uncheck Clean exit (flush buffers)
 *      This removes the unneeded call to exit when main returns, since it
 *      won't.
 *
 *    - Check Don't use C++
 *      This builds without the C++ support code.
 *
 *    - Check Small C library
 *      This uses a reduced functionality C library, which lacks  
 *      support for buffering, file IO, floating point and getch(), etc. 
 *      Check the Nios II Software Developers Manual for a complete list.
 *
 *    - Check Reduced device drivers
 *      This uses reduced functionality drivers if they're available. For the
 *      standard design this means you get polled UART and JTAG UART drivers,
 *      no support for the LCD driver and you lose the ability to program 
 *      CFI compliant flash devices.
 *
 *    - Check Access device drivers directly
 *      This bypasses the device file system to access device drivers directly.
 *      This eliminates the space required for the device file system services.
 *      It also provides a HAL version of libc services that access the drivers
 *      directly, further reducing space. Only a limited number of libc
 *      functions are available in this configuration.
 *
 *    - Use ALT versions of stdio routines:
 *
 *           Function                  Description
 *        ===============  =====================================
 *        alt_printf       Only supports %s, %x, and %c ( < 1 Kbyte)
 *        alt_putstr       Smaller overhead than puts with direct drivers
 *                         Note this function doesn't add a newline.
 *        alt_putchar      Smaller overhead than putchar with direct drivers
 *        alt_getchar      Smaller overhead than getchar with direct drivers
 *
 */

#include <stdio.h>
#include <sys/alt_timestamp.h>
#include <system.h>
#include <altera_avalon_pio_regs.h>

#define BAUD_RATE 10000
#define NBIT 8
#define STOPBIT 1
#define NOPARITY 0
#define EVENPARITY 1
#define ODDPARITY 2
#define PARITY EVENPARITY

int main(void){

int val, ticksPerSec, t1, t2, nticks, ticksPerBit, ticksPerHalf, totalbit;
int bitval, parity, data=0;
int cntparity=0, i;

alt_timestamp_start();
ticksPerSec= alt_timestamp_freq();
t1 = alt_timestamp();
t2 = alt_timestamp();
nticks= t2 - t1;

ticksPerBit=ticksPerSec/BAUD_RATE;
ticksPerHalf=ticksPerBit/2;
totalbit=1 + NBIT + (PARITY != 0 ? 1 : 0) + STOPBIT;

while(1){
	data=0;
	cntparity=0;
	do{
		val= IORD_ALTERA_AVALON_PIO_DATA(PIO_0_BASE);
		val= val & 0x00000001;
	}while(val!=0);

	//Read the start bit
	alt_timestamp_start();
	while(alt_timestamp() < ticksPerHalf);
	val= IORD_ALTERA_AVALON_PIO_DATA(PIO_0_BASE);
	val= val& 0x00000001;
	if(val==1)
		printf("Error receiving the start bit\n");
	//Read the data
	for(i=0; i<NBIT;i++){
		alt_timestamp_start();
		while(alt_timestamp() < ticksPerBit);
		bitval= IORD_ALTERA_AVALON_PIO_DATA(PIO_0_BASE);
		bitval= bitval& 0x00000001;

		if(bitval==1)
			cntparity++;

		bitval=bitval<<i;   //shift to left
		data=bitval|data;
	}
	//Read the parity
	if(PARITY>0)
		parity=cntparity%2;
	alt_timestamp_start();
	while(alt_timestamp() < ticksPerBit);
	val= IORD_ALTERA_AVALON_PIO_DATA(PIO_0_BASE);
	val= val& 0x00000001;
	if(val!=parity)
		printf("Single bit error\n");
	//Read stop bit(s)
	for(i=0; i<STOPBIT;i++){
		alt_timestamp_start();
		while(alt_timestamp() < ticksPerBit);
		val= IORD_ALTERA_AVALON_PIO_DATA(PIO_0_BASE);
		val= val& 0x00000001;

		if(val==0)
			printf("Stop bit error\n");
	}

	printf("Data output: %d - %c\n", data, data);
}

return 0;
}
