/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */
//  build:  g++ -Wall -O3 MediaInfoDll.cpp -o test.exe
 
#include <stdio.h>
#include <errno.h>
#include <io.h>   // _lseeki64
#include <sys/stat.h>   // _open
#include <fcntl.h>   // _open

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

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// int main (int argc, char **argv)
int parse_media_file(char *inpname)
{
   MediaInfo MI;

   //From: preparing an example file for reading
   // FILE* F=fopen(inpname, "rb"); //You can use something else than a file
   // if (F==0) {
   //    printf("fopen failed: %u\n", errno);
   //     return 1;
   // }
   int hdl = _open(inpname, _O_BINARY | _O_RDONLY) ;
   if (hdl == -1) {
      printf("%s: %s\n", inpname, strerror(NULL));
      return 1;
   }

   //From: preparing a memory buffer for reading
   unsigned char* From_Buffer = new unsigned char[7*188]; //Note: you can do your own buffer
   size_t From_Buffer_Size; //The size of the read file buffer

   //From: retrieving file size
   //  DDM Note: fseek/ftell won't work for files > 31 bits (2,147,483,647 bytes)
   // fseek(F, 0, SEEK_END);
   // long F_Size = ftell(F);
   // fseek(F, 0, SEEK_SET);
   _lseeki64(hdl, 0, SEEK_END);
   __int64 F_Size = _telli64(hdl);
   _lseeki64(hdl, 0, SEEK_SET);

   //Initializing MediaInfo

   //Preparing to fill MediaInfo with a buffer
   if (F_Size == 0) {
      printf("ERROR: file too large (> 31 bits)\n");
      return 1 ;
   }
   printf("F_Size: %I64d\n", F_Size);
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

   //Finalizing
   MI.Open_Buffer_Finalize(); //This is the end of the stream, MediaInfo must finnish some work

   //Get() example
   // To_Display=MI.Get(Stream_General, 0, __T("Format"));
   // cstrprn(MI.Get(Stream_General, 0, __T("Format")), true);
   // String sbuff ;

   // To_Display += __T("\r\n\r\nInfo_Parameters\r\n");
   // To_Display += MI.Option(__T("Info_Parameters"));
   
   //  To_Display += __T("\r\n\r\nInfo_Codecs\r\n");
   //  To_Display += MI.Option(__T("Info_Codecs"));
    
    // sbuff += __T("Inform with Complete=false\n");
    // cstrprn(sbuff);
    cstrprn(__T("Inform with Complete=false"), true);
    // stemp = sbuff.c_str() ;
    // printf("%s", stemp);
    MI.Option(__T("Complete"));
    // To_Display += MI.Inform();
    cstrprn(MI.Inform(), true);

    // To_Display += __T("Inform with Complete=true\n");
    cstrprn(__T("Inform with Complete=true"), true);
    MI.Option(__T("Complete"), __T("1"));
    // To_Display += MI.Inform();
    cstrprn(MI.Inform(), true);

    cstrprn(__T("Custom Inform"), true);
    MI.Option(__T("Inform"), __T("General;Example : FileSize=%FileSize%"));
    // To_Display += MI.Inform();
    cstrprn(MI.Inform(), true);

    cstrprn(__T("Get with Stream=General and Parameter=\"FileSize\""), true);
    cstrprn(MI.Get(Stream_General, 0, __T("FileSize"), Info_Text, Info_Name), true);

    cstrprn(__T("GetI with Stream=General and Parameter=46"), true);
    // To_Display += MI.Get(Stream_General, 0, 46, Info_Text);
    cstrprn(MI.Get(Stream_General, 0, 46, Info_Text), true);

    cstrprn(__T("Count_Get with StreamKind=Stream_Audio"), true);
    String To_Display ;
    #ifdef __MINGW32__
        Char* C1=new Char[33];
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
    // #ifdef _UNICODE
    //     std::wcout << To_Display;
    // #else
    //     std::cout  << To_Display;
    // #endif
    return 0;
}

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
   parse_media_file(infname);
   return 0 ;
}