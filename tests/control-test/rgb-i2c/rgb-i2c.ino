/** @file
 *
 * Contains the low-level hardware machinery for our basic controls test.
 *
 * 
 */
#include <Arduino.h>
#include <stdint.h>
#include <stddef.h>
#include <Wire.h>

/** These are the PWM pins to which the red, green, and blue cathodes of the RGB
 *  LED are connected.  */
uint8_t pins[] = { 11, 9, 10 };	/* r, g, b */


/** I2C slave address. */
#define I2C_ADDRESS 3


/** Values of a cos(x) for 0 < x < pi/2, scaled to eight-bit integer output.
 * The resulting full range (-255 to 255) of `cos` would require a 9-bit value,
 * hence the `cos9` tag.
 *
 * Generated in Ruby's `irb` with: l=512; o=[]; (0...l).each { |i| o << (Math.cos(i * (Math::PI/(2*(l-1)))) * 255).round }
 */
static uint8_t cos9_values[] = { 255, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 253, 253, 253, 253, 253, 253, 253, 253, 252, 252, 252, 252, 252, 252, 252, 251, 251, 251, 251, 251, 250, 250, 250, 250, 250, 249, 249, 249, 249, 249, 248, 248, 248, 248, 247, 247, 247, 247, 246, 246, 246, 245, 245, 245, 244, 244, 244, 244, 243, 243, 243, 242, 242, 242, 241, 241, 241, 240, 240, 239, 239, 239, 238, 238, 238, 237, 237, 236, 236, 236, 235, 235, 234, 234, 233, 233, 233, 232, 232, 231, 231, 230, 230, 229, 229, 228, 228, 228, 227, 227, 226, 226, 225, 225, 224, 224, 223, 223, 222, 221, 221, 220, 220, 219, 219, 218, 218, 217, 217, 216, 215, 215, 214, 214, 213, 213, 212, 211, 211, 210, 210, 209, 208, 208, 207, 207, 206, 205, 205, 204, 204, 203, 202, 202, 201, 200, 200, 199, 198, 198, 197, 197, 196, 195, 195, 194, 193, 193, 192, 191, 191, 190, 189, 188, 188, 187, 186, 186, 185, 184, 184, 183, 182, 182, 181, 180, 179, 179, 178, 177, 177, 176, 175, 174, 174, 173, 172, 171, 171, 170, 169, 168, 168, 167, 166, 166, 165, 164, 163, 163, 162, 161, 160, 159, 159, 158, 157, 156, 156, 155, 154, 153, 153, 152, 151, 150, 150, 149, 148, 147, 146, 146, 145, 144, 143, 143, 142, 141, 140, 139, 139, 138, 137, 136, 136, 135, 134, 133, 132, 132, 131, 130, 129, 129, 128, 127, 126, 125, 125, 124, 123, 122, 122, 121, 120, 119, 118, 118, 117, 116, 115, 115, 114, 113, 112, 111, 111, 110, 109, 108, 108, 107, 106, 105, 104, 104, 103, 102, 101, 101, 100, 99, 98, 98, 97, 96, 95, 95, 94, 93, 92, 91, 91, 90, 89, 88, 88, 87, 86, 86, 85, 84, 83, 83, 82, 81, 80, 80, 79, 78, 77, 77, 76, 75, 75, 74, 73, 72, 72, 71, 70, 70, 69, 68, 68, 67, 66, 66, 65, 64, 63, 63, 62, 61, 61, 60, 59, 59, 58, 57, 57, 56, 56, 55, 54, 54, 53, 52, 52, 51, 50, 50, 49, 49, 48, 47, 47, 46, 46, 45, 44, 44, 43, 43, 42, 41, 41, 40, 40, 39, 39, 38, 37, 37, 36, 36, 35, 35, 34, 34, 33, 33, 32, 31, 31, 30, 30, 29, 29, 28, 28, 27, 27, 26, 26, 26, 25, 25, 24, 24, 23, 23, 22, 22, 21, 21, 21, 20, 20, 19, 19, 18, 18, 18, 17, 17, 16, 16, 16, 15, 15, 15, 14, 14, 13, 13, 13, 12, 12, 12, 11, 11, 11, 10, 10, 10, 10, 9, 9, 9, 8, 8, 8, 7, 7, 7, 7, 6, 6, 6, 6, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

#define NUM_VALUES (sizeof(cos9_values)/sizeof(cos9_values[0]))
#define I_PI_2 NUM_VALUES
#define I_PI (2*NUM_VALUES)

/** Fetch the 9-bit `cos` value at the given integer theta value.
 *
 * @param t Theta value in "integer radians", where pi is equivalent to the
 *     constant I_PI.
 *
 * @return the integer value of `cos(t)` in the range [-255, 255].
 */
static int16_t
cos9(size_t t)
{
  t = t % (2 * I_PI);
  if ( t < I_PI_2 )
    return cos9_values[t];
  else if ( t < I_PI )
    return -cos9(I_PI - t - 1);
  else
    return cos9(I_PI - t % I_PI - 1);
}

/** Sets the output channels by hue angle.  Used in idle mode.
 */
static void
setChannelsByHue(uint16_t theta, uint8_t values[])
{
  values[0] = (uint8_t) (127 + cos9(theta) / 2);
  values[1] = (uint8_t) (127 + cos9(theta + (2 * I_PI / 3)) / 2);
  values[2] = (uint8_t) (127 + cos9(theta + (4 * I_PI / 3)) / 2);
}

/** Set the PWM output (at the pins in `pins`) to the given values.
 *
 *  Since the RGB LEDs I'm familiar with are common-anode (meaning each channel
 *  turns on when the pin connected to its cathod is _low_), we `analogWrite`
 *  with 255 _minus_ the given pin value;
 */
static void
update(uint8_t* values)
{
 /* red is really bright, so for red we divide by 4. */
  for ( uint8_t i = 0; i < sizeof(pins) / sizeof(uint8_t); ++i )
    analogWrite(pins[i], 255 - (i == 0 ? values[i] / 4 : values[i]));
}

/* Delays and stuff.  */
/** Time (in milliseconds) after receiving I2C data before we revert to cycling the color.  */
#define CYCLE_DELAY 5000

/** Duration (in milliseconds) to sleep after each LED update. */
#define UPDATE_DELAY 100

/** Amount to change the hue at each step in cycle mode. */
#define HUE_INCREMENT (I_PI/32)


/** Value of `millis()` at which we last received an I2C transmission. */
static size_t last_receive;

/** Input values. */
static uint8_t inputs[3];

/** Hue-angle counter for idle mode. */
static uint16_t theta;


/** Receive handler for I2C.  Reads multiples of three bytes, and sets the LED's
 * R/G/B channels to those values.
 *
 * @param @ignored Arduino's `Wire` library provides many ways to overspecify
 *     things.
 */
void
on_i2c_receive(int)
{
  last_receive = millis();

  for ( uint8_t i = 0; Wire.available(); ++i )
    inputs[i % (sizeof(inputs)/sizeof(inputs[0]))] = Wire.read();
}

void setup()
{
  Wire.begin(I2C_ADDRESS);
  Wire.onReceive(on_i2c_receive);

  last_receive = millis();
  inputs[0] = inputs[1] = inputs[2] = 0;
  theta = 0;

  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);

  digitalWrite(9, HIGH);
  digitalWrite(10, HIGH);
  digitalWrite(11, HIGH);
}

void
loop()
{
  if ( millis() - last_receive > CYCLE_DELAY )
    {
      setChannelsByHue(theta, inputs);
      theta += HUE_INCREMENT;
      if ( theta > 2 * I_PI )
        theta = 0;
    }

  update(inputs);

  delay(UPDATE_DELAY);
}
