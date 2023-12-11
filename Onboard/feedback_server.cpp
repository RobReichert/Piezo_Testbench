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

#include <termios.h> // for keyboard detection
#include <time.h>
#include <sys/time.h> // for time difference calculation
#include "sensor.h" // to read data from ADC128D818 (c++ code)


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
	uint16_t load_mode;
	int32_t param_L1;
	int32_t param_L2;
	int32_t param_L3;
	int32_t param_L4;
	uint16_t sample_mode;
	int32_t param_S1;
	int32_t param_S2;
	int32_t param_S3;
	int32_t param_S4;
	uint16_t temp_mode;
	int32_t param_T1;
	int32_t param_T2;
	uint16_t param_P;
	uint16_t param_I;
	uint16_t param_D;
	int32_t param_rate;
	uint16_t LI_amp_mode;
	int32_t dds_phase;
	uint16_t measure;
} config_t;

typedef struct system_pointers {
	volatile uint8_t *rx_com;
	volatile uint16_t *rx_rate;
	volatile uint32_t *rx_addr;
	volatile uint32_t *rx_cntr;
	volatile uint16_t *rx_fb;
	volatile uint8_t *rx_PWM_DAC1;
	volatile uint8_t *rx_PWM_DAC2;
	volatile uint8_t *rx_PWM_DAC3;
	volatile uint8_t *rx_PWM_DAC4;
	void *ram;
	volatile uint8_t *rx_PWM_DAC_change_indicator; // to let FPGA know PWM DAC value has changed
} system_pointers_t;

typedef struct parameters {
	volatile uint32_t *dds_phase;
	volatile uint8_t *mode;
	volatile int32_t *param_L1;
	volatile int32_t *param_L2;
	volatile int32_t *param_L3;
	volatile int32_t *param_L4;
	volatile int32_t *param_S1;
	volatile int32_t *param_S2;
	volatile int32_t *param_S3;
	volatile int32_t *param_S4;
	volatile int16_t *param_P;
	volatile int16_t *param_I;
	volatile int16_t *param_D;
	uint16_t mode_T;
	int32_t param_T1;
	int32_t param_T2;
} params_t;

typedef struct thermal_values {
	int16_t temp1;
	int16_t temp2;
	int16_t temp3;
	int16_t temp4;
	int16_t temp5;
	int16_t temp6;
	int16_t flow1;
	int16_t flow2;
} thermal_values_t;


// Check if a keyboard key is pressed (for linux)
int _kbhit(void) {
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt); // Get the current terminal I/O settings
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO); // Set terminal to non-canonical, no echo mode
    tcsetattr(STDIN_FILENO, TCSANOW, &newt); // Apply new settings
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0); // Get current file status flags
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK); // Set non-blocking mode

    ch = getchar(); // Try to read a character

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // Restore old settings
    fcntl(STDIN_FILENO, F_SETFL, oldf); // Restore file status flags

    if(ch != EOF) {
        ungetc(ch, stdin); // Put the character back if it was read
        return 1;
    }

    return 0;
}

// Get a keyboard input character (for linux)
char _getch(void) {
    char ch;
    struct termios oldt;
    struct termios newt;

    tcgetattr(STDIN_FILENO, &oldt); // Get the current terminal I/O settings
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO); // Set terminal to non-canonical, no echo mode
    tcsetattr(STDIN_FILENO, TCSANOW, &newt); // Apply new settings

    ch = getchar(); // Read a character

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // Restore old settings

    return ch;
}

// Get the current time in microseconds
uint64_t _get_time_in_us() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000ULL + tv.tv_usec;
}

void signal_handler(int sig) {
	interrupted = 1;
}

