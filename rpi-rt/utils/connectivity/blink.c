/**
install library libgpiod-dev
*/
#include <gpiod.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv)
{
  const char *chipname = "gpiochip0";
  struct gpiod_chip *chip;
  struct gpiod_line *lineRed;    // Red LED
  int i;

  // Open GPIO chip
  chip = gpiod_chip_open_by_name(chipname);

  // Open GPIO lines
  lineRed = gpiod_chip_get_line(chip, 21); // check gpioinfo

  // Open LED lines for output
  gpiod_line_request_output(lineRed, "example1", 0);


  // Blink LEDs in a binary pattern
  i = 0;
  while (true && i<10000000) {
    gpiod_line_set_value(lineRed, (i & 1) != 0);
    usleep(100000);
    i++;
  }

  // Release lines and chip
  gpiod_line_release(lineRed);
  gpiod_chip_close(chip);
  return 0;
}