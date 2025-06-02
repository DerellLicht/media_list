//**********************************************************************************
//  Copyright (c) 1998-2025 Derell Licht
//  media_list.cpp - list info about various media files
//**********************************************************************************

#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#include "common.h"
#include "conio_min.h"
#include "media_list.h"
#include "qualify.h"

//  V1.01  Convert to Unicode
//  V1.02  Add support for SVG files
#define  VER_NUMBER "1.02"

//lint -esym(843, Version, ShortVersion) could be declared as const
TCHAR *Version = _T("MediaList, Version " VER_NUMBER " ") ;   //lint !e707
TCHAR *ShortVersion = _T(" medialist " VER_NUMBER " ") ;       //lint !e707

//  per Jason Hood, this turns off MinGW's command-line expansion, 
//  so we can handle wildcards like we want to.                    
//lint -e765  external '_CRT_glob' could be made static
//lint -e714  Symbol '_CRT_glob' not referenced
int _CRT_glob = 0 ;

double total_ptime = 0.0 ;

//lint -esym(534, FindClose)  // Ignoring return value of function
//lint -esym(818, filespec, argv)  //could be declared as pointing to const
//lint -e10  Expecting '}'

static ffdata_t *ftop  = NULL;
static ffdata_t *ftail = NULL;

static uint filecount = 0 ;

