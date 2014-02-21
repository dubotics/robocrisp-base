/** @file DriveData.h
 *
 * Defines the structure that holds control information for the drive module.
 */
#ifndef DriveData_h
#define DriveData_h 1

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef enum
        {
            /** Whether to speed-control the motors (rather than power control) */
            DRIVE_DATA_PID_ENABLE = 1 << 0,

            /** Re-initialize the PID loop. This should clear accumulated error when
                switched on */
            DRIVE_DATA_PID_INIT   = 1 << 1,

            /** Trigger when the arduino should expect new values for P, I, and D from
                an external structure. */
            DRIVE_DATA_PID_ACCEPT = 1 << 2
        } DriveDataFlags;

    /** The "packed" attribute prevents GCC from adding extra bytes to the
        structure for memory-alignment purposes, which we don't want because the
        structure needs to have the same layout o nall architectures (e.g., your
        computer [probably 64-bit] and the AVR microcontrollers [8-bit]).  */
    typedef struct __attribute__ (( packed ))
    {
        int8_t flags;

        /** Left motor speed */
        int8_t left_motor;

        /** Right motor speed */
        int8_t right_motor;
    } DriveData;

#ifdef __cplusplus
}
#endif

#endif /* DriveData_h */
