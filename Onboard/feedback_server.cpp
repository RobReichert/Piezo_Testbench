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
double heat_flux_2_global = 0.0;

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

// PID structure (temperature control)
typedef struct temp_PID{
    double Kp;         // Proportional coefficient
    double Ki;         // Integral coefficient
    double Kd;         // Derivative coefficient
	double pre_error;  // Previous error
    double output;  // Previous error
	double output_previous;  // Previous error
    double integral;   // Error integral
    double K2;         // Static gain of the controlled process (default 5.63)
    double room_temperature; // Room temperature (default 20 degrees)
    double safe_temperature; // Safe temperature (default 105 degrees)
} Temperature_PID_t;

// PID structure (heat flux control in cascade)
typedef struct heat_flux_PID_in_cascade{
	double Kp;         // Proportional coefficient
	double Ki;         // Integral coefficient
	double Kd;         // Derivative coefficient
	double pre_error;  // Previous error
	double output;  // Previous error
	double output_previous;  // Previous error
	double integral;   // Error integral
} Heat_Flux_PID_in_cascade_t;


// Define a structure to hold the filter state
typedef struct {
    double previous_filtered_value;
} ExponentialFilterState;


/// @brief Initialize the PID structure Temperature_PID_t
/// @param pid the pointer to the PID structure
/// @param Kp the proportional coefficient
/// @param Ki the integral coefficient
/// @param Kd the derivative coefficient
void Temp_PID_Init(Temperature_PID_t *pid, double Kp, double Ki, double Kd) {
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
	pid->pre_error = 0.0;         // the previous error is set to 0
    pid->output = 0.0;         // the previous error is set to 0
	pid->output_previous = 0.0;         // the previous error is set to 0
    pid->integral = 0.0;          // the integral is set to 0
    pid->K2 = 5.63;             // the static gain K2 is of the process (bottom plate) is 5.63 (unit: Kelvin / Watt, achieved from parameter identification) (this parameter is for prefilter to eliminate steady-state error caused by the static gain of the process)
    pid->room_temperature = 22.0; // the default room temperature is 20 degrees
    pid->safe_temperature = 105.0 + 2.0; // the default safe temperature is 100 degrees
}

/// @brief Initialize the PID structure (heat flux control in cascade)
/// @param pid the pointer to the PID structure Heat_Flux_PID_in_cascade_t
/// @param Kp the proportional coefficient
/// @param Ki the integral coefficient
/// @param Kd the derivative coefficient
void Heat_Flux_PID_Init_in_cascade(Heat_Flux_PID_in_cascade_t *pid, double Kp, double Ki, double Kd) {
	pid->Kp = Kp;
	pid->Ki = Ki;
	pid->Kd = Kd;
	pid->pre_error = 0.0;         // the previous error is set to 0
	pid->output = 0.0;         // the previous error is set to 0
	pid->output_previous = 0.0;         // the previous error is set to 0
	pid->integral = 0.0;          // the integral is set to 0
}




/// @brief Update PID calculation (temperature control) (use prefilter to eliminate steady-state error)
/// @param pid the pointer to the PID structure
/// @param setpoint the target value (unit: degree Celsius)
/// @param measured_value the measured value (unit: degree Celsius)
/// @return the control output (unit: Watt)
double Temp_PID_Update_prefilter(Temperature_PID_t *pid, double setpoint, double measured_value) 
{
    // Check if the measured value is within the safe temperature range
    if (measured_value > pid->safe_temperature) 
    {
        // If the measured value is too high, turn off the heater
        pid->output = 0.0;

        return 0;
    }


    // Adjust the setpoint temperature using prefilter to eliminate steady-state error
    // Gain of the prefilter is calculated to 1 + 1 / (Kp * K2)
    // Kp is the proportional gain of the PID controller, K2 (unit: Kelvin / Watt) is the static gain of the process (bottom plate)

    // The adjusted setpoint is : (setpoint - room_temperature) * Gain_prefilter + room_temperature:
    double setpoint_adjusted = setpoint + (setpoint - pid->room_temperature) / (pid->Kp * pid->K2);

    // Calculate error
    double error_adjusted = setpoint_adjusted - measured_value;

	double error_standard = setpoint - measured_value;

	double error = error_adjusted;

	pid->integral += error;
	

	
	printf("power of I (w):%f\n",(pid->Ki * pid->integral));
	printf("pid->integral:%f\n",pid->integral);

    // Calculate derivative of error
    double derivative = error - pid->pre_error;

    // Calculate control output
    //output = (pid->Kp * error) + (pid->Ki * pid->integral) + (pid->Kd * derivative);
	pid->output = 0.0;

	pid->output = (pid->Kp * error) + (pid->Ki * pid->integral) + (pid->Kd * derivative);


    // Update previous error
    pid->pre_error = error;

    return pid->output;
}



