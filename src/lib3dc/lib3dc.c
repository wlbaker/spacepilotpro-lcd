/*
    This file is part of 3dctools.

    (c) 2022 William L Baker

    $Revision$ -  $Date$ $Author$
*/

#include "sys/types.h"
#include "lib3dc.h"
#include <stdio.h>
#include <stdarg.h>
#include <libusb.h>
#include <string.h>
#include <errno.h>
#include "config.h"

libusb_device_handle *keyboard_device = 0;
libusb_context *ctx = NULL;
static int lib3dc_debugging_enabled = 0;
static int enospc_slowdown = 0;

static int lib3dc_keys_endpoint = 0;     // HID --> 83
static int lib3dc_lcd_endpoint = 0;      // vendor specific LCM bulk (display) -- 02
static int lib3dc_lcdkeys_endpoint = 0;  // vendor specific LCM interrupt -- 81
				      // LED treated like endpoint=0 w SET_REPORT....and libusb_control_transfer

/* to add a new device, simply create a new DEVICE() in this list */
/* Fields are: "Name",VendorID,ProductID,Capabilities */
const lib3dc_devices_t lib3dc_devices[] = {
    DEVICE("3DConnexion Spacepilot Pro",0x46d,0xc629),
    DEVICE(NULL,0,0)
};

/* enable or disable debugging */
void libg15Debug(int level) {

    lib3dc_debugging_enabled = level;
    libusb_set_option(ctx, LIBUSB_OPTION_LOG_LEVEL, level);
}

/* debugging wrapper */
static int debug_log (FILE *fd, unsigned int level, const char *fmt, ...) {

    if (lib3dc_debugging_enabled && lib3dc_debugging_enabled>=level) {
        fprintf(fd,"lib3dc: ");
        va_list argp;
        va_start (argp, fmt);
        vfprintf(fd,fmt,argp);
        va_end (argp);
    }

    return 0;
}

/*
int hotplug_callback(struct libusb_context *ctx, struct libusb_device *dev,
                     libusb_hotplug_event event, void *user_data) {
  static libusb_device_handle *dev_handle = NULL;
  struct libusb_device_descriptor desc;
  int rc;

  (void)libusb_get_device_descriptor(dev, &desc);

  if (LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED == event) {
    rc = libusb_open(dev, &dev_handle);
    if (LIBUSB_SUCCESS != rc) {
      printf("Could not open USB device\n");
    }
  } else if (LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT == event) {
    if (dev_handle) {
      libusb_close(dev_handle);
      dev_handle = NULL;
    }
  } else {
    printf("Unhandled event %d\n", event);
  }
  count++;

  return 0;
}
libusb_hotplug_callback_handle callback_handle;
*/