//temperature controller will be executed every 10000us (0.01s)
uint32_t temp_control(params_t* params_struct, system_pointers_t* system_pointers, thermal_values_t* thermal_values_struct){
	static time_t last_time = 0; // Static variable to store the time of the last execution
    time_t current_time;		 // Variable to store the current time
    double time_diff;			 // Variable to store the time difference since the last execution

	time(&current_time); // Get current time
    time_diff = difftime(current_time, last_time); // Calculate the time difference
/////////////// to show the time difference in 0.01ms
	static uint64_t last_time_us = 0; // Static variable to store the last execution time in microseconds
	uint64_t current_time_us;         // Variable to store the current time in microseconds
	uint64_t time_diff_us;            // Variable to store the time difference in microseconds

	current_time_us = _get_time_in_us(); // Get current time in microseconds
/////////////////////////////////////////////

	//static uint8_t pwm_value = 0;	// Static variable to store the current PWM value
	static uint8_t pwm_value = 32; // for 1 heater, PWM 32/256 corresponds to 80 degree Celsius final stable temperature when room temperature is 20 degree Celsius


    // Check if 1 second has passed
    if(time_diff >= 1){

/////////////// to show the time difference in 0.01ms
		time_diff_us = current_time_us - last_time_us; // Calculate the time difference in microseconds
		last_time_us = current_time_us;                // Update the last execution time

		// Print the time difference in 0.01 milliseconds (10 microseconds)
		printf("Time difference: %.2f ms\n", time_diff_us / 1000.0);
/////////////////////////////////////////////

        // Update the last execution time
        last_time = current_time;

		if (_kbhit()) {
            char ch = _getch();
            if (ch == '+') {	//if "+" is pressed increase PWM value by 1
                if (pwm_value < 255) {
                    pwm_value++;
					*(system_pointers->rx_PWM_DAC1) = pwm_value;
                }

            } else if (ch == '-') {	//if "-" is pressed decrease PWM value by 1
                if (pwm_value > 0) {
                    pwm_value--;
					*(system_pointers->rx_PWM_DAC1) = pwm_value;
                }

            }
			else if (ch == '8') { //if "8" is pressed increase PWM value by 10
				if (pwm_value <= 245) {
                    pwm_value += 10;
					*(system_pointers->rx_PWM_DAC1) = pwm_value;
                }
			}
			else if (ch == '2') {	//if "2" is pressed decrease PWM value by 10
                if (pwm_value >= 10) {
                    pwm_value -= 10;
					*(system_pointers->rx_PWM_DAC1) = pwm_value;
                }
			}
			
        }


		*(system_pointers->rx_com) ^= (1 << 3); // toggle bit 3 every 1s (PWM DAC1 change indicator)

		// Perform control and print information
        printf("Control performed at %s", ctime(&current_time));
		printf("Current PWM value (0-255): %u\n", pwm_value);

		//control code goes here...





		// //every thing in heare is for testing only and can be deleted if temp control is implemented
		// //Print UI input
		// if (params_struct->mode_T==3){
		// 	printf("temp_mode: %d\n"
		// 			"param_T1: %d\n"
		// 			"param_T2: %d\n",
		// 			params_struct->mode_T,
		// 			params_struct->param_T1,
		// 			params_struct->param_T2);
		// }
		// //Set some PWM Values
		// *(system_pointers->rx_PWM_DAC1)=128; //50% PWM
		// //count temp2 up
		// (thermal_values_struct->temp2)++;
	}
	//printf("the code out ouf the 1-s area is executed every 10000us (0.01s)\n");


}

