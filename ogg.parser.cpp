//***************************************************************************************
//  Copyright (c) 2023 Daniel D. Miller                       
//  ogg.parser.cpp - ogg file parser
//***************************************************************************************
//***************************************************************************************
//  To compile this module as a stand-alone utility:
//  g++ -Wall -s -O3 -Wno-write-strings -DDO_CONSOLE ogg.parser.cpp -o oggparse.exe
//***************************************************************************************

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
// #include <unistd.h>  //  read()

#ifdef DO_CONSOLE

#define  LOOP_FOREVER   true

typedef  unsigned long long   u64 ;
typedef  unsigned char        uchar ;
typedef  unsigned long        ulong ;
typedef  unsigned int         uint ;
typedef  unsigned char        u8  ;
typedef  unsigned short       u16 ;
typedef  unsigned int         u32 ;

// One_Step_Forward.ogg
// 0000 0000 4f 67 67 53 00 02 00 00  00 00 00 00 00 00 34 56  OggS.........4V
// 0000 0010 00 00 00 00 00 00 fe 46  b3 66 01 1e 01 76 6f 72  ......¦F¦fvor
// 0000 0020 62 69 73 00 00 00 00 02  44 ac 00 00 ff ff ff ff  bis....D¼..
// 0000 0030 00 f4 01 00 ff ff ff ff  b8 01 4f 67 67 53 00 00  .(.    +OggS..
// 0000 0040 00 00 00 00 00 00 00 00  34 56 00 00 01 00 00 00  ........4V.....
// 0000 0050 53 8f 3a 4e 12 a4 ff ff  ff ff ff ff ff ff ff ff  SÅ:Nñ
// 0000 0060 ff ff ff ff ff ff 3c 03  76 6f 72 62 69 73 1d 00        <vorbis.
// 0000 0070 00 00 58 69 70 68 2e 4f  72 67 20 6c 69 62 56 6f  ..Xiph.Org libVo
// 0000 0080 72 62 69 73 20 49 20 32  30 30 32 30 37 31 37 06  rbis I 20020717
// 0000 0090 00 00 00 0e 00 00 00 61  72 74 69 73 74 3d 53 61  ......artist=Sa
// 0000 00a0 6d 73 75 6e 67 0d 00 00  00 61 6c 62 75 6d 3d 53  msung....album=S
// 0000 00b0 61 6d 73 75 6e 67 16 00  00 00 74 69 74 6c 65 3d  amsung...title=
// 0000 00c0 4f 6e 65 20 53 74 65 70  20 46 6f 72 77 61 72 64  One Step Forward
// 0000 00d0 0e 00 00 00 67 65 6e 72  65 3d 52 69 6e 67 74 6f  ...genre=Ringto
// 0000 00e0 6e 65 0f 00 00 00 63 6f  6d 6d 65 6e 74 3d 53 61  ne...comment=Sa
// 0000 00f0 6d 73 75 6e 67 11 00 00  00 41 4e 44 52 4f 49 44  msung...ANDROID
// 0000 0100 5f 4c 4f 4f 50 3d 74 72  75 65 01 05 76 6f 72 62  _LOOP=truevorb
// 0000 0110 69 73 29 42 43 56 01 00  08 00 00 80 22 4c 20 c3  is)BCV...Ç"L +
// 0000 0120 80 d0 90 55 00 00 10 00  00 80 a8 36 14 6b a9 b1  Ç-ÉU....Ç¿6k¬¦
// 0000 0130 d6 1a 63 a1 28 46 d4 62  6a 31 c6 18 63 e3 2c 46  +cí(F+bj1¦cp,F

//   Flags: 0x01: unset = fresh packet
//                  set = continued packet
//          0x02:   set = first page of logical bitstream (bos)
//          0x04:   set = last page of logical bitstream (eos)

typedef struct _ogg_header_s
{
    u32 Signature;
    u8 Version;
    u8 Flags;
    u64 GranulePosition;
    u32 SerialNumber;
    u32 SequenceNumber;
    u32 Checksum;
    u8 TotalSegments;
} __attribute__ ((packed)) _ogg_header_t ;
typedef _ogg_header_t *_ogg_header_p ;

#define  OGG_HEADER_SIZE   sizeof(_ogg_header_t)

static unsigned seek_byte = 0 ;   //  file offset to next frame

//*********************************************************************
typedef union ul2uc_u {
   unsigned       ul ;
   unsigned short us[2] ;
   unsigned char  uc[4] ;
} ul2uc_t;

#else
#include "ndir32.h"
#endif

//  this buffer needs to be large enough to completely read one frame.
//  An lseek() will be used to point to the next frame.
#define  RD_BFR_SZ   (4 * 1024)
static u8 rd_bfr[RD_BFR_SZ] ;