libusb_device_handle * findAndOpen()
{
    // struct libusb_bus *bus = 0;
    libusb_device **devs;
    libusb_device *dev = NULL;
    int retries=0;
    int j,i,l;
    int interface=0;

    int cnt = libusb_get_device_list(NULL, &devs);
    if (cnt < 0) {
	// libusb_exit(NULL);
	return NULL;
    }

    keyboard_device = NULL;

    libusb_device_handle *handle = NULL;
    for (i = 0; devs[i] && keyboard_device == NULL; i++) {
	struct libusb_device_descriptor desc;
	unsigned char txtbuf[256];
	const char *speed;
	const lib3dc_devices_t *rr = NULL;

        dev = devs[i];

	switch (libusb_get_device_speed(dev)) {
	case LIBUSB_SPEED_LOW:		speed = "1.5M"; break;
	case LIBUSB_SPEED_FULL:		speed = "12M"; break;
	case LIBUSB_SPEED_HIGH:		speed = "480M"; break;
	case LIBUSB_SPEED_SUPER:	speed = "5G"; break;
	case LIBUSB_SPEED_SUPER_PLUS:	speed = "10G"; break;
	default:			speed = "Unknown";
	}

	int ret = libusb_get_device_descriptor(dev, &desc);
	if (ret < 0) {
		fprintf(stderr, "failed to get device descriptor");
		continue;
	}

	printf("device: %04X - %04X speed: %s\n",
	       desc.idVendor, desc.idProduct, speed);

        for (int k=0; lib3dc_devices[k].name !=NULL  ;k++) {
	    const lib3dc_devices_t *ref = &lib3dc_devices[k];

            if ((desc.idVendor == ref->vendorid && desc.idProduct == ref->productid)) {
                 rr = ref;
                 break;
            }
        }

	if( rr == NULL ) {
	     continue;
	}

        char name_buffer[65535];
        name_buffer[0] = 0;
        debug_log(stderr,LIB3DC_LOG_INFO,"Found %s, trying to open it\n",rr->name);

	ret = libusb_open(dev, &handle);

        if ( ret < 0) {
            debug_log(stderr,LIB3DC_LOG_INFO, "Error, could not open the keyboard\n");
            debug_log(stderr,LIB3DC_LOG_INFO, "Perhaps you dont have enough permissions to access it\n");
            return 0;
        }

        // usleep(50*1000);
	if (handle) {
		if (desc.iManufacturer) {
			ret = libusb_get_string_descriptor_ascii(handle, desc.iManufacturer, txtbuf, sizeof(txtbuf));
			if (ret > 0)
				printf("  Manufacturer:              %s\n", (char *)txtbuf);
		}

		if (desc.iProduct) {
			ret = libusb_get_string_descriptor_ascii(handle, desc.iProduct, txtbuf, sizeof(txtbuf));
			if (ret > 0)
				printf("  Product:                   %s\n", (char *)txtbuf);
		}

		if (desc.iSerialNumber ) {
			ret = libusb_get_string_descriptor_ascii(handle, desc.iSerialNumber, txtbuf, sizeof(txtbuf));
			if (ret > 0)
				printf("  Serial Number:             %s\n", (char *)txtbuf);
		}
	}


        debug_log(stderr, LIB3DC_LOG_INFO, "Device has %i possible configurations\n",desc.bNumConfigurations);

        /* if device is shared with another driver, such as the Z-10 speakers sharing with alsa, we have to disable some calls */

        for (j = 0; j<desc.bNumConfigurations;j++){
            struct libusb_config_descriptor *config;
	    // ret = libusb_get_config_descriptor(dev, i, &config);
	    ret = libusb_get_active_config_descriptor(dev, &config);
	    if (LIBUSB_SUCCESS != ret) {
		printf("  Couldn't retrieve descriptors: %d\n", ret);
		continue;
	    }

            debug_log(stderr, LIB3DC_LOG_INFO, "Device has %i interfaces\n",config->bNumInterfaces);
            for (int ii=0;ii<config->bNumInterfaces; ii++){

                /* if endpoints are already known, finish up */
                if(lib3dc_keys_endpoint && lib3dc_lcd_endpoint) {
                  debug_log(stderr, LIB3DC_LOG_INFO, "...gone far enough\n");
                  break;
		}
		
                const struct libusb_interface *ifp = &config->interface[ii];

                debug_log(stderr, LIB3DC_LOG_INFO, "...[%d] device has %i Alternate Settings\n", i, ifp->num_altsetting);

                for(int k=0;k<ifp->num_altsetting;k++){
                            const struct libusb_interface_descriptor *as = &ifp->altsetting[k];
                            /* verify that the interface is for a HID device */

                            for (l=0; l< as->bNumEndpoints;l++){
                                const struct libusb_endpoint_descriptor *ep=&as->endpoint[l];

                                debug_log(stderr,LIB3DC_LOG_INFO,"...CLZ: %2.2x  of INTF=%2.2x\n", as->bInterfaceClass, ep->bEndpointAddress);
                                if(as->bInterfaceClass==LIBUSB_CLASS_HID){
                                    if(0x80 & ep->bEndpointAddress) {
                                        debug_log(stderr, LIB3DC_LOG_INFO, "......Found Extra Keys endpoint %i with address 0x%X maxtransfersize=%i \n",
                                            ep->bEndpointAddress&0x0f,ep->bEndpointAddress, ep->wMaxPacketSize
                                           );
                                        lib3dc_keys_endpoint = ep->bEndpointAddress;
                                    } else {
                                        debug_log(stderr,LIB3DC_LOG_INFO,"....UNRECOGNIZED ENDPOINT %d\n", ep->bEndpointAddress );
			            }
		                } else if(as->bInterfaceClass==LIBUSB_CLASS_VENDOR_SPEC){
                                    if(0x80 & ep->bEndpointAddress) {
                                        debug_log(stderr, LIB3DC_LOG_INFO, "......Found LCDKBD endpoint with address 0x%X maxtransfersize=%i \n",
                                            ep->bEndpointAddress, ep->wMaxPacketSize );

                                        lib3dc_lcdkeys_endpoint = ep->bEndpointAddress;
			            } else {
                                        debug_log(stderr, LIB3DC_LOG_INFO, "......Found LCD endpoint with address 0x%X maxtransfersize=%i \n",
                                            ep->bEndpointAddress, ep->wMaxPacketSize );
                                        lib3dc_lcd_endpoint = ep->bEndpointAddress;
				    }
                                }
                            }

                            if (ret) {
                                debug_log(stderr, LIB3DC_LOG_INFO, "Error setting Alternate Interface\n");
                            }
                }
            }
	    libusb_free_config_descriptor(config);
        }

    }

    libusb_free_device_list(devs, 1);

    if( handle ) {
        int retval = libusb_claim_interface( handle, 0 ); // hid
        debug_log(stderr,LIB3DC_LOG_INFO,"CLAIMING INTERFACES [HID=0x%2.2x]=%d\n", 0, retval);

        // retval = libusb_claim_interface( handle, 0x1 ); // vendor
        // debug_log(stderr,LIB3DC_LOG_INFO,"CLAIMING INTERFACES [VENDOR=0x%2.2x]=%d\n", 0x1, retval);
    }
    return handle;
}




