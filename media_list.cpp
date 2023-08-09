//**********************************************************************************
//  Copyright (c) 1998-2023 Daniel D. Miller                       
//  media_list.cpp - list info about various media files
//                                                                 
//  Written by:   Daniel D. Miller  (the derelict)                 
//  
//**********************************************************************************

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>  //  PATH_MAX

#include "common.h"
#include "media_list.h"
#include "qualify.h"

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
int read_files(char *filespec)
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
      else if (fdata.cFileName[0] != '.') //  fn=".something"
         fn_okay = 1;
      else if (fdata.cFileName[1] == 0)   //  fn="."
         fn_okay = 0;
      else if (fdata.cFileName[1] != '.') //  fn="..something"
         fn_okay = 1;
      else if (fdata.cFileName[2] == 0)   //  fn=".."
         fn_okay = 0;
      else
         fn_okay = 1;

      if (fn_okay) {
         // printf("DIRECTORY %04X %s\n", fdata.attrib, fdata.cFileName) ;
         // printf("%9ld %04X %s\n", fdata.file_size, fdata.attrib, fdata.cFileName) ;
         filecount++;

         //****************************************************
         //  allocate and initialize the structure
         //****************************************************
         // ftemp = new ffdata;
         ftemp = (ffdata_t *) malloc(sizeof(ffdata_t)) ;
         if (ftemp == NULL) {
            return -errno;
         }
         memset((char *) ftemp, 0, sizeof(ffdata_t));

         //  convert filename to lower case if appropriate
         // if (!n.ucase)
         //    strlwr(fblk.name) ;

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

         ftemp->filename = (char *) malloc(strlen ((char *) fdata.cFileName) + 1);
         strcpy (ftemp->filename, (char *) fdata.cFileName);

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
      if (FindNextFile (handle, &fdata) == 0)
         done = 1;
   }

   FindClose (handle);
   return 0;
}

//**********************************************************************************
char file_spec[PATH_MAX+1] = "" ;

int main(int argc, char **argv)
{
   int idx, result ;
   for (idx=1; idx<argc; idx++) {
      char *p = argv[idx] ;
      strncpy(file_spec, p, PATH_MAX);
      file_spec[PATH_MAX] = 0 ;
   }

   if (file_spec[0] == 0) {
      strcpy(file_spec, ".");
   }

   result = qualify(file_spec) ;
   if (result == QUAL_INV_DRIVE) {
      printf("%s: %d\n", file_spec, result);
      return 1 ;
   }
   // printf("file spec: %s\n", file_spec);
   
   //  Extract base path from first filespec,
   //  and strip off filename
   strcpy(base_path, file_spec) ;
   char *strptr = strrchr(base_path, '\\') ;
   if (strptr != 0) {
       strptr++ ;  //lint !e613  skip past backslash, to filename
      *strptr = 0 ;  //  strip off filename
   }
   base_len = strlen(base_path) ;
   // printf("base path: %s\n", base_path);
   
   result = read_files(file_spec);
   if (result < 0) {
      printf("filespec: %s, %s\n", file_spec, strerror(-result));
   }
   else {
      printf("filespec: %s\n", file_spec);
      if (filecount > 0) {
         puts("");
         for (ffdata_t *ftemp = ftop; ftemp != NULL; ftemp = ftemp->next) {
            // printf("%s\n", ftemp->filename);
            print_media_info(ftemp);
         }

      }
      
      //  see if there is any special results to display
      char mlstr[80] ;
      if (total_ptime > 0x01) {
         if (total_ptime < 60.0) {
            sprintf(mlstr, "%.2f seconds     ", total_ptime) ;
         } else {
            total_ptime /= 60.0 ;
            sprintf(mlstr, "%.2f minutes     ", total_ptime) ;
         }
         puts("");
         printf("total playing time: %s\n", mlstr) ;
      }
   }
   return 0;
}

