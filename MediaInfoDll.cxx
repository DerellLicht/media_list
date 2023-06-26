/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */
//  build:  g++ -Wall -O3 -DSTAND_ALONE MediaInfoDll.cxx -o test.exe

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
#include <iostream>
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
#ifdef  STAND_ALONE
static void cstrprn(String sbuff, bool end_newline)
{
   const char *stemp = sbuff.c_str() ;
   if (end_newline) {
      printf("%s\n", stemp);
   }
   else {
      printf("%s", stemp);
   }
}
#endif
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static uint cstruint(String sbuff)
{
   // const char *stemp = sbuff.c_str() ;
   return (uint) strtoul(sbuff.c_str(), NULL, 10);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef  STAND_ALONE
static __int64 cstrull(String sbuff)
{
   // const char *stemp = sbuff.c_str() ;
   return (__int64) strtoull(sbuff.c_str(), NULL, 10);
}
#endif
/*
String Get(stream_t StreamKind, size_t StreamNumber, size_t Parameter, info_t InfoKind = Info_Text)  
{
   MEDIAINFO_TEST_STRING; 
   return MediaInfo_GetI(Handle, (MediaInfo_stream_C) StreamKind, StreamNumber, Parameter, 
   (MediaInfo_info_C) InfoKind);
};
   
String Get(stream_t StreamKind, size_t StreamNumber, const String &Parameter, 
           info_t InfoKind = Info_Text, info_t SearchKind = Info_Name)  
{
   MEDIAINFO_TEST_STRING; 
   return MediaInfo_Get(Handle, (MediaInfo_stream_C)StreamKind, StreamNumber, 
      Parameter.c_str(), (MediaInfo_info_C) InfoKind, (MediaInfo_info_C) SearchKind);
};
*/

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static int parse_media_file(char *inpname, char *mlstr)
{
   MediaInfo MI;

#ifndef  STAND_ALONE
static char fpath[260] ;
   sprintf(fpath, "%s\\%s", base_path, inpname) ;
   int hdl = _open(fpath, _O_BINARY | _O_RDONLY) ;
#else
   int hdl = _open(inpname, _O_BINARY | _O_RDONLY) ;
#endif   
   if (hdl == -1) {
      printf("[%d] %s: %s\n", hdl, inpname, strerror(NULL));
      return 1;
   }

   //From: preparing a memory buffer for reading
   unsigned char* From_Buffer = new unsigned char[7*188]; //Note: you can do your own buffer
   size_t From_Buffer_Size; //The size of the read file buffer

   //From: retrieving file size
   _lseeki64(hdl, 0, SEEK_END);
   __int64 F_Size = _telli64(hdl);
   _lseeki64(hdl, 0, SEEK_SET);

   //Initializing MediaInfo

   //Preparing to fill MediaInfo with a buffer
   if (F_Size == 0) {
#ifdef  STAND_ALONE
      printf("ERROR: file too large (> 31 bits)\n");
#endif      
      return 1 ;
   }
#ifdef  STAND_ALONE
   printf("F_Size: %I64d\n", F_Size);
#endif      
   MI.Open_Buffer_Init(F_Size, 0);

   //The parsing loop
   do
   {
       //Reading data somewhere, do what you want for this.
       // From_Buffer_Size = fread(From_Buffer, 1, 7*188, F);
       From_Buffer_Size = _read(hdl, From_Buffer, 7*188);

       //Sending the buffer to MediaInfo
       size_t Status = MI.Open_Buffer_Continue(From_Buffer, From_Buffer_Size);
       if (Status&0x08) //Bit3=Finished
           break;

       //Testing if there is a MediaInfo request to go elsewhere
       if (MI.Open_Buffer_Continue_GoTo_Get() != (MediaInfo_int64u)-1)
       {
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

#ifdef  STAND_ALONE
   //Get() example
    puts("");
    puts("Inform with Complete=false");
    MI.Option(__T("Complete"));
    cstrprn(MI.Inform(), true);

    // To_Display += __T("Inform with Complete=true\n");
    // puts("");
    // puts("Inform with Complete=true");
    // MI.Option(__T("Complete"), __T("1"));
    // cstrprn(MI.Inform(), true);

    puts("");
    puts("Custom Inform");
    MI.Option(__T("Inform"), __T("General;Example : FileSize=%FileSize%"));
    cstrprn(MI.Inform(), true);

    printf("Get with Stream=General and Parameter=\"FileSize\"\n");
    __int64 file_size = cstrull(MI.Get(Stream_General, 0, __T("FileSize"), Info_Text, Info_Name));
    printf("file size: %I64u bytes\n", file_size);
#endif    
    uint video_width = cstruint(MI.Get(Stream_Video, 0, __T("Width"), Info_Text, Info_Name));
    uint video_height = cstruint(MI.Get(Stream_Video, 0, __T("Height"), Info_Text, Info_Name));
    uint video_duration = cstruint(MI.Get(Stream_Video, 0, __T("Duration"), Info_Text, Info_Name));
#ifdef  STAND_ALONE
    printf("video size: %ux%u, %u msec\n", video_width, video_height, video_duration);

    cstrprn(__T("GetI with Stream=General and Parameter=46"), true);
    // To_Display += MI.Get(Stream_General, 0, 46, Info_Text);
    cstrprn(MI.Get(Stream_General, 0, 46, Info_Text), true);

    cstrprn(__T("Count_Get with StreamKind=Stream_Audio"), true);
    String To_Display ;
    #ifdef __MINGW32__
        Char* C1 = new Char[33];
        _itot (MI.Count_Get(Stream_Audio), C1, 10);
        To_Display +=C1;
        delete[] C1;
    #else
        toStringStream SS;
        SS << std::setbase(10) << MI.Count_Get(Stream_Audio);
        To_Display += SS.str();
    #endif
    cstrprn(To_Display, true);

    cstrprn(__T("Get with Stream=General and Parameter=\"AudioCount\""), true);
    cstrprn(MI.Get(Stream_General, 0, __T("AudioCount"), Info_Text, Info_Name), true);

    cstrprn(__T("Get with Stream=Audio and Parameter=\"StreamCount\""), true);
    cstrprn(MI.Get(Stream_Audio, 0, __T("StreamCount"), Info_Text, Info_Name), true);
    
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