/// @brief Update PID calculation (temperature control)(use conditional integration to eliminate steady-state error and to avoid integral windup)
/// @param pid the pointer to the PID structure
/// @param setpoint the target value (unit: degree Celsius)
/// @param measured_value the measured temperature (unit: degree Celsius)
/// @param power_upper_limit the upper limit of the output power (unit: Watt) (for anti-windup)
/// @param power_lower_limit the lower limit of the output power (unit: Watt) (for anti-windup)
/// @param factor_for_cooling e.g. 2.0 means the integrated error will be 2*e when cooling (to cool down faster)
/// @return the control output (unit: Watt)
double Temp_PID_Update_conditional_integration(Temperature_PID_t *pid, double setpoint, double measured_value, double power_upper_limit, double power_lower_limit, double factor_for_cooling) 
{
    // Check if the measured value is within the safe temperature range
    if (measured_value > pid->safe_temperature) 
    {
        // If the measured value is too high, turn off the heater
        pid->output = 0.0;
        pid->integral += 0; // Reset integral part
        return 0;
    }

    double error = setpoint - measured_value;

	// Anti-windup
    // Conditional integration
	// if (u(kT-T) > u_max and e(kT) >= 0) or (u(kT-T) < u_min and e(kT) <= 0), then do not integrate
    if (!((pid->output_previous > power_upper_limit && error >= 0) ||
          (pid->output_previous < power_lower_limit && error <= 0)))
    {
		if(error >= 0) // heating
		{
			pid->integral += error;
		}
		else // cooling
		{
			pid->integral += factor_for_cooling * error; // to cool down faster
		}
        
    }



    // Calculate derivative of error
    double derivative = error - pid->pre_error;

    // Calculate control output
	pid->output = (pid->Kp * error) + (pid->Ki * pid->integral) + (pid->Kd * derivative);


	printf("\t\t\tpower of P (w):%f\n",(pid->Kp * error));
    printf("\t\t\tpower of I (w): %f\n", (pid->Ki * pid->integral));


	printf("\t\t\ttotal power (w): %f\n", pid->output);



    // Update previous error and output
    pid->pre_error = error;
    pid->output_previous = pid->output;

    return pid->output;
}

/// @brief Update PID calculation (heat flux control in cascade)(use conditional integration to eliminate steady-state error and to avoid integral windup)
/// @param pid the pointer to the PID structure Heat_Flux_PID_in_cascade_t
/// @param setpoint the target heat flux value (unit: Watt/m^2)
/// @param measured_heat_flux_value the measured heat flux value (unit: Watt/m^2)
/// @param temp_diff_upper_limit the upper limit of the temperature difference (T_top - T_bottom) (unit: degree Celsius) (for anti-windup and output limit)
/// @param temp_diff_lower_limit the lower limit of the temperature difference (T_top - T_bottom) (unit: degree Celsius) (for anti-windup and output limit)
/// @return the target temperature difference (T_top - T_bottom) (unit: degree Celsius)
double Heat_Flux_PID_Update_in_cascade(Heat_Flux_PID_in_cascade_t *pid, int32_t setpoint, double measured_heat_flux_value, double temp_diff_upper_limit, double temp_diff_lower_limit) 
{

	double error = setpoint - measured_heat_flux_value;

	// Anti-windup
	// Conditional integration
	// if (u(kT-T) > u_max and e(kT) >= 0) or (u(kT-T) < u_min and e(kT) <= 0), then do not integrate
	if (!((pid->output_previous > temp_diff_upper_limit && error >= 0) ||
		  (pid->output_previous < temp_diff_lower_limit && error <= 0)))
	{
		pid->integral += error;
	}

	// Calculate derivative of error
	double derivative = error - pid->pre_error;

	// Calculate control output
	pid->output = (pid->Kp * error) + (pid->Ki * pid->integral) + (pid->Kd * derivative);

	printf("\t\t\t\t\tP [k]: %f\n", (pid->Kp * error));
	printf("\t\t\t\t\tI [k]: %f\n", (pid->Ki * pid->integral));
	printf("\t\t\t\t\tD [k]: %f\n", (pid->Kd * derivative));

	// Update previous error and output
	pid->pre_error = error;
	pid->output_previous = pid->output;

	// Limit the output (temperature difference) to the range of [temp_diff_lower_limit, temp_diff_upper_limit]
	if (pid->output > temp_diff_upper_limit) 
	{
		pid->output = temp_diff_upper_limit;
	} else if (pid->output < temp_diff_lower_limit) 
	{
		pid->output = temp_diff_lower_limit;
	}

	printf("\t\t\t\t target temp diff which the inner loop should follow: %.2f\n", pid->output);

	return pid->output; // return the target temperature difference: T_top - T_bottom
}
	



