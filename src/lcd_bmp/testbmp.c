
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#include <lib3dc.h>

int const NUM_BYTES = SPP_LCD_WIDTH * SPP_LCD_HEIGHT * 2;
unsigned short test_data[SPP_LCD_WIDTH][SPP_LCD_HEIGHT];

// Define the function to be called when ctrl-c (SIGINT) is sent to process
void signal_callback_handler(int signum)
{
  fprintf(stderr, "Caught signal %d\n", signum);
  // Terminate program
  exitLib3dc();
  exit(signum);
}

int main(int argc, char *argv[])
{
  // Register signal and signal handler
  signal(SIGINT, signal_callback_handler);

  // Set libg15 debugging
  libg15Debug(1);

  // Init LibG15
  printf("Init-ing libg15... ");
  fflush(stdout);
  int ret = initLib3dc();
  printf("%d\n", ret);

  // // No idea what this does
  // printf("Setting LED... ");
  // fflush(stdout);
  // ret = setLEDs(1);
  // printf("%d\n", ret);

  memset(test_data, 0, NUM_BYTES);

  for (int hx = 0; hx < SPP_LCD_HEIGHT; hx++)
  {
    for (int hy = 0; hy < SPP_LCD_WIDTH; hy++)
    {
      test_data[hy][hx] = hx - hy;
    }
  }

  writePixmapToLCDSPP(test_data);

  printf("Complete.\n");
  exitLib3dc();
}
