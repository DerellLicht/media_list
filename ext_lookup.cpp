//*****************************************************************
//  Copyright (c) 1998-2023 Daniel D. Miller                       
//  ext_lookup - file extension lookup
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
{ "jpg",  get_jpeg_info },
{ "gif",  get_gif_info },
{ "wav",  get_wave_info },
{ "bmp",  get_bmp_info },
{ "png",  get_png_info },
{ "ico",  get_ico_info },
{ "cur",  get_cur_info },
{ "ani",  get_ani_info },
{ "sid",  get_sid_info },
//  audio formats
{ "mp3",  get_mp3_info },
{ "webp", get_webp_info },
{ "ogg",  get_zplay_info },
{ "flac", get_zplay_info },
{ "ac3",  get_zplay_info },
{ "aac",  get_zplay_info },
{ "pcm",  get_zplay_info },
//  video formats
{ "mp4",  get_mi_info },
{ "mkv",  get_mi_info },
{ "avi",  get_mi_info },
{ "mov",  get_mi_info },
{ "mpg",  get_mi_info },
{ "mpeg", get_mi_info },
{ "wmv",  get_mi_info },
{ "flv",  get_mi_info },
{ "webm", get_mi_info },
{ "", 0 }} ;

// static char const monthstr[12][4] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
//    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
// };

//************************************************************************
int print_media_info(ffdata const * const fptr)
{
   char mlstr[31] = "";
   int show_normal_info ;
   // uchar SHRattr = fptr->attrib & 7;
   // FILETIME lft;
   // FileTimeToLocalFileTime (&(fptr->ft), &lft);
   // SYSTEMTIME sdt ;
   // FileTimeToSystemTime(&lft, &sdt) ;
   // int secs  = sdt.wSecond ;
   // int mins  = sdt.wMinute ;
   // int hour  = sdt.wHour   ;
   // int day   = sdt.wDay    ;
   // int month = sdt.wMonth  ;
   // long year = sdt.wYear   ;

   show_normal_info = 1 ;
   //  display directory entry
   if (fptr->dirflag) {
      printf("%14s  ", "");
      // printf("%3s %02d, %04lu ", monthstr[month - 1], day, year);
      // printf("%02d:%02d:%02d ", hour, mins, secs);
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
         // printf("%3s %02d, %04lu ", monthstr[month - 1], day, year);
         // printf("%02d:%02d:%02d ", hour, mins, secs);
      }

      //  format filename as required
      printf("%s\n", fptr->filename);
   }
   return 0 ;
}
