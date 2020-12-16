# uDMX.dll - the one and only

This library is used by some dmx programs to connect to the uDMX DMX device.

The dll will be loaded directly by the software or indirectly by programm-specific plugins or programs like:

- Freestyler loads direct
- uDMXPlug.out.dll for DMXControl
- uDMX_PcDimm.dll  for PC-Dimmer
- ...

## History
When i started working with DMX, my first device was a device with serial input.  
Soon there was nearly no hardware (PC / notebook) with serial connectors any more and usb came up.  
  
When i found [uDMX](https://www.anyma.ch/research/udmx/) in the early 2000th there was only a driver for MacOS.  
  
As i wanted to try uDMX on Windows and Linux i wrote the first uDMX plugin for [QLC / QLC+](https://www.qlcplus.org/).
  
Soon i wanted to try more dmx software and released the uDMX.dll that is implemented in some programs as described above.  

In the meantime i use more and more LED matrices. Then the [uDMX timing](https://illutzmination.de/udmx-timing.html) is a problem when changing all channels at a time.  

Event at home i use 9 universes at the moment most of the with [ESPixelStick](https://github.com/forkineye/ESPixelStick).  

Consequently i use a `network to DMX` device based on ESP8266 in the meantime. Have a look at [ESP8266_Artnet_DMX_DC](https://github.com/Lutzion/ESP8266_Artnet_DMX_DC).  
  
And if you still use uDMX, i hope this code is useful for you :-)  
  

## Compiling
You need libusb-win32 to compile.  
As far as i remember i uses Visual Studio 6.0 (version 1998) as compiler.  


