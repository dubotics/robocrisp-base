/** @file ChassisData.h
 *
 * Defines the structure that holds control information for the chassis.
 */
#ifndef ChassisData.h
#define ChassisData_h 1

#ifdef __cplusplus
extern "C" {
#endif

  /* The "packed" attribute prevents GCC from adding extra bytes to the
     structure for memory-alignment purposes, which we don't want because the
     structure needs to have the same layout o nall architectures (e.g., your
     computer [probably 64-bit] and the AVR microcontrollers [8-bit]).  */
  typedef struct __attribute__ (( __packed__ ))
  {
	/*	Whether to speed-control the motors (rather than power control) */
	unsigned char pid_enabled : 2;

	/*	Re-initialize the PID loop. This should clear accumulated error when
		switched on */
	unsigned char pid_init : 2;

	/*	Trigger when the arduino should expect new values for P, I, and D from
		an external structure. */
	unsigned char pid_accept_values : 2;

	/*	Left motor speed */
	unsigned char left_motor : 5;

	/*	Right motor speed */
	unsigned char right_motor : 5;

  } ChassisData;

#ifdef __cplusplus
}
#endif

#endif	/* ChassisData_h */
