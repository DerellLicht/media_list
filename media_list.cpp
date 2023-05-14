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

#include "media_list.h"
#include "qualify.h"

WIN32_FIND_DATA fdata ; //  long-filename file struct

//  per Jason Hood, this turns off MinGW's command-line expansion, 
//  so we can handle wildcards like we want to.                    
//lint -e765  external '_CRT_glob' could be made static
//lint -e714  Symbol '_CRT_glob' not referenced
int _CRT_glob = 0 ;

uint filecount = 0 ;

double total_ptime = 0.0 ;
//lint -esym(843, show_all)
bool show_all = true ;

//lint -esym(534, FindClose)  // Ignoring return value of function
//lint -esym(818, filespec, argv)  //could be declared as pointing to const
//lint -e10  Expecting '}'

ffdata *ftop  = NULL;
ffdata *ftail = NULL;

//  name of drive+path without filenames
char base_path[PATH_MAX] ;
unsigned base_len ;  //  length of base_path

//*****************************************************************************
// ULLONG_MAX = 18,446,744,073,709,551,615
//*****************************************************************************
char *convert_to_commas(ULONGLONG uli, char *outstr)
{  //lint !e1066
   int slen, inIdx, j ;
   char *strptr ;
   char temp_ull_str[MAX_ULL_COMMA_LEN+1] ;
   static char local_ull_str[MAX_ULL_COMMA_LEN+1] ;
   if (outstr == NULL) {
       outstr = local_ull_str ;
   }

   // sprintf(temp_ull_str, "%"PRIu64"", uli);
   // sprintf(temp_ull_str, "%llu", uli);
   sprintf(temp_ull_str, "%I64u", uli);
   // _ui64toa(uli, temp_ull_str, 10) ;
   slen = strlen(temp_ull_str) ;
   inIdx = --slen ;//  convert byte-count to string index 

   //  put NULL at end of output string
   strptr = outstr + MAX_ULL_COMMA_LEN ;
   *strptr-- = 0 ;   //  make sure there's a NULL-terminator

   for (j=0; j<slen; j++) {
      *strptr-- = temp_ull_str[inIdx--] ;
      if ((j+1) % 3 == 0)
         *strptr-- = ',' ;
   }
   *strptr = temp_ull_str[inIdx] ;

   //  copy string from tail-aligned to head-aligned
   strcpy(outstr, strptr) ;
   return outstr ;
}

//**********************************************************************************
int read_files(char *filespec)
{
   int done, fn_okay ;
   HANDLE handle;
   ffdata *ftemp;

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
      if (!show_all) {
         if ((fdata.dwFileAttributes & 
            (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM)) != 0) {
            fn_okay = 0 ;
            goto search_next_file;
         }
      }
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
         ftemp = (struct ffdata *) malloc(sizeof(ffdata)) ;
         if (ftemp == NULL) {
            return -errno;
         }
         memset((char *) ftemp, 0, sizeof(ffdata));

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

search_next_file:
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
      strcpy(file_spec, ".\\*");
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
      printf("filespec: %s, %u found\n", file_spec, filecount);
      if (filecount > 0) {
         puts("");
         for (ffdata *ftemp = ftop; ftemp != NULL; ftemp = ftemp->next) {
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
         printf("total playing time: %s\n", mlstr) ;
      }
   }
   return 0;
}

