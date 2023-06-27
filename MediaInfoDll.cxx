/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DDM Notes:    
//  https://mediaarea.net/en/MediaInfo/Download/Windows 
//  I downloaded the 32bit version in .zip format
//  build:  g++ -Wall -O3 -DSTAND_ALONE MediaInfoDll.cxx -o test.exe
//  
//  The reason for renaming this file from .cpp to .cxx is so
//  I could specify different build rules for it.
//  The main goal was to eliminate warning messages that I get from gcc,
//  due to use of the -Weffc++ compiler flag, which implements additional
//  warnings related to the book Effective C++ .
//  This also allows me to skip this file when running Lint,
//  which also flags many issues in the MediaInfoDLL package.
//  The main goal, overall, is to avoid making modifications to that
//  library, which I have no control over.
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// #define  STAND_ALONE 1
 
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Example for MediaInfoLib
// Command line version
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef MEDIAINFO_LIBRARY
    #include "MediaInfo.h" //Staticly-loaded library (.lib or .a or .so)
    #define MediaInfoNameSpace MediaInfoLib;
#else //MEDIAINFO_LIBRARY
    #include "MediaInfoDLL.h" //Dynamicly-loaded library (.dll or .so)
    #define MediaInfoNameSpace MediaInfoDLL;
#endif //MEDIAINFO_LIBRARY
// #include <iostream>
// #include <iomanip>
using namespace MediaInfoNameSpace;

#ifdef __MINGW32__
    #ifdef _UNICODE
        #define _itot _itow
    #else //_UNICODE
        #define _itot itoa
    #endif //_UNICODE
#endif //__MINGW32

// #include <windows.h>
#include <stdio.h>
#include <errno.h>
#include <io.h>   // _lseeki64
#include <sys/stat.h>   // _open
#include <fcntl.h>   // _open

#ifndef  STAND_ALONE
#include "media_list.h"
#include "file_fmts.h"

