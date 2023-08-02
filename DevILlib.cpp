//-----------------------------------------------------------------------------
// Example for DevIL image-handling library
// Copyright (C) 2000-2002 by Denton Woods
// Last modified:  4/22/2002
//
// Description: Simplest implementation of an DevIL application.
//  
// Source of the DevIL libraries/SDK:
// https://openil.sourceforge.net/
//
// Source of example programs, 
// This is the ResIL SDK, which was intended to be a currently-maintained
// successor to DevIL, though the headers aren't actually buildable.
// Fortunately, it *did* have usable example programs.
// https://sourceforge.net/projects/resil/
//
// build:  g++ -Wall -O3 -DSTAND_ALONE DevILlib.cpp -o devil.exe -lDevIL
// build:  g++ -Wall -O3 -DSTAND_ALONE DevILlib.cpp -std=c++0x -o devil.exe -lDevIL
//-----------------------------------------------------------------------------
//  NOTE: effective from August 2023, the DevIL library is no longer used
//  in this application... support for tiff/avif is provided by MediaInfo library.
//-----------------------------------------------------------------------------

#include <windows.h>

// Required include files.
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#ifdef  STAND_ALONE
#include <stdio.h>

typedef  unsigned int  uint ;

#else
#include "media_list.h"
#include "file_fmts.h"

static char tempstr[MAXLINE+1] ;
#endif

#include <IL/il.h>

/* We would need ILU just because of iluErrorString() function... */
/* So make it possible for both with and without ILU!  */
#ifdef ILU_ENABLED
#include <IL/ilu.h>
#define PRINT_ERROR_MACRO printf("Error: %s\n", iluErrorString(Error))
#else /* not ILU_ENABLED */
#define PRINT_ERROR_MACRO printf("Error: 0x%X\n", (unsigned int)Error)
#endif /* not ILU_ENABLED */

//****************************************************************************
//  error codes from il.h

typedef struct il_error_s {
   unsigned short ecode ;
   char const * const errstr ;
} il_error_t ;

il_error_t const il_error_msgs[28] = {
{ IL_NO_ERROR             , "no error" },
{ IL_INVALID_ENUM         , "Invalid enum" },
{ IL_OUT_OF_MEMORY        , "Out of memory" },
{ IL_FORMAT_NOT_SUPPORTED , "Unsupported format" },
{ IL_INTERNAL_ERROR       , "Internal error" },
{ IL_INVALID_VALUE        , "Invalid value" },
{ IL_ILLEGAL_OPERATION    , "Illegal operation" },
{ IL_ILLEGAL_FILE_VALUE   , "Illegal file value" },
{ IL_INVALID_FILE_HEADER  , "Invalid file header" },
{ IL_INVALID_PARAM        , "Invalid parameter" },
{ IL_COULD_NOT_OPEN_FILE  , "Cannot open file" },
{ IL_INVALID_EXTENSION    , "Invalid extension" },
{ IL_FILE_ALREADY_EXISTS  , "File already exists" },
{ IL_OUT_FORMAT_SAME      , "Out format same" },
{ IL_STACK_OVERFLOW       , "Stack overflow" },
{ IL_STACK_UNDERFLOW      , "Stack underflow" },
{ IL_INVALID_CONVERSION   , "Invalid conversion" },
{ IL_BAD_DIMENSIONS       , "Bad dimensions" },
{ IL_FILE_READ_ERROR      , "File read error" },
{ IL_FILE_WRITE_ERROR     , "File write error" },

{ IL_LIB_GIF_ERROR        , "Lib: GIF error" },
{ IL_LIB_JPEG_ERROR       , "Lib: JPEG error" },
{ IL_LIB_PNG_ERROR        , "Lib: PNG error" },
{ IL_LIB_TIFF_ERROR       , "Lib: TIFF error" },
{ IL_LIB_MNG_ERROR        , "Lib: MNG error" },
{ IL_LIB_JP2_ERROR        , "Lib: JP2 error" },
{ IL_LIB_EXR_ERROR        , "Lib: EXR error" },
{ IL_UNKNOWN_ERROR        , "Unknown error" }};