//**********************************************************************************
int read_files(TCHAR *filespec)
{
   WIN32_FIND_DATA fdata ; //  long-filename file struct
   int done, fn_okay ;
   HANDLE handle;
   ffdata_t *ftemp;

   handle = FindFirstFile (filespec, &fdata);
   //  according to MSDN, Jan 1999, the following is equivalent
   //  to the preceding... unfortunately, under Win98SE, it's not...
   // handle = FindFirstFileEx(target[i], FindExInfoStandard, &fdata, 
   //                      FindExSearchNameMatch, NULL, 0) ;
   if (handle == INVALID_HANDLE_VALUE) {
      return -errno;
   }

   //  loop on find_next
   done = 0;
   while (!done) {
      //  filter out directories if not requested
      if ((fdata.dwFileAttributes & FILE_ATTRIBUTE_VOLID) != 0)
         fn_okay = 0;
      else if ((fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
         fn_okay = 1;
      //  For directories, filter out "." and ".."
      else if (fdata.cFileName[0] != _T('.')) //  fn=".something"
         fn_okay = 1;
      else if (fdata.cFileName[1] == 0)   //  fn="."
         fn_okay = 0;
      else if (fdata.cFileName[1] != _T('.')) //  fn="..something"
         fn_okay = 1;
      else if (fdata.cFileName[2] == 0)   //  fn=".."
         fn_okay = 0;
      else
         fn_okay = 1;

      if (fn_okay) {
         filecount++;

         //****************************************************
         //  allocate and initialize the structure
         //****************************************************
         ftemp = (ffdata_t *) new ffdata_t ;
         ZeroMemory((void *) ftemp, sizeof(ffdata_t));

         ftemp->attrib = (uchar) fdata.dwFileAttributes;

         //  convert file time
         // if (n.fdate_option == FDATE_LAST_ACCESS)
         //    ftemp->ft = fdata.ftLastAccessTime;
         // else if (n.fdate_option == FDATE_CREATE_TIME)
         //    ftemp->ft = fdata.ftCreationTime;
         // else
         //    ftemp->ft = fdata.ftLastWriteTime;
         ftemp->ft = fdata.ftLastAccessTime;

         //  convert file size
         u64toul iconv;
         iconv.u[0] = fdata.nFileSizeLow;
         iconv.u[1] = fdata.nFileSizeHigh;
         ftemp->fsize = iconv.i;

         // ftemp->filename = (char *) malloc(strlen ((char *) fdata.cFileName) + 1);
         ftemp->filename = (TCHAR *) new TCHAR[(_tcslen ((TCHAR *) fdata.cFileName) + 1)];
         _tcscpy (ftemp->filename, (TCHAR *) fdata.cFileName);

         ftemp->dirflag = ftemp->attrib & FILE_ATTRIBUTE_DIRECTORY;

         //****************************************************
         //  add the structure to the file list
         //****************************************************
         if (ftop == NULL) {
            ftop = ftemp;
         }
         else {
            ftail->next = ftemp;
         }
         ftail = ftemp;
      }  //  if file is parseable...

      //  search for another file
      if (FindNextFile (handle, &fdata) == 0) {
         done = 1;
      }
   }

   FindClose (handle);
   return 0;
}

//********************************************************************************
//  this solution is from:
//  https://github.com/coderforlife/mingw-unicode-main/
//********************************************************************************
#if defined(__GNUC__) && defined(_UNICODE)

#ifndef __MSVCRT__
#error Unicode main function requires linking to MSVCRT
#endif

#include <wchar.h>
#include <stdlib.h>

extern int _CRT_glob;
extern 
#ifdef __cplusplus
"C" 
#endif
void __wgetmainargs(int*,wchar_t***,wchar_t***,int,int*);

#ifdef MAIN_USE_ENVP
int wmain(int argc, wchar_t *argv[], wchar_t *envp[]);
#else
int wmain(int argc, wchar_t *argv[]);
#endif

int main() 
{
   wchar_t **enpv, **argv;
   int argc, si = 0;
   __wgetmainargs(&argc, &argv, &enpv, _CRT_glob, &si); // this also creates the global variable __wargv
#ifdef MAIN_USE_ENVP
   return wmain(argc, argv, enpv);
#else
   return wmain(argc, argv);
#endif
}

#endif //defined(__GNUC__) && defined(_UNICODE)

//********************************************************************************
static TCHAR file_spec[MAX_FILE_LEN+1] = _T("") ;

int wmain(int argc, wchar_t *argv[])
{
   int idx, result ;

   console_init() ;
   
   //  okay, the cause of this, is that apparently I have to use 
   //  double-backslash to put a quote after a backslash...
   //  But forward slash works fine...
   // > medialist glock17\"буяновский страйкбол"
   // arg 1: [glock17"буяновский]
   // arg 2: [страйкбол]
   
   //  unicons derelict cornucopia "буяновский страйкбол"
   // > medialist glock17\\"буяновский страйкбол"
   // filespec: D:\SourceCode\Git\media_list\glock17\буяновский страйкбол\*, fcount: 3
   
   for (idx=1; idx<argc; idx++) {
      TCHAR *p = argv[idx] ;
      _tcsncpy(file_spec, p, MAX_FILE_LEN);
      file_spec[MAX_FILE_LEN] = 0 ;
   }

   if (file_spec[0] == 0) {
      _tcscpy(file_spec, _T("."));
   }

   result = qualify(file_spec) ;
   if (result == QUAL_INV_DRIVE) {
      dputsf(_T("%s: %d\n"), file_spec, result);
      return 1 ;
   }
   
   //  Extract base path from first filespec,
   //  and strip off filename
   _tcscpy(base_path, file_spec) ;
   TCHAR *strptr = _tcsrchr(base_path, _T('\\')) ;
   if (strptr != 0) {
       strptr++ ;  //lint !e613  skip past backslash, to filename
      *strptr = 0 ;  //  strip off filename
   }
   base_len = _tcslen(base_path) ;
   // printf("base path: %s\n", base_path);
   
   result = read_files(file_spec);
   if (result < 0) {
      dputsf(_T("filespec: %s, %s\n"), file_spec, strerror(-result));
   }
   else {
      dputsf(_T("\nfilespec: %s, fcount: %u\n"), file_spec, filecount);
      if (filecount > 0) {
         puts("");
         for (ffdata_t *ftemp = ftop; ftemp != NULL; ftemp = ftemp->next) {
            // _tprintf(_T("%s\n"), ftemp->filename);
            print_media_info(ftemp);
         }

      }
      
      //  see if there is any special results to display
      TCHAR timestr[80] ;
      if (total_ptime > 0x01) {
         if (total_ptime < 60.0) {
            _stprintf(timestr, _T("%.2f seconds     "), total_ptime) ;  //lint !e592
         } else {
            total_ptime /= 60.0 ;
            _stprintf(timestr, _T("%.2f minutes     "), total_ptime) ;  //lint !e592
         }
         dputsf(_T("\ntotal playing time: %s\n"), timestr) ;
      }
   }
   restore_console_attribs();
   return 0;
}