/// @brief Convert heating power to PWM value
/// @param power the heating power (unit: Watt)
/// @param heaterAmount the amount of heaters (the power of each heater is 100 Watt)
/// @return the PWM value (0 - 255)
uint8_t powerToPWM(double power, uint8_t heaterAmount) {
	if (power < 0) 
	{
        // If the power is negative, turn off the heater
        return 0;
    }

    // Total power is the sum of power of all heaters
    double totalMaxPower = heaterAmount * 100.0; // 100W per heater at full PWM

    // Calculate the ratio of required power to the maximum power
    double ratio = power / totalMaxPower;

    // Ensure the ratio does not exceed 1.0
    if (ratio > 1.0) {
        ratio = 1.0;
    }

    // Map the ratio to the PWM range
    uint8_t pwm = (uint8_t)(ratio * 255);

    return pwm;
}

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

/// @brief Adjusts the PWM value (0 - 255) based on keyboard. ("+": +1, "-": -1, "8": +10, "2": -10)
/// @param pwm_value Pointer to an uint8_t integer that represents the current PWM value. This value will be modified based on the keyboard input.
/// @param system_pointers 
void change_PWM_using_keyboard(uint8_t *pwm_value, system_pointers_t* system_pointers) {
    if (_kbhit()) {
        char ch = _getch();
        switch (ch) {
            case '+':
                // If "+" is pressed, increase PWM value by 1
                if (*pwm_value < 255) {
                    (*pwm_value)++;
                    *(system_pointers->rx_PWM_DAC1) = *pwm_value;
                    *(system_pointers->rx_PWM_DAC2) = *pwm_value;
                }
                break;
            case '-':
                // If "-" is pressed, decrease PWM value by 1
                if (*pwm_value > 0) {
                    (*pwm_value)--;
                    *(system_pointers->rx_PWM_DAC1) = *pwm_value;
                    *(system_pointers->rx_PWM_DAC2) = *pwm_value;
                }
                break;
            case '8':
                // If "8" is pressed, increase PWM value by 10
                if (*pwm_value <= 245) {
                    *pwm_value += 10;
                    *(system_pointers->rx_PWM_DAC1) = *pwm_value;
                    *(system_pointers->rx_PWM_DAC2) = *pwm_value;
                }
                break;
            case '2':
                // If "2" is pressed, decrease PWM value by 10
                if (*pwm_value >= 10) {
                    *pwm_value -= 10;
                    *(system_pointers->rx_PWM_DAC1) = *pwm_value;
                    *(system_pointers->rx_PWM_DAC2) = *pwm_value;
                }
                break;
            // Additional key presses can be handled here
        }
    }
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



/// @brief Exponential filter function
/// @param state Pointer to the filter state
/// @param current_value Current value to be filtered
/// @param sampling_interval Time interval between samples
/// @param time_constant Time constant of the low-pass filter
/// @return Filtered value
double exponential_filter(ExponentialFilterState *state, double current_value, double sampling_interval, double time_constant) {
    // Calculate alpha (smoothing factor)
    double alpha = sampling_interval / (time_constant + sampling_interval);

    // Apply the exponential filter
    double filtered_value = alpha * current_value + (1 - alpha) * state->previous_filtered_value;

    // Update the previous filtered value for next use
    state->previous_filtered_value = filtered_value;

    return filtered_value;
}

//temperature controller will be executed every 10000us (0.01s)
uint32_t temp_control(params_t* params_struct, system_pointers_t* system_pointers, thermal_values_t* thermal_values_struct){

	static uint64_t last_time_us = 0; // Static variable to store the last execution time in microseconds
	uint64_t current_time_us;         // Variable to store the current time in microseconds
	uint64_t time_diff_us;            // Variable to store the time difference in microseconds

	current_time_us = _get_time_in_us(); // Get current time in microseconds
	time_diff_us = current_time_us - last_time_us; // Calculate the time difference in microseconds
	


	static uint8_t pwm_value = 0;	// Static variable to store the current PWM value
	//static uint8_t pwm_value_2 = 0; // for 1 heater, PWM 32/256 corresponds to 80 degree Celsius final stable temperature when room temperature is 20 degree Celsius


    // execute the control every 1000ms (1s)
    if(time_diff_us >= 1e6-1.5e3)
	{

		last_time_us = current_time_us;                // Update the last execution time


		// Print the time difference in 0.01 milliseconds (10 microseconds)
		printf("\t\t\tTime difference between two temp controls: %.2f ms\n", time_diff_us / 1000.0);


        
		// // control the PWM manually using keyboard
		// change_PWM_using_keyboard(&pwm_value, system_pointers);

		// // Set the PWM value of the two heaters
		// *(system_pointers->rx_PWM_DAC1) = 29;
		// *(system_pointers->rx_PWM_DAC2) = 16;

		if (params_struct->mode_T==0) // temperature control mode
		// control the temperature of 
		// setpoint 1 (int32_t param_T1, bottom plate) and setpoint 2 (int32_t param_T2, top plate)
		{
			printf("temperature control mode\n");

			// PID controller (temperature control)
			static Temperature_PID_t PID_bottom_plate; // controller for bottom plate
			static Temperature_PID_t PID_top_plate;  // controller for top plate
			static int isInitialized = 0; // Static variable to store the initialization flag

			// Check if the PID is initialized
			if (!isInitialized) 
			{
				Temp_PID_Init(&PID_bottom_plate, 5, 0.014, 0.0); // Initialize the PID controller for bottom plate
				Temp_PID_Init(&PID_top_plate, 4, 0.01, 0.0); // Initialize the PID controller for top plate

				isInitialized = 1; // Set the initialization flag
			}

			double power_bottom_plate = Temp_PID_Update_conditional_integration(
				&PID_bottom_plate, 
				(double)params_struct->param_T1, 					// target temperature
				(double)thermal_values_struct->temp1/100.0, // measured temperature (unit: degree Celsius)
				100.0, 	// upper limit of the output power for anti-windup (unit: Watt) 
				0.0, 	// lower limit of the output power for anti-windup (unit: Watt)
				2.0); 	// factor for cooling, 2.0 means the integrated error will be 2*e when cooling (to cool down faster)

			double power_top_plate = Temp_PID_Update_conditional_integration(
				&PID_top_plate, 
				(double)params_struct->param_T2, 					// target temperature
				(double)thermal_values_struct->temp2/100.0, // measured temperature (unit: degree Celsius)
				100.0, 	// upper limit of the output power for anti-windup (unit: Watt) 
				0.0, 	// lower limit of the output power for anti-windup (unit: Watt)
				2.0); 	// factor for cooling, 2.0 means the integrated error will be 2*e when cooling (to cool down faster)

			*(system_pointers->rx_PWM_DAC1) = powerToPWM(power_bottom_plate, 1);
			*(system_pointers->rx_PWM_DAC2) = powerToPWM(power_top_plate, 1);
			

			// toggle synchronization signal for FPGA PWM DAC to enter a new PWM period (every 1s)
			*(system_pointers->rx_com) ^= (1 << 3); // toggle bit 3 every 1s (PWM DAC1 change indicator)
			*(system_pointers->rx_com) ^= (1 << 4); // toggle bit 4 every 1s (PWM DAC2 change indicator)

		}

		if (params_struct->mode_T==1) // heat flux control mode (cascade)
		// control the temperature of setpoint 1 (int32_t param_T1, bottom plate)
		// and the the heat flux of the heat flux at the surface of bottom plate (int32_t param_T2, bottom plate)
		{
			printf("heat flux control mode (cascade)\n");

			static Temperature_PID_t PID_bottom_plate; // controller for bottom plate
			static Temperature_PID_t PID_top_plate;  // controller for top plate
			static Heat_Flux_PID_in_cascade_t PID_heat_flux_in_cascade; // controller for heat flux
			static int isInitialized_temp = 0; // Static variable to store the initialization flag
			static int isInitialized_heat_flux = 0; // Static variable to store the initialization flag

			// Check if the PID is initialized
			if (!isInitialized_temp) 
			{
				Temp_PID_Init(&PID_bottom_plate, 5, 0.014, 0.0); // Initialize the PID controller for bottom plate (same parameters as in the temperature control mode)
				Temp_PID_Init(&PID_top_plate, 4, 0.01, 0.0); // Initialize the PID controller for top plate (same parameters as in the temperature control mode)

				isInitialized_temp = 1; // Set the initialization flag
			}

			if (!isInitialized_heat_flux) 
			{
				Heat_Flux_PID_Init_in_cascade(&PID_heat_flux_in_cascade, 1.4/190, 0.000035, 0.0); // Initialize the PID controller (in cascade) for heat flux

				isInitialized_heat_flux = 1; // Set the initialization flag
			}

			// (parameters are same as in the temperature control mode)
			// fix the temperature of bottom plate to the user-defined setpoint
			double power_bottom_plate = Temp_PID_Update_conditional_integration(
				&PID_bottom_plate, 
				(double)params_struct->param_T1, 					// target temperature
				(double)thermal_values_struct->temp1/100.0, // measured temperature (unit: degree Celsius)
				100.0, 	// upper limit of the output power for anti-windup (unit: Watt) 
				0.0, 	// lower limit of the output power for anti-windup (unit: Watt)
				2.0); 	// factor for cooling, 2.0 means the integrated error will be 2*e when cooling (to cool down faster)

			// to check whether right parameters are passed from python GUI
			// printf("\t\t\t\t\t\t(double)params_struct->param_T1:%.2f\n",(double)params_struct->param_T1);
			// printf("\t\t\t\t\t\t(double)params_struct->param_T2:%.2f\n",(double)params_struct->param_T2);


			// this is the output of the outer loop controller, which is the target temperature difference (T_top - T_bottom). This will be the input of the inner loop controller
			// (output of heat flux controller in cascade)
			double target_temp_diff = Heat_Flux_PID_Update_in_cascade(
				&PID_heat_flux_in_cascade, 
				params_struct->param_T2, 					// target heat flux (unit: Watt/m^2)
				heat_flux_2_global, // measured heat flux (unit: Watt/m^2)

				40.0, 	// upper limit of the temperature difference for anti-windup and output limit (unit: degree Celsius) 
				0.0); 	// lower limit of the temperature difference for anti-windup and output limit (unit: degree Celsius)

			

			// this is the output of the inner loop controller, which is the power of the top plate heater
			// (parameters are same as in the temperature control mode)
			// The temperature of top plate will follow: temp of bottom_plate + target_temp_diff
			double power_top_plate = Temp_PID_Update_conditional_integration(
				&PID_top_plate, 
				(double)(thermal_values_struct->temp1/100.0 + target_temp_diff), // target temperature
				(double)thermal_values_struct->temp2/100.0, // measured temperature 2 (unit: degree Celsius)
				100.0, 	// upper limit of the output power for anti-windup (unit: Watt) 
				0.0, 	// lower limit of the output power for anti-windup (unit: Watt)
				2.0); 	// factor for cooling, 2.0 means the integrated error will be 2*e when cooling (to cool down faster)


			*(system_pointers->rx_PWM_DAC1) = powerToPWM(power_bottom_plate, 1);
			*(system_pointers->rx_PWM_DAC2) = powerToPWM(power_top_plate, 1);
			

			// toggle synchronization signal for FPGA PWM DAC to enter a new PWM period (every 1s)
			*(system_pointers->rx_com) ^= (1 << 3); // toggle bit 3 every 1s (PWM DAC1 change indicator)
			*(system_pointers->rx_com) ^= (1 << 4); // toggle bit 4 every 1s (PWM DAC2 change indicator)
		}

		


		// Perform control and print information

		//printf("\t\t\tCurrent PWM value (0-255): %u\n", pwm_value);
		printf("\n");
		printf("\t\t\tCurrent PWM 1 value (0-255): %u\n", *(system_pointers->rx_PWM_DAC1));
		printf("\t\t\tCurrent PWM 2 value (0-255): %u\n", *(system_pointers->rx_PWM_DAC2));



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
                                    .ram = 0
									}; 
	
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


	// Initialize filter states for two sensors
    ExponentialFilterState sensor1_state = {20.0}; // Initial temperature is 20 degree Celsius for exponential filter
    ExponentialFilterState sensor2_state = {20.0}; // Initial temperature is 20 degree Celsius for exponential filter

	ExponentialFilterState heat_flux_state = {0.0}; // Initial heat flux is 0 W/m^2 for exponential filter

//// Main Loop
	while(!interrupted)	{
		// Stop measurement
		*(system_regs.rx_com) &= ~1; //write 0 to bit 0

		static uint64_t last_time_us = 0; // Static variable to store the last execution time in microseconds
		uint64_t current_time_us;         // Variable to store the current time in microseconds
		uint64_t time_diff_us;            // Variable to store the time difference in microseconds

		current_time_us = _get_time_in_us(); // Get current time in microseconds

		time_diff_us = current_time_us - last_time_us; // Calculate the time difference in microseconds

		// sample every 100ms (0.1s)
		if(time_diff_us >= 1e5-3e3)
		{
			last_time_us = current_time_us;                // Update the last execution time
			//printf("time difference of sampling: %.2f ms\n", time_diff_us / 1000.0);




		


			// get the temperature values here first, otherwise the "detected temperature" will be 0 before getting in touch with python client. The "detected temperature" < target temperature, therefore the PID temperature controller will heat accidentially. This can be dangerous.

			/* usage examples for temperature sensor

			1) look up table method (data from datasheet, not from individual calibration)

			thermal_values.temp1=(int)(100*sensor.get_channel_temperature_lookup_table(0)); //get temperature from sensor *100 and convert to int
			thermal_values.temp2=(int)(100*sensor.get_channel_temperature_lookup_table(1)); //get temperature from sensor *100 and convert to int


			2) calibration method (Beta parameter equation)

			thermal_values.temp1=(int)(100 * sensor.get_channel_temp_calibration(0, 9770, 3167.23));
			thermal_values.temp2=(int)(100 * sensor.get_channel_temp_calibration(1, 9895, 3124.64));

			old calibration parameter
			thermal_values.temp1=(int)(100 * sensor.get_channel_temp_calibration(0, 9890, 3196.22));
			thermal_values.temp2=(int)(100 * sensor.get_channel_temp_calibration(1, 10010, 3225.12));
			

			3) calibration method (Steinhart-Hart equation)

			thermal_values.temp1=(int)(100 * sensor.get_channel_temp_Steinhart(1, 0.0008159, 0.0002485, 3.29766787739819E-07));
			thermal_values.temp2=(int)(100 * sensor.get_channel_temp_Steinhart(3, 0.0007564, 0.0002564, 3.06324334315379E-07));

			*/
			// read temperature sensor values
			//calibration method (Beta parameter equation)
			//thermal_values.temp1=(int)(100 * sensor.get_channel_temp_calibration(0, 9770, 3167.23));
			//thermal_values.temp2=(int)(100 * sensor.get_channel_temp_calibration(1, 9895, 3124.64));

			thermal_values.temp1 = (int)(100*exponential_filter(
				&sensor1_state,  // pointer which stores the previous filtered value

				// read temperature in degree Celsius
				sensor.get_channel_temp_calibration(0,  		// channel number
													9770,  		// R0
													3167.23),  	// Beta parameter
				0.1, 			// sampling interval (s)
				0.1));			// time constant of the lowpass filter (s)

				

			thermal_values.temp2 = (int)(100*exponential_filter(
				&sensor2_state,  // pointer which stores the previous filtered value

				// read temperature in degree Celsius
				sensor.get_channel_temp_calibration(1,  		// channel number
													9895,  		// R0
													3124.64),  	// Beta parameter
				0.1, 			// sampling interval (s)
				0.1));			// time constant of the lowpass filter (s)

			thermal_values.temp3=666;
			thermal_values.temp4=5;
			thermal_values.temp5=10;
			thermal_values.temp6=20;
			thermal_values.flow1=1000;

			// this value will be sent to PC client in Python GUI
			// but this value is not used for the control, because the range here is limitted to -32768 to 32767 W/ m^2, because the type of flow 2 is int16_t.

			// // !!! notice that in this case, the value range of heat flux sensor (including the value transferd to PC client in Python GUI) is limitted to -32768 to 32767 W/ m^2, because the type of flow 2 is int16_t.

			// thermal_values.flow2=32768 + (int16_t)(1*sensor.get_channel_heat_flux(
			// 	7,												// channel number
			// 	(float)thermal_values.temp1/100.0,	// temperature of the heat flux sensor (unit: degree Celsius)
			// 	11.9,								// the gain of the amplifier (unit: V/V)									
			// 	2.42));								// the output voltage of the heat flux sensor when the heat flux is 0 (W/m^2) (unit: V)


			// thermal_values.flow2 = 32768 + (int16_t)(exponential_filter(
			// 	&heat_flux_state,  // pointer which stores the previous filtered value

			// 	// read heat flux in W/m^2
			// 	sensor.get_channel_heat_flux(7,												// channel number
			// 									(float)thermal_values.temp1/100.0,	// temperature of the heat flux sensor (unit: degree Celsius)
			// 									11.9,								// the gain of the amplifier (unit: V/V)									
			// 									2.42),								// the output voltage of the heat flux sensor when the heat flux is 0 (W/m^2) (unit: V)
			// 	0.1, 			// sampling interval (s)
			// 	0.1));			// time constant of the lowpass filter (s)
			





		// printf("flow2-32768: %d\n", thermal_values.flow2-32768);




		}

		
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
				
				// The following temperature detection has been moved to a few lines above (to get the temperature values before getting in touch with python client)
				//thermal_values.temp1++;
				//thermal_values.temp2=555; //--> counted up in temp_control for testing
				// thermal_values.temp3=666;
				// thermal_values.temp4=5;
				// thermal_values.temp5=10;
				// thermal_values.temp6=20;
				// thermal_values.flow1=1000;
				// thermal_values.flow2=0;

				// // for sensor calibration (get the resistance of each channel)
				// float R0 = sensor.get_channel_R(0);
				// printf("R0: %.2f\n", R0);
				// float R1 = sensor.get_channel_R(1);
				// printf("R1: %.2f\n", R1);
				// float R2 = sensor.get_channel_R(2);
				// printf("R2: %.2f\n", R2);
				// float R3 = sensor.get_channel_R(3);
				// printf("R3: %.2f\n", R3);
				// float R4 = sensor.get_channel_R(4);
				// printf("R4: %.2f\n", R4);
				// float R5 = sensor.get_channel_R(5);
				// printf("R5: %.2f\n", R5);

				//printf("test the execution frequency.\n");

				// // read voltage on channel 7
				// float voltage_7 = sensor.get_channel_voltage(7);

				// printf("voltage on channel 7: %.2f\n", voltage_7);

				// !!! notice that in this case, the value range of heat flux sensor (including the value transferd to PC client in Python GUI) is limitted to -32768 to 32767 W/ m^2, because the type of flow 2 is int16_t.

				float heat_flux_origin_in_float = sensor.get_channel_heat_flux(
					7,												// channel number
					(float)thermal_values.temp1/100.0,	// temperature of the heat flux sensor (unit: degree Celsius)
					11.9,								// the gain of the amplifier (unit: V/V)									
					2.42);								// the output voltage of the heat flux sensor when the heat flux is 0 (W/m^2) (unit: V)

				printf("\t\t\theat_flux_origin_in_float: %.2f W/ m^2\t", heat_flux_origin_in_float);
				printf("\theat_flux_2_global:%.2f\n",heat_flux_2_global);


				// this value (heat_flux_2_global) will be used for the control
				// because thermal_values.flow2=32768 + (int16_t)heat_flux_origin_in_float;
				// is difficult to be used for the control
				heat_flux_2_global = heat_flux_origin_in_float;
				
				
				// this value will be sent to PC client in Python GUI
				// +32768 is for Python GUI
				thermal_values.flow2=32768 + (int16_t)heat_flux_origin_in_float;
				

				// // to check whether there is problem with the type conversion
				// printf("\t\t(double)(thermal_values.flow2+32768)=%.2f\n",(double)(thermal_values.flow2+32768));
				// this method only works for positive values

				


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