#else
typedef  unsigned int  uint ;
#endif

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static int parse_media_file(char *inpname, char *mlstr)
{
   MediaInfo MI;

#ifdef  STAND_ALONE
   int hdl = _open(inpname, _O_BINARY | _O_RDONLY) ;
   if (hdl == -1) {
      printf("[%d] %s: %s\n", hdl, inpname, strerror(NULL));
      return 1;
   }
#else
static char fpath[1024] ;
   sprintf(fpath, "%s\\%s", base_path, inpname) ;
   int hdl = _open(fpath, _O_BINARY | _O_RDONLY) ;
   if (hdl == -1) {
      sprintf(mlstr, "%-30s", "open failed") ;
      return 0 ;
   }
#endif   

   //From: preparing a memory buffer for reading
   unsigned char* From_Buffer = new unsigned char[7*188]; //Note: you can do your own buffer
   size_t From_Buffer_Size; //The size of the read file buffer

   //From: retrieving file size
   _lseeki64(hdl, 0, SEEK_END);
   __int64 F_Size = _telli64(hdl);
   _lseeki64(hdl, 0, SEEK_SET);
   //Preparing to fill MediaInfo with a buffer
   if (F_Size == 0) {
#ifdef  STAND_ALONE
      printf("ERROR: bad file handle\n");
      return 1 ;
#else      
      sprintf(mlstr, "%-30s", "bad file handle") ;
      return 0 ;
#endif      
   }

   //Initializing MediaInfo

   // MediaInfoDLL_Load(); //  already done
   void *Handle = MediaInfo_New();
#ifdef  STAND_ALONE
   MediaInfo_Open(Handle, inpname);
#else
   MediaInfo_Open(Handle, fpath);
#endif   
   MI.Open_Buffer_Init(F_Size, 0);

   //The parsing loop
   do {
       //Reading data somewhere, do what you want for this.
       // From_Buffer_Size = fread(From_Buffer, 1, 7*188, F);
       From_Buffer_Size = _read(hdl, From_Buffer, 7*188);

       //Sending the buffer to MediaInfo
       size_t Status = MI.Open_Buffer_Continue(From_Buffer, From_Buffer_Size);
       if (Status&0x08) { //Bit3=Finished
           break;
       }

       //Testing if there is a MediaInfo request to go elsewhere
       if (MI.Open_Buffer_Continue_GoTo_Get() != (MediaInfo_int64u)-1) {
           // fseek(F, (long)MI.Open_Buffer_Continue_GoTo_Get(), SEEK_SET);   //Position the file
           _lseeki64(hdl, (__int64)MI.Open_Buffer_Continue_GoTo_Get(), SEEK_SET);
           // MI.Open_Buffer_Init(F_Size, ftell(F));                          //Informing MediaInfo we have seek
           MI.Open_Buffer_Init(F_Size, _telli64(hdl));                          //Informing MediaInfo we have seek
       }
   }
   while (From_Buffer_Size>0);
   _close(hdl);

   //Finalizing
   MI.Open_Buffer_Finalize(); //This is the end of the stream, MediaInfo must finnish some work
   const MediaInfo_Char* sptr ;

#ifdef  STAND_ALONE
   //Get() example
   puts("");
   puts("Inform with Complete=false");
   MI.Option(__T("Complete"));
   // cstrprn(MI.Inform(), true);
   sptr = MediaInfo_Inform(Handle, 0);
   printf("%s\n", sptr);

   // To_Display += __T("Inform with Complete=true\n");
   // puts("");
   // puts("Inform with Complete=true");
   // MI.Option(__T("Complete"), __T("1"));
   // cstrprn(MI.Inform(), true);

   // puts("");
   // puts("Custom Inform");
   // MI.Option(__T("Inform"), __T("General;Example : FileSize=%FileSize%"));
   // sptr = MediaInfo_Inform(Handle, 0);
   // printf("%s\n", sptr);

   printf("Get with Stream=General and Parameter=\"FileSize\"\n");
   // __int64 file_size = cstrull(MI.Get(Stream_General, 0, __T("FileSize"), Info_Text, Info_Name));
   sptr = MediaInfo_Get(Handle, (MediaInfo_stream_C)Stream_General, 0, "FileSize", 
                       (MediaInfo_info_C) Info_Text, (MediaInfo_info_C) Info_Name);
   __int64 file_size = (__int64) strtoull(sptr, NULL, 10);
   printf("file size: %I64u bytes\n", file_size);
#endif    

   sptr = MediaInfo_Get(Handle, (MediaInfo_stream_C)Stream_Video, 0, "Width", 
                       (MediaInfo_info_C) Info_Text, (MediaInfo_info_C) Info_Name);
   uint video_width = (uint) atoi(sptr);
   sptr = MediaInfo_Get(Handle, (MediaInfo_stream_C)Stream_Video, 0, "Height", 
                       (MediaInfo_info_C) Info_Text, (MediaInfo_info_C) Info_Name);
   uint video_height = (uint) atoi(sptr);
   sptr = MediaInfo_Get(Handle, (MediaInfo_stream_C)Stream_Video, 0, "Duration", 
                       (MediaInfo_info_C) Info_Text, (MediaInfo_info_C) Info_Name);
   uint video_duration = (uint) atoi(sptr);

#ifdef  STAND_ALONE
   printf("video size: %ux%u, %u msec\n", video_width, video_height, video_duration);

   // printf("GetI with Stream=General and Parameter=46\n");
   // // String To_Display = MI.Get(Stream_General, 0, 46, Info_Text);
   // // sptr = To_Display.c_str() ;
   //  This one doesn't work; passing int to Get() is not valid
   // sptr = MediaInfo_Get(Handle, (MediaInfo_stream_C)Stream_General, 0, 46, 
   //                     (MediaInfo_info_C) Info_Text, (MediaInfo_info_C) Info_Name);
   // printf("Param 46: %s\n", sptr)                    ;

   // printf("Count_Get with StreamKind=Stream_Audio\n");
   // String To_Display ;
   // #ifdef __MINGW32__
   //     Char* C1 = new Char[33];
   //     _itot (MI.Count_Get(Stream_Audio), C1, 10);
   //     To_Display +=C1;
   //     delete[] C1;
   // #else
   //     toStringStream SS;
   //     SS << std::setbase(10) << MI.Count_Get(Stream_Audio);
   //     To_Display += SS.str();
   // #endif
   // cstrprn(To_Display, true);

   printf("Get with Stream=General and Parameter=\"AudioCount\"\n");
   // cstrprn(MI.Get(Stream_General, 0, __T("AudioCount"), Info_Text, Info_Name), true);
   sptr = MediaInfo_Get(Handle, (MediaInfo_stream_C)Stream_General, 0, "AudioCount", 
                       (MediaInfo_info_C) Info_Text, (MediaInfo_info_C) Info_Name);
   printf("AudioCount: %u\n", (uint) atoi(sptr));

   printf("Get with Stream=Audio and Parameter=\"StreamCount\"\n");
   // cstrprn(MI.Get(Stream_Audio, 0, __T("StreamCount"), Info_Text, Info_Name), true);
   sptr = MediaInfo_Get(Handle, (MediaInfo_stream_C)Stream_Audio, 0, "StreamCount", 
                       (MediaInfo_info_C) Info_Text, (MediaInfo_info_C) Info_Name);
   printf("StreamCount: %u\n", (uint) atoi(sptr));

   MediaInfo_Close(Handle);
   MI.Close();   //  ??
   return 0;
#else
   MI.Close();   //  ??
   double run_time = (double) video_duration / 1000.0 ;
   if (run_time < 60.0) {
      sprintf(tempstr, "%4u x %4u, %6.2f secs", video_width, video_height, run_time) ;
   } else {
      run_time /= 60.0 ;
      sprintf(tempstr, "%4u x %4u, %6.2f mins", video_width, video_height, run_time) ;
   }
   sprintf(mlstr, "%-30s", tempstr) ;
    
   MediaInfo_Close(Handle);
   MI.Close();   //  ??
   return 0;
#endif

}

#ifdef  STAND_ALONE
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
int main (int argc, char **argv)
{
   char infname[1024] = "";
   int idx ;
   for (idx=1; idx<argc; idx++) {
      char *p = argv[idx];
      strcpy(infname, p);
   }
   
   if (infname[0] == 0) {
      printf("Usage: MediaInfoDll media_filename\n");
      return 1 ;
   }
   
   printf("parsing: %s\n", infname);
   parse_media_file(infname, NULL);
   return 0 ;
}

#else
int get_mi_info(char *fname, char *mlstr)
{
   return parse_media_file(fname, mlstr);
}

#endif