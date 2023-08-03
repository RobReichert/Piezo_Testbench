#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <sched.h>
#include <fcntl.h>
#include <math.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define TCP_PORT 1001

#define CMA_ALLOC _IOWR('Z', 0, uint32_t)

// Starting RP Config
#define FIXED_FREQ_INIT 65536			// 1Hz			
#define SAMPLING_DIVIDER_INIT 1250  	// 100 kHz

#define MODE_MASK 192
#define TRIG_MASK 4
#define CONFIG_ACK 2

int interrupted = 0;

typedef struct config_struct {
	uint16_t trigger;
	uint16_t mode;
	uint16_t CIC_divider;
	int32_t dds_phase;
	int32_t param_b;
	int32_t param_c;
	int32_t param_d;
	int32_t param_e;
	int32_t param_f;
	int32_t param_g;
	int32_t param_h;
} config_t;

typedef struct system_pointers {
	volatile uint8_t *rx_com;//*rx_rst;
	volatile uint16_t *rx_rate;
	volatile uint32_t *rx_addr;
	volatile uint32_t *rx_cntr;
	volatile uint16_t *rx_fb;
	void *ram;
} system_pointers_t;

typedef struct parameters {
	//volatile int32_t *param_a;
	//volatile int32_t *param_b;
	//volatile int32_t *param_c;
	//volatile int32_t *param_d;
	//volatile int32_t *param_e;
	//volatile int32_t *param_f;
	//volatile int32_t *param_g;
	//volatile int32_t *param_h;
	//volatile uint16_t *rx_rate;
	volatile uint16_t *dds_phase;
} params_t;

void signal_handler(int sig) {
	interrupted = 1;
}

// Receive message "header" bytes. Return acknowledge of config send if header ==0, otherwise echo back
// bytes_to_send to confirm a recording request
uint32_t get_socket_type(int sock_client)
{
	int32_t message = 0;
	uint32_t config_ack = 2;

	if(recv(sock_client, &message, sizeof(message), 0) > 0) {
		
		printf("Request message: %d\n", message);

		if (message == 0) {
			if (send(sock_client, &config_ack, sizeof(config_ack), MSG_NOSIGNAL) == sizeof(config_ack)) {
				return message;
			} else {
				perror("Message ack send failed");
				return EXIT_FAILURE;
			}
		} else {
			if (send(sock_client, &message, sizeof(message), MSG_NOSIGNAL) == sizeof(message)) {
				return message;
			} else {
				perror("Message ack send failed");
				return EXIT_FAILURE;
			}
		}	
	} else { 
		perror("No message type received");
		return EXIT_FAILURE;
	}
}

uint32_t get_config(int sock_client, config_t* current_config_struct, config_t* fetched_config_struct, system_pointers_t *system_pointers) {
	
	//Block waiting for config struct
	// TODO: Do we need to call mutliple times to ensure we receive the whole thing? <-- no it's only called if the massage fits sizeof(config_t) 
	if(recv(sock_client, fetched_config_struct, sizeof(config_t), 0) > 0) {	
		// Can't guarantee checking whole struct for inequality due to padding
		// Can replace with a whole struct overwrite if required but this will depend on overhead
		//difference between conditional tests and write operations. 

		//TODO: Consider having trigger in its own branch to speed up a trigger operation 
		
		//Print all fetched config
		/*printf("\nfetched config: \n"
				"trigger: %d\n"
				"mode: %d\n"
				"CIC_divider: %d\n"
				"param_a: %d\n"
				"param_b: %d\n"
				"param_c: %d\n"
				"param_d: %d\n"
				"param_e: %d\n"
				"param_f: %d\n"
				"param_g: %d\n"
				"param_h: %d\n\n",
				fetched_config_struct->trigger,
				fetched_config_struct->mode,
				fetched_config_struct->CIC_divider,
				fetched_config_struct->param_a,
				fetched_config_struct->param_b,
				fetched_config_struct->param_c,
				fetched_config_struct->param_d,
				fetched_config_struct->param_e,
				fetched_config_struct->param_f,
				fetched_config_struct->param_g,
				fetched_config_struct->param_h);
		
		// Trigger
		if (fetched_config_struct->trigger == 0) {
			*(system_pointers->rx_rst) &= ~TRIG_MASK;
			printf("Trigger off \n\n");
		}
		current_config_struct->trigger = fetched_config_struct->trigger;
		
		// mode
		if (fetched_config_struct->mode < 4) {
			current_config_struct->mode = fetched_config_struct->mode;
		}

		// CIC Devider
		if (fetched_config_struct->CIC_divider <= 12500) {
			current_config_struct->CIC_divider = fetched_config_struct->CIC_divider;
		}

		// Parameter
		current_config_struct->param_a = fetched_config_struct->param_a;
		current_config_struct->param_b = fetched_config_struct->param_b;
		current_config_struct->param_c = fetched_config_struct->param_c;
		current_config_struct->param_d = fetched_config_struct->param_d;
		current_config_struct->param_e = fetched_config_struct->param_e;
		current_config_struct->param_f = fetched_config_struct->param_f;
		current_config_struct->param_g = fetched_config_struct->param_g;
		current_config_struct->param_h = fetched_config_struct->param_h;*/

	}	
}