char const * const iLGetErrorStr(unsigned short ecode)   
{
   uint idx ;
   for (idx=0; idx != IL_UNKNOWN_ERROR; idx++) {
      if (ecode == il_error_msgs[idx].ecode) {
         return il_error_msgs[idx].errstr ;
      }
   }
   return "Unknown error" ;
}

//****************************************************************************
int use_DevIL_lib(char *fname, char *mlstr)
{
   ILuint   ImgId;
   char* image_name ;
   
#ifdef  STAND_ALONE
   image_name = fname ;
#else
static char fpath[1024] ;
   sprintf(fpath, "%s\\%s", base_path, fname) ;
   image_name = fpath ;
#endif
   

   // Check if the shared lib's version matches the executable's version.
   if (ilGetInteger(IL_VERSION_NUM) < IL_VERSION) {
#ifdef  STAND_ALONE
      printf("DevIL version is different...exiting!\n");
#else      
      sprintf(tempstr, "DevIL version is different");
      sprintf(mlstr, "%-30s", tempstr) ;
#endif      
      return 2;
   }

   // Initialize DevIL.
   ilInit();
#ifdef ILU_ENABLED
   iluInit();
#endif 

   // Generate the main image name to use.
   ilGenImages(1, &ImgId);

   // Bind this image name.
   ilBindImage(ImgId);

   // Loads the image specified by File into the image named by ImgId.
   if (!ilLoadImage(image_name)) {
      ILenum Error = ilGetError();
#ifdef  STAND_ALONE
      // Simple Error detection loop that displays the Error to the user in a human-readable form.
      printf("%s: %s\n", image_name, iLGetErrorStr(Error));
      
      // while ((Error = ilGetError())) {
      //    PRINT_ERROR_MACRO;
      // }
      
#else
      sprintf(tempstr, "%s", iLGetErrorStr((u16) Error));
      sprintf(mlstr, "%-30s", tempstr) ;
#endif      
      return 3;
   }

   uint width  = (uint) ilGetInteger(IL_IMAGE_WIDTH);
   uint height = (uint) ilGetInteger(IL_IMAGE_HEIGHT);
   uint depth  = (uint) ilGetInteger(IL_IMAGE_DEPTH);
   uint bpp    = (uint) ilGetInteger(IL_IMAGE_BITS_PER_PIXEL);
   
#ifdef  STAND_ALONE
   //  trying to differentiate betwee IL_JFIF / IL_EXIF in jpg files
   // uint itype  = (uint) ilGetInteger(IL_IMAGE_TYPE);
   
   // Display the image's dimensions to the end user.
   printf("%s: %ux%u, Depth: %u  Bpp: %u\n", image_name, width, height, depth, bpp);
#else
   if (depth == 1) {
      sprintf(tempstr, "%4u x %4u, %u bpp", width, height, bpp);
   }
   else {
      sprintf(tempstr, "%4u x %4u, %u/%u bpp", width, height, depth, bpp);
   }
   sprintf(mlstr, "%-30s", tempstr) ;
#endif          

   // We're done with the image, so let's delete it.
   ilDeleteImages(1, &ImgId);

   {
   ILenum   Error;
   // Simple Error detection loop that displays the Error to the user in a human-readable form.
   // I don't want to do this in an actual application, due to potential for infinite loop.
   if ((Error = ilGetError()) != IL_NO_ERROR) {
      PRINT_ERROR_MACRO;
   }
   }

   return 0 ;   
}

#ifdef  STAND_ALONE
//****************************************************************************
int main(int argc, char **argv)
{
   // We use the filename specified in the first argument of the command-line.
   if (argc < 2) {
      printf("DevIL_test : DevIL simple command line application.\n");
      printf("Usage : DevIL_test <file> [output]\n");
      printf("Default output is test.tga\n");
      return 1;
   }

   return use_DevIL_lib(argv[1], NULL);
}

#else
int get_devil_info(char *fname, char *mlstr)
{
   return use_DevIL_lib(fname, mlstr);
}

#endif

