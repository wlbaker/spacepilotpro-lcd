
#ifndef _LIBG15_H_
#define _LIBG15_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define SPP_LCD 1
#define G15_KEYS 2

#define SPP_KEY_READ_LENGTH 64

struct lib3dc_devices_t {
  const char *name;	// name - can be anything, only used for informational purposes
  unsigned int vendorid;
  unsigned int productid;
};

typedef struct lib3dc_devices_t lib3dc_devices_t;

#define DEVICE(name, vendorid, productid) { \
    name, \
    vendorid, \
    productid \
}

  /* allow for api changes */
#define LIBG15_VERSION 1201

  enum 
  {
    LIB3DC_LOG_INFO = 1,
    LIB3DC_LOG_WARN
  };
  
  enum
  {
    LIB3DC_NO_ERROR = 0,
    G15_ERROR_OPENING_USB_DEVICE,
    G15_ERROR_WRITING_PIXMAP,
    G15_ERROR_TIMEOUT,
    G15_ERROR_READING_USB_DEVICE,
    G15_ERROR_TRY_AGAIN,
    G15_ERROR_WRITING_BUFFER,
    G15_ERROR_UNSUPPORTED
  };
  
  enum
  {
    SPP_LCD_OFFSET = 512,
    SPP_LCD_HEIGHT = 240,
    SPP_LCD_WIDTH = 320
  };

  enum
  {
    SPP_BUFFER_LEN = SPP_LCD_HEIGHT*SPP_LCD_WIDTH*2 + SPP_LCD_OFFSET
  };
  
  enum
  {
    G15_LED_M1=1<<0,
    G15_LED_M2=1<<1,
    G15_LED_M3=1<<2,
    G15_LED_MR=1<<3
  };
  
  enum
  {
    LIB3DC_CONTRAST_LOW=0,
    LIB3DC_CONTRAST_MEDIUM,
    LIB3DC_CONTRAST_HIGH    
  };
  

  /* this one return G15_NO_ERROR on success, something
   * else otherwise (for instance G15_ERROR_OPENING_USB_DEVICE */
  int initLib3dc();

  int exitLib3dc();
  /* enable or disable debugging */
  void libg15Debug(int option);
  
  int writePixmapToLCDSPP(const void *data);
  int setLCDContrast(unsigned int level);
  int setLEDs(unsigned int leds);
  int setLCDBrightness(unsigned int bright, unsigned int cont);
  int setKBBrightness(unsigned int level);  

  void renderText(void *buffer, const char *line, size_t len, int y, int font_size, int rgb);

#ifdef __cplusplus
}
#endif
	
#endif