uint32_t send_recording(int sock_client, int32_t bytes_to_send, system_pointers_t *system_pointers) {
	// Enable RAM writer and CIC divider, send "go" signal to GUI
	uint32_t position, limit, offset = 0;
	int buffer = 1; // set output buffer to 1

	limit = 32*1024;
	
	
	if (~(*(system_pointers->rx_fb) & 1)) { //check if first bit of rx_fb is 0 --> mesurement mode is off
		//Turn on measurement mode
		printf("Start Measurement\n");
		*(system_pointers->rx_com) |= 1; //write 1 to bit 0
	}
	//Link lnterupt to signal_handler --> brake while with STRG+C
	signal(SIGINT, signal_handler); 
	while (bytes_to_send > 0 && !interrupted) {
		//printf("is measure: %s \n", *(system_pointers->rx_fb) & 1 ? "true" : "false");
		// read ram writer position
		position = *(system_pointers->rx_cntr);
		// send 256 kB if ready, otherwise sleep 0.1 ms 
		if((limit > 0 && position > limit) || (limit == 0 && position < 32*1024)) {
			offset = limit > 0 ? 0 : 256*1024;
			limit = limit > 0 ? 0 : 32*1024;
			printf("bytes to send: %d \n", bytes_to_send);
			bytes_to_send -= send(sock_client, (system_pointers->ram) + offset, 256*1024, MSG_NOSIGNAL);			
		} else {
			usleep(100);
		}
	}
	signal(SIGINT, SIG_DFL); //reset interrupt handler to default

	//Turn off measurement mode
	printf("Stop Measurement\n");
	
	*(system_pointers->rx_com) &= ~1; //write 0 to bit 0
	
	return 1;
}