// static unsigned file_offset = 0 ;

static _ogg_header_t *mp = NULL;

//***************************************************************************************
//  This function is used to find the FIRST frame in the file;
//  subsequent frames are found by calculating frame length.
//***************************************************************************************
static bool read_ogg_segment(u8 *rbfr, unsigned rlen)
{
   mp = (_ogg_header_p) rbfr ;
   if (* rbfr    == 'O'  &&
       *(rbfr+1) == 'g'  &&
       *(rbfr+2) == 'g'  &&
       *(rbfr+3) == 'S') {
       
       return true;
   }
   return false;
}

//***************************************************************************************
static unsigned sum_bytes(u8 *rbfr, u8 num_bytes)
{
   unsigned sum = 0 ;
   unsigned idx ;
   for (idx=0; idx<num_bytes; idx++) {
      u8 x = *(rbfr+idx);
      sum += (unsigned) x;
   }
   // puts("");
   // printf("sum of %u bytes: %u\n", num_bytes, sum);
   return sum ;
}

//***************************************************************************************
//  note that this function is *not* optimized for performance.
//  It does a new lseek() to locate each frame, and ogg files tend to have
//  MANY frames.  It would be more efficient to buffer large chunks of
//  data and work thru the buffers, but that would make for more complex
//  code, partially because of allowing for frames (especially headers)
//  overlapping end of buffer.
//***************************************************************************************
static int read_ogg_file(char *fname)
{
   //  open file and start reading
   int hdl = open(fname, O_BINARY | O_RDONLY) ;
   if (hdl < 0) {
      perror(fname) ;
      return -(int)errno;
   }
   int result = 0 ;
   int ogg_offset ;           //  file offset to current frame (negative on error)
   
   //********************************************************************
   //  each pass through this loop, processes the current frame,
   //  and then seeks to the next frame.
   //********************************************************************
   while (LOOP_FOREVER) {
      int rdbytes = read(hdl, rd_bfr, RD_BFR_SZ) ;
      if (rdbytes <= 0) {
         // printf("rdbytes=%d\n", rdbytes) ;
         //  rdbytes == 0  indicates EOF
         if (rdbytes < 0) {
            printf("offset %u: %s\n", seek_byte, strerror(errno)) ;
            result = -(int)errno;
         }
         else {
            printf("file size: %u bytes\n", seek_byte) ;
         }
         break;
      }
      //********************************************************************
      //  on first pass, skip ID3 block if present
      //  
      //  06/29/20 Note: it is quite possible for ID3 header block to be
      //  larger than RD_BFR_SZ !!  In this case, we currently just fail.
      //  We probably should just do a new read at end of ID3 block size.
      //********************************************************************
      //  scan current buffer for ogg data signature
      bool success = read_ogg_segment(rd_bfr, rdbytes) ;
      // if (ogg_offset < 0) {
      if (!success) {
         printf("offset %u: OggS frame not found...\n", seek_byte) ;
         // return -EINVAL;
         result = -(int)EINVAL;
         break;
      }
      u8 *rbfr = &rd_bfr[OGG_HEADER_SIZE] ;
      unsigned SegmentSize = sum_bytes(rbfr, mp->TotalSegments);
      printf("offset %u: Flags %02X, Segments/size = %u/%u\n", 
        seek_byte, mp->Flags, mp->TotalSegments, SegmentSize);
      ogg_offset = OGG_HEADER_SIZE + mp->TotalSegments + SegmentSize ;
   
      //********************************************************************
      //  if you find an invalid frame, you probably found end-of-data.
      //  some files have ID data at end of file vs beginning.
      //********************************************************************
//       result = parse_ogg_frame(rd_bfr+ogg_offset, seek_byte) ;
//       if (result <= 0) {
//          if (result < 0) {
//             printf("parse_frame: %s\n", strerror(-result)) ;
//          } 
//          break;
//       }
//       //  on first pass, optionally skip past ID3 block
//       if (first_pass) {
//          seek_byte += (unsigned) ogg_offset ;
//       }
//       
      //  seek to next frame in file
      seek_byte += (unsigned) ogg_offset ;
      lseek(hdl, seek_byte, SEEK_SET) ;
   }  //  end of reads from ogg file
   close(hdl) ;
   return result ;
}

//***************************************************************************************
#ifndef DO_CONSOLE
static void clear_existing_list(void)
{
//    ogg_frame_p mkill ;
//    ogg_frame_p mtemp = frame_list; 
//    frame_list = 0 ;
//    while (mtemp != 0) {
//       mkill = mtemp ;
//       mtemp = mtemp->next ;
//       delete mkill ;
//    }
   
}
#endif

#ifdef DO_CONSOLE
//***************************************************************************************
void usage(void)
{
   puts("Usage: myogg ogg_filename") ;
}