int initLib3dc()
{
    int retval = libusb_init( NULL );
    if (retval < 0)
        return retval;

  /*
  retval = libusb_hotplug_register_callback(NULL, LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED |
                                        LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT, 0, 0x045a, 0x5005,
                                        LIBUSB_HOTPLUG_MATCH_ANY, hotplug_callback, NULL,
                                        &callback_handle);
  printf("hotplug_register_callback: %d\n", retval );
  */

    debug_log(stderr,LIB3DC_LOG_INFO,"%s\n",PACKAGE_STRING);

    keyboard_device = findAndOpen();
    if (!keyboard_device)
        return -99;

    return retval;
}

/* reset the keyboard, returning it to a known state */
int exitLib3dc()
{
    int retval = LIB3DC_NO_ERROR;
    if (keyboard_device){
        retval = libusb_release_interface (keyboard_device, 0);
        keyboard_device=0;
    }
    libusb_close(keyboard_device);

    return retval;
}

int handle_libusb_errors(const char *prefix, int ret) {

    switch (ret){
        case -ENOSPC: /* the we dont have enough bandwidth, apparently.. something has to give here.. */
            debug_log(stderr,LIB3DC_LOG_INFO,"usb error: ENOSPC.. reducing speed\n");
            enospc_slowdown = 1;
            break;
        case -ETIMEDOUT:
            // break;
        case -ENODEV: /* the device went away - we probably should attempt to reattach */
        case -ENXIO: /* host controller bug */
        case -EINVAL: /* invalid request */
        case -EAGAIN: /* try again */
        case -EFBIG: /* too many frames to handle */
        case -EMSGSIZE: /* msgsize is invalid */
             debug_log(stderr,LIB3DC_LOG_INFO,"usb error: %s %s (%i)\n",prefix,libusb_strerror(ret),ret);
             break;
        case -EPIPE: /* endpoint is stalled */
             debug_log(stderr,LIB3DC_LOG_INFO,"usb error: %s EPIPE! clearing...\n",prefix);
             libusb_clear_halt(keyboard_device, 0x81);
             break;
        default: /* timed out */
             debug_log(stderr,LIB3DC_LOG_INFO,"Unknown usb error: %s !! (err is %i (%s))\n",prefix,ret,libusb_strerror(ret));
    }
    return ret;
}

