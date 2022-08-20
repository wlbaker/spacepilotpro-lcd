
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

  libg15Debug(1);

  printf("Init-ing libg15... ");
  fflush(stdout);
  int ret = initLib3dc();
  printf("%d\n", ret);

  printf("Setting LED... ");
  fflush(stdout);
  ret = setLEDs(1);
  printf("%d\n", ret);

  /*
  printf( "Setting LCD Brightness" );
  fflush(stdout);
  for( unsigned int i = 0; i <= 100; i+=20) {
          ret = setLCDBrightness(i,16);
          printf("...%d" , i );
          fflush(stdout);
          sleep(1000);
  }
  */

  printf("Sending images...\n");
  for (int kk = 0; kk < 8; kk++)
  {
    char *txt = NULL;

    memset(test_data, 0, NUM_BYTES);
    if (kk % 8 == 0)
    {
      for (int i = 0; i < SPP_LCD_HEIGHT; i++)
        test_data[i][i] = 0xf0f0;
      renderText(test_data, txt = "HeLLo", 5, 50, 2, 0xfff0);
    }
    else if (kk % 8 == 1)
    {
      for (int i = 0; i < SPP_LCD_HEIGHT; i++)
        test_data[i][SPP_LCD_HEIGHT - i - 1] = 0x0f00;
      renderText(test_data, txt = "hello", 5, 50, 2, 0xfff0);
    }
    else if (kk % 8 == 2)
    {
      for (int i = 0; i < 100; i++)
        test_data[80][i] = 0xf000;
      renderText(test_data, txt = "world", 5, 50, 2, 0xfff0);
    }
    else if (kk % 8 == 3)
    {
      for (int i = 0; i < 100; i++)
        test_data[i][SPP_LCD_HEIGHT - i - 1] = 0xf000;
      renderText(test_data, txt = "HELLO WORLD", 11, 50, 2, 0xff00);
    }
    else if (kk % 8 == 4)
    {
      for (int i = 0; i < SPP_LCD_WIDTH * SPP_LCD_HEIGHT; i++)
      {
        ((unsigned short *)test_data)[i] = 0xf000;
      }
      txt = "[red]";
    }
    else if (kk % 8 == 5)
    {
      for (int i = 0; i < SPP_LCD_WIDTH * SPP_LCD_HEIGHT; i++)
      {
        ((unsigned short *)test_data)[i] = 0x0f00;
      }
      txt = "[green]";
    }
    else if (kk % 8 == 6)
    {
      for (int i = 0; i < SPP_LCD_WIDTH * SPP_LCD_HEIGHT; i++)
      {
        ((unsigned short *)test_data)[i] = 0x00f0;
      }
      txt = "[blue]";
    }
    else if (kk % 8 == 7)
    {
      renderText(test_data, txt = "Done", 4, 50, 0, 0xfff0);
      renderText(test_data, txt = "Done", 4, 100, 1, 0xff00);
      renderText(test_data, txt = "Done", 4, 150, 2, 0xf000);
    }
    printf("* image w text: [%s]\n", txt);

    writePixmapToLCDSPP(test_data);
    sleep(2);
  }

  printf("Complete.");
  exitLib3dc();
}