// Receive message "header" bytes. 
// Return acknowledge of config send if header ==0
// Return acknowledge of thermal value request if header ==1
// otherwise echo back bytes_to_send to confirm a recording request
uint32_t get_socket_type(int sock_client)
{
	int32_t message = 0;
	uint32_t temp_ack = 1;
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
		} else if (message == 1) {
			if (send(sock_client, &temp_ack, sizeof(temp_ack), MSG_NOSIGNAL) == sizeof(temp_ack)) {
				return message;
			} else {
				perror("Message ack send failed");
				return EXIT_FAILURE;
			}
		}
		else {
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

//Get config from UI and write it to params_struct
uint32_t get_config(int sock_client, config_t* config_struct, params_t* params_struct, system_pointers_t *system_pointers) 
{
	//Block waiting for config struct
	int wait_count=0;
	while(wait_count<=1000){
		wait_count++;
		printf("error count: %d\n", wait_count);
		if(recv(sock_client, config_struct, sizeof(config_t), 0) > 0) {	
			//Print all fetched config
			printf("\nfetched config: \n"
					"load_mode: %d\n"
					"param_L1: %d\n"
					"param_L2: %d\n"
					"param_L3: %d\n"
					"param_L4: %d\n"
					"sample_mode: %d\n"
					"param_S1: %d\n"
					"param_S2: %d\n"
					"param_S3: %d\n"
					"param_S4: %d\n"
					"temp_mode: %d\n"
					"param_T1: %d\n"
					"param_T2: %d\n"
					"param_P: %d\n"
					"param_I: %d\n"
					"param_D: %d\n"
					"param_rate: %d\n"
					"LI_amp_mode: %d\n"
					"dds_phase: %d\n"
					"measure: %d\n\n",
					config_struct->load_mode,
					config_struct->param_L1,
					config_struct->param_L2,
					config_struct->param_L3,
					config_struct->param_L4,
					config_struct->sample_mode,
					config_struct->param_S1,
					config_struct->param_S2,
					config_struct->param_S3,
					config_struct->param_S4,
					config_struct->temp_mode,
					config_struct->param_T1,
					config_struct->param_T2,
					config_struct->param_P,
					config_struct->param_I,
					config_struct->param_D,
					config_struct->param_rate,
					config_struct->LI_amp_mode,
					config_struct->dds_phase,
					config_struct->measure);

			//set values
			//system input commands
			*(params_struct->mode) = config_struct->measure;
			*(params_struct->mode) |= config_struct->LI_amp_mode<<1;
			//DDS
			*(params_struct->dds_phase) = config_struct->dds_phase;
			//Measurement modes
			*(params_struct->mode) = config_struct->load_mode;
			*(params_struct->mode) |= config_struct->sample_mode<<4;
			//Test Parameters
			*(params_struct->param_L1) = config_struct->param_L1;
			*(params_struct->param_L2) = config_struct->param_L2;
			*(params_struct->param_L3) = config_struct->param_L3;
			*(params_struct->param_L4) = config_struct->param_L4;
			*(params_struct->param_S1) = config_struct->param_S1;
			*(params_struct->param_S2) = config_struct->param_S2;
			*(params_struct->param_S3) = config_struct->param_S3;
			*(params_struct->param_S4) = config_struct->param_S4;
			//Controller Parameters
			*(params_struct->param_P) = config_struct->param_P;
			*(params_struct->param_I) = config_struct->param_I;
			*(params_struct->param_D) = config_struct->param_D;
			//Temperature Controll Parameters
			params_struct->mode_T = config_struct->temp_mode;
			params_struct->param_T1 = config_struct->param_T1;
			params_struct->param_T2 = config_struct->param_T2;
			return 0;
		}
		usleep(100);
	}
	perror("Message get failed");
	return EXIT_FAILURE;
}

//Try to send the recorded data if possible, otherwise run a temperature controller cycle and wait 10000us
uint32_t send_recording(int sock_client, int32_t bytes_to_send, params_t* params_struct, system_pointers_t *system_pointers, thermal_values_t* thermal_values_struct) {
	// Enable RAM writer and CIC divider, send "go" signal to GUI
	uint32_t position, offset = 0;
	uint32_t limit = 32*1024;
	int buffer = 1; // set output buffer to 1
	
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
			// if nothing can be send do a temperatur controller cycle and wait for 10000us
			temp_control(params_struct, system_pointers, thermal_values_struct);
			usleep(10000);
		}
	}
	signal(SIGINT, SIG_DFL); //reset interrupt handler to default

	//Turn off measurement mode
	printf("Stop Measurement\n");
	
	*(system_pointers->rx_com) &= ~1; //write 0 to bit 0
	
	return 1;
}