//***************************************************************************************
int main(int argc, char **argv)
{
   char fname[1024] ;
   //  parse command line
   int j, result = 0;
   char *p ;
   fname[0] = 0 ;
   for (j=1; j<argc; j++) {
      p = argv[j] ;
      strncpy(fname, p, sizeof(fname)) ;
   }

   //  validate arguments
   if (fname[0] == 0) {
      usage() ;
      return 1;
   }

   printf("input: %s, struct size: %u\n", fname, OGG_HEADER_SIZE) ;
   result = read_ogg_file(fname) ;
   if (result < 0) {
      printf("result = %d, exiting...\n", result);
      return 1 ;
   }
   
   //***************************************************************
   //  now, see how many frames we found,
   //  and summarize the file information
   //***************************************************************
//    if (result >= 0) {
//       ogg_frame_p mtemp = frame_list; 
//       unsigned bitrate  = mtemp->bitrate ;
//       unsigned samprate = mtemp->sample_rate ;
//       unsigned vbr = 0 ;   //  if bitrate changes, set this to TRUE
// 
//       unsigned frame_count = 0 ;
//       double play_secs = 0.0 ;
//       for (mtemp = frame_list; mtemp != 0; mtemp = mtemp->next) {
//          frame_count++ ;
//          play_secs += mtemp->play_time ;
//          if (mtemp->bitrate != bitrate) 
//             vbr = 1 ;
//       }
//       char *verid_str[3] = { "1", "2", "2.5" } ;
//       char *layer_str[3] = { "I", "II", "III" } ;
//       printf("found %u frames\n", frame_count) ;
//       mtemp = frame_list; 
//    // unsigned mpeg_version ; //  1=1, 2=2, 3=2.5
//    // unsigned mpeg_layer ;
//       printf("mpegV%s [%u], layer %s [%u], chnl_mode: %u, mode_ext: %u\n", 
//          verid_str[mtemp->mpeg_version], mtemp->mpeg_version,
//          layer_str[mtemp->mpeg_layer], mtemp->mpeg_layer,
//          mtemp->channel_mode,
//          mtemp->mode_ext);
//       if (vbr) {
//          printf("bitrate=variable\n") ;
//       } else {
//          printf("bitrate=%uKbps\n", bitrate) ;
//       }
//       printf("sample rate=%u Hz\n", samprate) ;
// 
//       unsigned uplay_secs = (unsigned) play_secs ;
//       unsigned uplay_mins = uplay_secs / 60 ;
//       uplay_secs = uplay_secs % 60 ;
//       printf("total play time=%u:%02u; raw first header: 0x%08X, fname: %s\n", 
//          uplay_mins, uplay_secs, frame_list->raw, fname) ;
//    }
   
   return 0;
}
#else
//*********************************************************
int get_ogg_info(char *fname, char *mlstr)
{
   int result ;
   char fpath[260] ;

   clear_existing_list() ;
   //    431,340 44100 hz,  2.45 seconds     Bell1.wav
   sprintf(fpath, "%s\\%s", base_path, fname) ;

   result = read_ogg_file(fpath);
   if (result < 0) {
      sprintf(mlstr, "%-28s", "cannot parse file") ;
      return 0;
   }

   ogg_frame_p mtemp = frame_list; 
   unsigned bitrate  = mtemp->bitrate ;
   unsigned vbr = 0 ;   //  if bitrate changes, set this to TRUE

   double play_secs = 0.0 ;
   for (mtemp = frame_list; mtemp != 0; mtemp = mtemp->next) {
      // frame_count++ ;
      play_secs += mtemp->play_time ;
      if (mtemp->bitrate != bitrate) 
         vbr = 1 ;
   }

   total_ptime += play_secs ; //  total play time of *all* measured files
   unsigned uplay_secs = (unsigned) play_secs ;
   unsigned uplay_mins = uplay_secs / 60 ;
   uplay_secs = uplay_secs % 60 ;
   double dplay_secs = play_secs - (double) (uplay_mins * 60.0) ;
   if (vbr) {
      if (uplay_mins == 0) {
         sprintf(mlstr, "var Kbps,                 %5.2f sec    ", dplay_secs) ;
      }
      else {
         sprintf(mlstr, "var Kbps, %3u:%02u minutes [%5.2f sec]   ", uplay_mins, uplay_secs, dplay_secs) ;
      }
   } else {
      if (uplay_mins == 0) {
         sprintf(mlstr, "%3u Kbps,                 %5.2f sec    ", bitrate, dplay_secs) ;
      } 
      else {
         sprintf(mlstr, "%3u Kbps, %3u:%02u minutes [%5.2f sec]   ", bitrate, uplay_mins, uplay_secs, dplay_secs) ;
      }
   }
   return 0;
}

#endif
