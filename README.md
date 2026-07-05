# media_list - list info about media files
This application is copyright (c) 2023  Derell Licht  
This program, and its source code, are distributed as unrestricted freeware.
You can use them for any purpose, personal or commercial, in whole or in part,
for any purpose that you wish, without contacting me further.

This console (i.e., command line) utility will list useful data about various media files.  
This replaces the /mm functionality in my ndir utility.

### File formats supported:

- image formats  
jpg, gif, bmp, png, ico, cur, ani, sid, webp, tif, tiff, avif

- audio formats  
flac, ogg, mp3, wav, wma

- video formats  
avi, flv, mkv, mov, mp4, mpg, mpeg, wmv, webm

View [sample output](http://derelllicht.com/files/MediaList.out) here

Download [executable](http://derelllicht.com/files/media_list.zip) here

<hr>

#### building the application
This application is built using the MinGW toolchain; 
I recommend the [TDM](http://tdm-gcc.tdragon.net/) distribution, 
to avoid certain issues with library accessibility. 
The makefile also requires certain Cygwin tools (rm, make, etc).

#### NOTE: this program requires my ```der_libs``` submodule
If you clone the repository without the --recursive flag, 
you can recover the submodule later, with this command:

```git submodule update --init --recursive```
<br>