//Send thermal sensor values to UI
uint32_t send_thermal(int sock_client, thermal_values_t* thermal_values_struct) {
	if (send(sock_client, thermal_values_struct, sizeof(thermal_values_t), MSG_NOSIGNAL) == sizeof(thermal_values_t)) {
        return 1;
    } else {
        perror("Thermal value send failed");
        return EXIT_FAILURE;
    }
    return 0;
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


	Sensor sensor; // initialise sensor and ADC

	// Initialise config struct
	config_t config = { .load_mode = 0,
						.param_L1 = 0,
						.param_L2 = 0,
						.param_L3 = 0,
						.param_L4 = 0,
						.sample_mode = 0,
						.param_S1 = 0,
						.param_S2 = 0,
						.param_S3 = 0,
						.param_S4 = 0,
						.temp_mode = 0,
						.param_T1 = 20,
						.param_T2 = 20,
                        .param_P = 0,
                        .param_I = 0,
                        .param_D = 0,
						.param_rate = SAMPLING_DIVIDER_INIT,
						.LI_amp_mode = 0,
						.dds_phase = FIXED_FREQ_INIT,
						.measure = 0};

	// Initialise thermal value struct
	thermal_values_t thermal_values = { .temp1=0,
										.temp2=0,
										.temp3=0,
										.temp4=0,
										.temp5=0,
										.temp6=0,
										.flow1=0,
										.flow2=0};

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
	system_pointers_t system_regs ={.rx_com = (uint8_t *)(cfg + 0), //system input commands 
									.rx_rate = (uint16_t *)(cfg + 2), //sample rate (Sample/Hold Config --> f=125MHz/value)
									.rx_addr = (uint32_t *)(cfg + 4), //RAM_adress --> set address of writer
									.rx_cntr = (uint32_t *)(sts + 0), //ram writer position counter
									.rx_fb = (uint16_t *)(sts + 4), //system feedback
									.rx_PWM_DAC1 = (uint8_t *)(cfg + 12), //PWM DAC1 Value 
									.rx_PWM_DAC2 = (uint8_t *)(cfg + 13), //PWM DAC2 Value
									.rx_PWM_DAC3 = (uint8_t *)(cfg + 14), //PWM DAC3 Value
									.rx_PWM_DAC4 = (uint8_t *)(cfg + 15), //PWM DAC4 Value
                                    .ram = 0,
									.rx_PWM_DAC_change_indicator = (uint8_t *)(cfg + 54)}; 
	
	//Customisable parameter space
	params_t params = {	.dds_phase = (uint32_t *)(cfg + 8), //DDS phase (phase=f/125MHz*2^30-1)
						.mode = (uint8_t *)(cfg + 1), //measurement modes
						.param_L1 = (int32_t *)(cfg + 16),
						.param_L2 = (int32_t *)(cfg + 20),
						.param_L3 = (int32_t *)(cfg + 24),
						.param_L4 = (int32_t *)(cfg + 28),
						.param_S1 = (int32_t *)(cfg + 32),
						.param_S2 = (int32_t *)(cfg + 36),
						.param_S3 = (int32_t *)(cfg + 40),
						.param_S4 = (int32_t *)(cfg + 44),
						.param_P = (int16_t *)(cfg + 48),
						.param_I = (int16_t *)(cfg + 50),
						.param_D = (int16_t *)(cfg + 52),
						.mode_T = 0,
						.param_T1 = 20,
						.param_T2 = 20};

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

	// Set the socket to non-blocking mode
	int flags = fcntl(sock_server, F_GETFL, 0);
	fcntl(sock_server, F_SETFL, flags | O_NONBLOCK);

	// Listening for incomming connections
	listen(sock_server, 1024);
	printf("Listening on port %i ...\n", TCP_PORT);

	//initial config for DDS and Sample devider
	*(params.dds_phase) = config.dds_phase;
	*(system_regs.rx_rate) = config.param_rate;

//// Main Loop
	while(!interrupted)	{
		// Stop measurement
		*(system_regs.rx_com) &= ~1; //write 0 to bit 0
		
		if((sock_client = accept(sock_server, NULL, NULL)) >= 0)	{			
			printf("sock client accepted\n");

			message_type = get_socket_type(sock_client);
			//printf("Request message: %d\n", message_type);
			if (message_type == 0) {
				printf("get_config\n");
				get_config(sock_client, &config, &params, &system_regs);
			}
			else if (message_type == 1) {
				printf("send thermal data\n");
				if (send_thermal(sock_client, &thermal_values) < 1) {
					printf("send_recording error");
				}
				// set some values for testing
				//thermal_values.temp1=(int)(100*sensor.get_channel_temperature(0)); //get temperature from sensor *100 and convert to int
				thermal_values.temp1=(int)(100 * sensor.get_channel_temp_calibration(0, 10240, 3210.0));
				//thermal_values.temp1++;
				//thermal_values.temp2=555; //--> counted up in temp_control for testing
				thermal_values.temp2=(int)(100 * sensor.get_channel_temp_calibration(1, 10080, 3219.745));
				thermal_values.temp3=666;
				thermal_values.temp4=5;
				thermal_values.temp5=10;
				thermal_values.temp6=20;
				thermal_values.flow1=1000;
				thermal_values.flow2=0;
			}
			// Assume any other number is a number of bytes to receive
			else {
				bytes_to_send = message_type;

				if (send_recording(sock_client, bytes_to_send, &params, &system_regs, &thermal_values) < 1) {
					printf("send_recording error");
				}
								
			}
			close(sock_client);
		}else{
			// if no thing to do, do a temperatur controller cycle and wait for 10000us
			temp_control(&params, &system_regs, &thermal_values);
			//printf("temp_control just executed\n");
			usleep(10000);
		}
	}
	// Stop measurement
	*(system_regs.rx_com) &= ~1;
	close(sock_server);
	return EXIT_SUCCESS;
}
