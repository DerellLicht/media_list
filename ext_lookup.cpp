//*****************************************************************
//  Copyright (c) 1998-2023 Daniel D. Miller                       
//  ext_lookup - file extension lookup
//  This file contains the extension-lookup and vectoring table
//  with associated code.
//*****************************************************************

#undef __STRICT_ANSI__
#ifdef USE_64BIT
#define  _WIN32_WINNT    0x0600
#endif
#include <windows.h>
#include <stdio.h>
#ifdef _lint
#include <stdlib.h>  //  PATH_MAX
#endif
#ifdef USE_64BIT
#include <fileapi.h>
#endif

#include "media_list.h"
#include "file_fmts.h"

//************************************************************************
//  lookup tables for special-extension display functions
//************************************************************************
typedef struct mm_lookup_s {
   char  ext[MAX_EXT_SIZE] ;
   int (*func)(char *fname, char *mlstr) ;
} mm_lookup_t ;

static mm_lookup_t const mm_lookup[] = {
//  image formats
{ "ani",  get_ani_info },
{ "bmp",  get_bmp_info },
{ "cur",  get_ico_cur_info },
{ "ico",  get_ico_cur_info },
{ "gif",  get_gif_info },
{ "jpg",  get_jpeg_info },
{ "png",  get_png_info },
// { "bmp",  get_devil_info },
// { "cur",  get_devil_info },   //  wrong color depth
// { "ico",  get_devil_info },   //  wrong color depth
// { "gif",  get_devil_info },
// { "jpg",  get_devil_info },
// { "png",  get_devil_info },   //  much slower than my code!!
{ "sid",  get_sid_info },
{ "tif",  get_devil_info },
{ "tiff", get_devil_info },
{ "webp", get_webp_info },
//  audio formats
{ "flac", get_mi_info },
{ "mp3",  get_mi_info },
{ "ogg",  get_mi_info },
{ "wav",  get_mi_info },
{ "wma",  get_mi_info },
//  video formats
{ "avi",  get_mi_info },
{ "flv",  get_mi_info },
{ "mkv",  get_mi_info },
{ "mov",  get_mi_info },
{ "mp4",  get_mi_info },
{ "mpeg", get_mi_info },
{ "mpg",  get_mi_info },
{ "webm", get_mi_info },
{ "wmv",  get_mi_info },
{ "", 0 }} ;

//************************************************************************
int print_media_info(ffdata const * const fptr)
{
   char mlstr[31] = "";
   int show_normal_info ;

   show_normal_info = 1 ;
   //  display directory entry
   if (fptr->dirflag) {
      printf("%14s  ", "");
      printf("%30s", " ");
      printf("[%s]\n", fptr->filename);
   }

   //  display file entry
   else {
      char *p ;
      unsigned idx ;

      p = strrchr(fptr->filename, '.') ;
      if (p != 0  &&  strlen(p) <= MAX_EXT_SIZE) {
         p++ ; //  skip past the period

         for (idx=0; mm_lookup[idx].ext[0] != 0; idx++) {
            if (strnicmp(p, mm_lookup[idx].ext, sizeof(mm_lookup[idx].ext)) == 0) {
               //  call the special string generator function
               (*mm_lookup[idx].func)(fptr->filename, mlstr) ; //lint !e522
               show_normal_info = 0 ;
               break;
            }
         }
      }

      //  show file size
      printf("%14s  ", convert_to_commas(fptr->fsize, NULL));

      //  process multimedia display
      if (!show_normal_info) {
         printf("%-30s", mlstr);
      } 
      //  display normal file listing
      else {
         printf("%30s", " ");
      }

      //  format filename as required
      printf("%s\n", fptr->filename);
   }
   return 0 ;
}
