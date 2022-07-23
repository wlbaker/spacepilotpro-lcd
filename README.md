# spacepilotpro-lcd

Demostration of finding the SpacePilot Pro LCD on the USB bus and sending a basice image.

# USAGE

In the "src" directory, compile lib3dc first.  This uses the standard autotools.  The script src/lib3dc/bootstrap shold help.

Next, in the "src/testlcd" directory, compile the testlcd program using "make".  No autotools here.  The output program must be run as **root** unless appropriate care is taken for permission management.

If interested, "testkeys" shows how to receive key events directly.  This should work with the ancient spacenavd utilities.  It might also server to test other code, such as a kernel driver.
