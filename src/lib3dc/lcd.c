
#include <stdlib.h>
#include <libusb.h>
#include <sys/types.h>
#include <sys/stat.h>
// #include <fcntl.h>

#include "lib3dc.h"

extern unsigned char fontdata_8x8[];
extern unsigned char fontdata_7x5[];
extern unsigned char fontdata_6x4[];

void setPixel(void *buffer, unsigned int x, unsigned int y, int rgb)
{
   // x = x*3;
   // y = y*3;
   unsigned int const width = 240;
   unsigned int const height = 320;
 
   if (x >= width || y >= height)
      return;
   
   int offset = width*x + y;

   unsigned short *p = (unsigned short *)buffer;
    
   p[offset] = rgb;
   
}

int renderCharacterLarge(void *buffer, int top_left_pixel_y, int top_left_pixel_x, char character, int rgb)
{
   int helper = character * 8; // for our font which is 8x8
   
   int x, y;
   for (y=0;y<8;++y)
   {
      for (x=0;x<8;++x)
      {
         char font_entry = fontdata_8x8[helper + y];
        
         if (font_entry & 1<<(7-x))       
            setPixel(buffer,top_left_pixel_x + x, top_left_pixel_y + y, rgb);
         // else setPixel(buffer,top_left_pixel_x + x, top_left_pixel_y + y, rgb);
      }
   }
   return 8;
}

int renderCharacterMedium(void *buffer, int top_left_pixel_y, int top_left_pixel_x, char character, int rgb)
{
   int helper = character * 7 * 5; // for our font which is 6x4
   
   int x, y;
   for (y=0;y<7;++y)
   {
      for (x=0;x<5;++x)
      {
         char font_entry = fontdata_7x5[helper + y * 5 + x];
         if (font_entry)       
            setPixel(buffer,top_left_pixel_x + x, top_left_pixel_y + y, rgb);
         // else setPixel(buffer,top_left_pixel_x + x, top_left_pixel_y + y, rgb);
         
      }
   }
   return 7;
}

int renderCharacterSmall(void *buffer, int top_left_pixel_y, int top_left_pixel_x, char character, int rgb)
{
   int helper = character * 6 * 4; // for our font which is 6x4
   
   int x, y;
   for (y=0;y<6;++y)
   {
      for (x=0;x<4;++x)
      {
         char font_entry = fontdata_6x4[helper + y * 4 + x];
         if (font_entry)       
            setPixel(buffer,top_left_pixel_x + x, top_left_pixel_y + y, rgb);
         // else setPixel(buffer,top_left_pixel_x + x, top_left_pixel_y + y, rgb);
         
      }
   }
   return 6;
}

void renderText(void *buffer, const char *line, size_t len, int y, int font_size, int rgb)
{
   unsigned int col = 0;
    
      while( len-- > 0) {
         if (*line != '\r')
         {
            if (font_size == 0)
               col += renderCharacterSmall(buffer,y,col,*line, rgb);
            else if (font_size == 1)
               col += renderCharacterMedium(buffer,y,col,*line, rgb);
            else if (font_size == 2)
               col += renderCharacterLarge(buffer,y,col,*line, rgb);
         }
	 line++;
      }
    
   
}




