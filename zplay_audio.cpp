//**********************************************************************************
// This is an audio-file player implemented using the libZPlay library.
// https://libzplay.sourceforge.net/
// 
// build: g++ -Wall -DSTAND_ALONE=1 -O2 zplay.cpp -o zplay.exe -lzplay
//**********************************************************************************

// #define  STAND_ALONE    1

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

// include libzplay header file
#include <libzplay.h>

#ifndef  STAND_ALONE
#include "media_list.h"
#include "file_fmts.h"
#endif

// use libZPlay namespace
using namespace libZPlay;

//lint -esym(534, libZPlay::ZPlay::SetEchoParam, SleepEx, libZPlay::ZPlay::Close)
//lint -esym(526, CreateZPlay)  Symbol not defined
//lint -e755   global macro not referenced
//lint -e757   global declarator not referenced
//lint -e758   global enum not referenced
//lint -e768   global struct member not referenced
//lint -e769   global enumeration constant not referenced
//lint -e1716  Virtual member function not referenced

static TStreamInfo pInfo;

//***************************************************************************
//  play mp3 (and other) file via zplay library
//***************************************************************************
int zplay_audio_file(char const * mp3_file)
{
   if(mp3_file == NULL) {
      return 1 ;
   }
#ifndef  STAND_ALONE
   char fpath[260] ;
   sprintf(fpath, "%s\\%s", base_path, mp3_file) ;
   mp3_file = fpath ;
#endif   
   // create class instance
   ZPlay *player = CreateZPlay();

   // chek if we have class instance
   if(player == 0)
   {
      // printf("Error: Can't create class instance !\nPress key to exit.\n");
      return 2;
   }

   // get library version
   int ver = player->GetVersion();
   // check if we have version 1.90 and above ( 1.90 if PF (pattent free) version, 2.00 version is full version
   if(ver < 190)
   {
      // printf("Error: Need library version 2.00 and above !\nPress key to exit.\r\n");
      player->Release();
      return 3;
   }

#ifdef  STAND_ALONE
   // display version info
   printf("libZPlay v.%i.%02i\n", ver / 100, ver % 100);
#endif   

   // check input arguments
      // open file using input argument as filename
   if(player->OpenFile(mp3_file, sfAutodetect) == 0)
   {
      // printf("OpenFile Error: %s\n", player->GetError());
      player->Release();
      return 4;
   }

   // program some echo efffect
   TEchoEffect effect[2];

   effect[0].nLeftDelay = 1000;
   effect[0].nLeftEchoVolume = 20;
   effect[0].nLeftSrcVolume = 80;
   effect[0].nRightDelay = 500;
   effect[0].nRightEchoVolume = 20;
   effect[0].nRightSrcVolume = 80;

   effect[1].nLeftDelay = 300;
   effect[1].nLeftEchoVolume = 20;
   effect[1].nLeftSrcVolume = 0;
   effect[1].nRightDelay = 300;
   effect[1].nRightEchoVolume = 20;
   effect[1].nRightSrcVolume = 0;

   // set echo effects
   player->SetEchoParam(effect, 2);
      
   // get stream info
   player->GetStreamInfo(&pInfo);

   // display stream info
#ifdef  STAND_ALONE
   printf("\r\n%s %i Hz %s [%d], bitrate: %u, Channels: %i, Length: %02u:%02u:%02u:%02u\r\n\r\n",
       pInfo.Description,
       pInfo.SamplingRate,
      (pInfo.VBR != 0) ? "VBR" : "CBR",
       pInfo.VBR,
       pInfo.Bitrate,
       pInfo.ChannelNumber,
       pInfo.Length.hms.hour,
       pInfo.Length.hms.minute,
       pInfo.Length.hms.second,
       pInfo.Length.hms.millisecond);
#endif       

   // start playing
   // if(player->Play() == 0) {
   //    player->Release(); // delete ZPlay class
   //    return 5;
   // }
   //  this probably *should* include the minutes and hours as well,
   //  though I don't think anyone would want that for a timer program...
   // unsigned total_secs = (pInfo.Length.hms.second+1) * 1000 ;
   // SleepEx(total_secs, false);

   player->Close();  // close open stream
   player->Release(); // destroy class
   return 0;
}

#ifdef  STAND_ALONE
//***************************************************************************
void usage(void)
{
   puts("Usage: play_once mp3_file_name");
}

//***************************************************************************
#define  MAX_PATH_LEN   1024

//lint -esym(818, argv)  Pointer parameter could be declared as pointing to const
int main(int argc, char **argv)
{
   char mp3_file[MAX_PATH_LEN+1] = "" ;
   int idx ;
   for (idx=1; idx<argc; idx++) {
      strncpy(mp3_file, argv[idx], MAX_PATH_LEN);
      mp3_file[MAX_PATH_LEN] = 0 ;
   }
   
   if (mp3_file[0] == 0) {
      usage() ;
      return 1 ;
   }
   int result = zplay_audio_file(mp3_file);
   if (result != 0) {
      printf("Error %d playing %s\n", result, mp3_file);
   }

   return 0;
}

#else
//************************************************************************
int get_zplay_info(char *fname, char *mlstr)
{
   // printf("\r\n%s %i Hz %s, bitrate: %u  Channel: %i  Length: %02u:%02u:%02u:%02u\r\n\r\n",
   //     pInfo.Description,
   //     pInfo.SamplingRate,
   //    (pInfo.VBR != 0) ? "VBR" : "CBR",
   //     pInfo.Bitrate,
   //     pInfo.ChannelNumber,
   //     pInfo.Length.hms.hour,
   //     pInfo.Length.hms.minute,
   //     pInfo.Length.hms.second,
   //     pInfo.Length.hms.millisecond);
   
   int result = zplay_audio_file(fname);
   if (result != 0) {
      sprintf(mlstr, "%s: Error %d\n", fname, result);
   }
   else {
      uint play_msecs = pInfo.Length.hms.minute * 60 ;
      uint play_hsecs = pInfo.Length.hms.hour * 60 * 60 ;
      uint play_secs  = pInfo.Length.hms.second + play_msecs + play_hsecs ;
      total_ptime += play_secs ; //  total play time of *all* measured files
   
      if (pInfo.Length.hms.hour > 0) {
         if (pInfo.Length.hms.second > 30) {
            pInfo.Length.hms.minute++ ;
         }
         sprintf(tempstr, "%u Kbps %s, %3u:%02u hours", 
            (unsigned) pInfo.Bitrate,
           (pInfo.VBR != 0) ? "VBR" : "CBR",
            pInfo.Length.hms.hour,
            pInfo.Length.hms.minute) ;
      }
      else if (pInfo.Length.hms.minute > 0) {
         if (pInfo.Length.hms.millisecond > 30) {
            pInfo.Length.hms.second++ ;
         }
         sprintf(tempstr, "%u Kbps %s, %3u:%02u mins", 
            (unsigned) pInfo.Bitrate,
           (pInfo.VBR != 0) ? "VBR" : "CBR",
            pInfo.Length.hms.minute,
            pInfo.Length.hms.second) ;
      }
      else {
         sprintf(tempstr, "%u Kbps %s, %3u.%02u secs", 
            (unsigned) pInfo.Bitrate,
           (pInfo.VBR != 0) ? "VBR" : "CBR",
            pInfo.Length.hms.second,
            pInfo.Length.hms.millisecond) ;
         
      }
      // sprintf(tempstr, "%4u x %4u x %u colors", cols, rows, (1U << bpp)) ;
      sprintf(mlstr, "%-30s", tempstr) ;
   }
   return 0 ;
}

#endif