int main () {
//// Variables declaration
	int fd; //file descriptor for memoryfile
	int sock_server; //Socket for Server
	int sock_client; //Client identefire 
	int optval=1; //Number of socket options

	volatile void *cfg, *sts; //Memory pointer
	struct sockaddr_in addr; //Server address struct

	uint32_t data_size;
	int32_t bytes_to_send, message_type;

	// Initialise config structs - current and next
	config_t fetched_config, current_config = { .trigger = 0,
												.mode = 0,
												.CIC_divider = SAMPLING_DIVIDER_INIT,
					    						.dds_phase = FIXED_FREQ_INIT,
												.param_b = 0,
												.param_c = 0,
												.param_d = 0,
												.param_e = 0,
												.param_f = 0,
												.param_g = 0,
												.param_h = 0};


//// write bitstream to FPGA
	system("cat /usr/src/system_wrapper.bit > /dev/xdevcfg ");

//// Shared memory configuration
	// Open GPIO memory section
	if((fd = open("/dev/mem", O_RDWR)) < 0)	{
		perror("open");
		return EXIT_FAILURE;
	}

	// Map Status and config addresses, close memory section once mapped
	sts = mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0x40000000);
	cfg = mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0x40001000);
	close(fd);

	// Assign "system" pointers
	system_pointers_t system_regs ={.ram = 0,
									.rx_com = (uint8_t *)(cfg + 0), //system input commands 
									.rx_rate = (uint16_t *)(cfg + 2), //sample rate (Sample/Hold Config --> f=125MHz/value)
									.rx_addr = (uint32_t *)(cfg + 4), //RAM_adress --> set address of writer
									.rx_cntr = (uint32_t *)(sts + 0), //ram writer position counter
									.rx_fb = (uint16_t *)(sts + 4)}; //system feedback
	
	//Customisable parameter space
	params_t params = {.dds_phase = (uint16_t *)(cfg + 8)
					   /*.freq = (int32_t *)(cfg + 8),
					   .param_b = (int32_t *)(cfg + 12),
					   .param_c = (int32_t *)(cfg + 16),
					   .param_d = (int32_t *)(cfg + 20),
					   .param_e = (int32_t *)(cfg + 24),
					   .param_f = (int32_t *)(cfg + 28),
					   .param_g = (int32_t *)(cfg + 32),
					   .param_h = (int32_t *)(cfg + 36)*/};	

	// Open contiguous data memory section
	if((fd = open("/dev/cma", O_RDWR)) < 0)	{
		perror("open");
		return EXIT_FAILURE;
	}

	// PD's code relating to contiguous data section
	data_size = 128*sysconf(_SC_PAGESIZE);
	if(ioctl(fd, CMA_ALLOC, &data_size) < 0) {
		perror("ioctl");
		return EXIT_FAILURE;
	}
	system_regs.ram = mmap(NULL, 128*sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

	// Sets current read address at top of section
	*(system_regs.rx_addr) = data_size;

//// Socket Configuration
	//Create server socket
	if((sock_server = socket(AF_INET, SOCK_STREAM, 0)) < 0)	{
		perror("socket");
		return EXIT_FAILURE;
	}

	// Set socket options
	setsockopt(sock_server, SOL_SOCKET, SO_REUSEADDR, (void *)&optval, sizeof(optval));

	// Setup listening address 
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(TCP_PORT);

	// Bind adress to socket
	if(bind(sock_server, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("bind");
		return EXIT_FAILURE;
	}

	// Listening for incomming connections
	listen(sock_server, 1024);
	printf("Listening on port %i ...\n", TCP_PORT);


//// Main Loop
	while(!interrupted)	{
		// Stop measurement
		*(system_regs.rx_com) &= ~1; //write 0 to bit 0
		
		// Set sample rate
		*(system_regs.rx_rate) = current_config.CIC_divider;

		printf("reset complete\n");
		
		
		if((sock_client = accept(sock_server, NULL, NULL)) >= 0)	{			
			printf("sock client accepted\n");

			message_type = get_socket_type(sock_client);

			if (message_type == 0) {
				get_config(sock_client, &current_config, &fetched_config, &system_regs);

				/*if (current_config.mode == 0) {
					*(params.fixed_phase) = (uint32_t)floor(current_config.fixed_freq / 125.0e6 * (1<<30) + 0.5);
					*(system_regs.rx_rst) = (uint8_t)((*(system_regs.rx_rst) & (~MODE_MASK)) | (current_config.mode << 6));
					printf("Mode changed to %d\n", current_config.mode);
				} else if (current_config.mode == 1) {
					*(params.start_freq) = current_config.start_freq;
					*(params.stop_freq) = current_config.stop_freq;
					*(params.interval) = current_config.interval;
					*(system_regs.rx_rst) = (uint8_t)((*(system_regs.rx_rst) & (~MODE_MASK)) | (current_config.mode << 6));
					printf("Mode changed to %d\n", current_config.mode);
				} else if (current_config.mode == 2) {
					*(params.a_const) = current_config.a_const;
					*(params.b_const) = current_config.b_const;
					*(system_regs.rx_rst) = (uint8_t)((*(system_regs.rx_rst) & (~MODE_MASK)) | (current_config.mode << 6));
					printf("Mode changed to %d\n", current_config.mode);
				}
				*(params.param_a) = current_config.param_a;
				*(params.param_b) = current_config.param_b;
				*(params.param_c) = current_config.param_c;
				*(params.param_d) = current_config.param_d;
				*(params.param_e) = current_config.param_e;
				*(params.param_f) = current_config.param_f;
				*(params.param_g) = current_config.param_g;
				*(params.param_h) = current_config.param_h;
				*(system_regs.rx_rst) = (uint8_t)((*(system_regs.rx_rst) & (~MODE_MASK)) | (current_config.mode << 6));
				
				*/
				*(params.dds_phase) = 420;//current_config.dds_phase;
			}

			// Assume any other number is a number of bytes to receive
			else {
				bytes_to_send = message_type;

				if (send_recording(sock_client, bytes_to_send, &system_regs) < 1) {
					printf("send_recording error");
				}
								
			}
		}
		
		
		close(sock_client);
	}
	// Stop measurement
	*(system_regs.rx_com) &= ~1;

	close(sock_server);

	return EXIT_SUCCESS;
}