int writePixmapToLCDSPP(const void *data)
{
    int ret = 0;
    int transfercount=0;
    unsigned char lcd_buffer[SPP_BUFFER_LEN];
    /* The pixmap conversion function will overwrite everything after SPP_LCD_OFFSET, so we only need to blank
       the buffer up to this point.  (Even though the keyboard only cares about bytes 0-23.) */

    // memset(lcd_buffer, 0, SPP_LCD_OFFSET);
    for (int i=0;i<SPP_LCD_OFFSET;++i)
        lcd_buffer[i] = i%256;
    memcpy( lcd_buffer + SPP_LCD_OFFSET, data, SPP_LCD_HEIGHT*SPP_LCD_WIDTH*2 );

    /* the keyboard needs this magic byte */
    lcd_buffer[ 0] = 0x10;
    lcd_buffer[ 1] = 0x0f;
    lcd_buffer[ 2] = 0x00;
    lcd_buffer[ 3] = 0x58;
    lcd_buffer[ 4] = 0x02;
    lcd_buffer[ 5] = 0x0;
    lcd_buffer[ 6] = 0x0;
    lcd_buffer[ 7] = 0x0;
    lcd_buffer[ 8] = 0x0;
    lcd_buffer[ 9] = 0x0;
    lcd_buffer[10] = 0x0;
    lcd_buffer[11] = 0x3f;
    lcd_buffer[12] = 0x01;
    lcd_buffer[13] = 0xef;
    lcd_buffer[14] = 0x0;
    lcd_buffer[15] = 0x0f;
  /* ALT: WLB in an attempt to reduce peak bus utilisation, we COULD break the transfer into chunks and sleep a bit in between.
    It shouldnt make much difference, but then again, the g15 shouldnt be flooding the bus enough to cause ENOSPC, yet
    apparently does on some machines...  I'm not sure how successful this will be in combatting ENOSPC, but we'll give it try in the real-world. */

        /* transfer entire buffer in one hit */
        debug_log(stderr,LIB3DC_LOG_WARN,"lcd_endpoint: %d len=%d\n",lib3dc_lcd_endpoint, SPP_BUFFER_LEN);
        ret = libusb_bulk_transfer(keyboard_device, lib3dc_lcd_endpoint, lcd_buffer, SPP_BUFFER_LEN, &transfercount, 1000);
        if (ret != 0)
        {
            handle_libusb_errors ("LCDPixmap Write",ret);
            return G15_ERROR_WRITING_PIXMAP;
        }

    return 0;
}

int setLCDContrast(unsigned int level)
{
    int retval = 0;
    unsigned char libusb_data[] = { 2, 32, 129, 0 };

    switch(level)
    {
        case 1:
            libusb_data[3] = 22;
            break;
        case 2:
            libusb_data[3] = 26;
            break;
        default:
            libusb_data[3] = level;
    }
    retval = libusb_control_transfer(keyboard_device, LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE, 9, 0x302, 0, (char*)libusb_data, 4, 10000);
    return retval;
}

int initXKEYs()
{
    int retval = 0;
    unsigned char m_led_buf[2] = { 0x13, 0x1a };

    retval = libusb_control_transfer(keyboard_device, LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE, 
		    LIBUSB_REQUEST_SET_CONFIGURATION, 0x313, 1, m_led_buf, 2, 10000);

    return retval;
}

int setLEDs(unsigned int on_off)
{
    int retval = 0;
    unsigned char m_led_buf[2] = { 4, 0 };
    m_led_buf[1] = (unsigned char)on_off;

    retval = libusb_control_transfer(keyboard_device, LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE, 
		    LIBUSB_REQUEST_SET_CONFIGURATION, 0x204, 1, m_led_buf, 2, 10000);
    return retval;
}

int setLCDBrightness(unsigned int bright, unsigned int cont)
{
    int retval = 0;
    unsigned char libusb_data[] = { bright,0,0,0, 0,0,0,0, cont };

    retval = libusb_control_transfer(keyboard_device, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_INTERFACE, 10, 0x0, 0, libusb_data, 9, 10000);
    return retval;
}

/* set the keyboard backlight. doesnt affect lcd backlight. 0==off,1==medium,2==high */
int setKBBrightness(unsigned int level)
{
    int retval = 0;
    unsigned char libusb_data[] = { 2, 1, 0, 0 };

    switch(level)
    {
        case 1 :
            libusb_data[2] = 0x1;
            break;
        case 2 :
            libusb_data[2] = 0x2;
            break;
        default:
            libusb_data[2] = 0x0;
    }
    retval = libusb_control_transfer(keyboard_device, LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE, 9, 0x302, 0, (char*)libusb_data, 4, 10000);
    return retval;
}

