//**********************************************************************************
//  Copyright (c) 1998-2025 Derell Licht                       
//  common.cpp - common library functions
//**********************************************************************************

#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#include "common.h"

//lint -esym(755, LOOP_FOREVER, MAX_EXT_SIZE, MAXLINE)
//lint -esym(756, u16, u32, ul2uc_t)
//lint -esym(759, base_path, convert_to_commas)
//lint -esym(768, ul2uc_u::uc, ul2uc_u::ul, ul2uc_u::us)

//  name of drive+path without filenames
TCHAR base_path[MAX_FILE_LEN+1] ;
unsigned base_len ;  //  length of base_path

//lint -esym(714, get_system_message)
//lint -esym(759, get_system_message)
//lint -esym(765, get_system_message)
//*************************************************************
//  each subsequent call to this function overwrites 
//  the previous report.
//*************************************************************
TCHAR *get_system_message(void)
{
   static TCHAR msg[261] ;
   int slen ;

   LPVOID lpMsgBuf;
   FormatMessage( 
      FORMAT_MESSAGE_ALLOCATE_BUFFER | 
      FORMAT_MESSAGE_FROM_SYSTEM | 
      FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL,
      GetLastError(),
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
      (LPTSTR) &lpMsgBuf,
      0, 0);
   // Process any inserts in lpMsgBuf.
   // ...
   // Display the string.
   _tcsncpy(msg, (TCHAR *) lpMsgBuf, 260) ;

   // Free the buffer.
   LocalFree( lpMsgBuf );

   //  trim the newline off the message before copying it...
   slen = _tcslen(msg) ;
   if (msg[slen-1] == 10  ||  msg[slen-1] == 10) {
      msg[slen-1] = 0 ;
   }

   return msg;
}

TCHAR *get_system_message(DWORD errcode)
{
   static TCHAR msg[261] ;
   int slen ;

   LPVOID lpMsgBuf;
   FormatMessage( 
      FORMAT_MESSAGE_ALLOCATE_BUFFER | 
      FORMAT_MESSAGE_FROM_SYSTEM | 
      FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL,
      errcode,
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
      (LPTSTR) &lpMsgBuf,
      0, 0);
   // Process any inserts in lpMsgBuf.
   // ...
   // Display the string.
   _tcsncpy(msg, (TCHAR *) lpMsgBuf, 260) ;

   // Free the buffer.
   LocalFree( lpMsgBuf );

   //  trim the newline off the message before copying it...
   slen = _tcslen(msg) ;
   if (msg[slen-1] == 10  ||  msg[slen-1] == 10) {
      msg[slen-1] = 0 ;
   }

   return msg;
}

//********************************************************************
//  On Windows platform, try to redefine printf/fprintf
//  so we can output code to a debug window.
//  Also, shadow syslog() within OutputDebugStringA()
//  Note: printf() remapping was unreliable,
//  but syslog worked great.
//********************************************************************
//lint -esym(714, syslog)
//lint -esym(759, syslog)
//lint -esym(765, syslog)
int syslog(const TCHAR *fmt, ...)
{
   TCHAR consoleBuffer[3000] ;
   va_list al; //lint !e522

//lint -esym(526, __builtin_va_start)
//lint -esym(628, __builtin_va_start)
   va_start(al, fmt);   //lint !e1055 !e530
   _vstprintf(consoleBuffer, fmt, al);   //lint !e64
   // if (common_logging_enabled)
   //    fprintf(cmlogfd, "%s", consoleBuffer) ;
   OutputDebugString(consoleBuffer) ;
   va_end(al);
   return 1;
}

//****************************************************************************
//lint -esym(714, ascii2unicode)
//lint -esym(759, ascii2unicode)
//lint -esym(765, ascii2unicode)
WCHAR *ascii2unicode(char const * const AsciiStr)
{
   uint AsciiLen = strlen(AsciiStr) ;
   static WCHAR UnicodeStr[MAX_UNICODE_LEN+1] ;
   if (AsciiLen > MAX_UNICODE_LEN)
       AsciiLen = MAX_UNICODE_LEN ;
   MultiByteToWideChar(CP_ACP, 0, AsciiStr, -1, UnicodeStr, AsciiLen+1);
   return UnicodeStr;
}

//****************************************************************************
//lint -esym(714, unicode2ascii)
//lint -esym(759, unicode2ascii)
//lint -esym(765, unicode2ascii)
char *unicode2ascii(WCHAR const * const UnicodeStr)
{
   static char AsciiStr[MAX_UNICODE_LEN+1] ;
   WideCharToMultiByte(CP_ACP, 0, UnicodeStr, -1, AsciiStr, MAX_UNICODE_LEN, NULL, NULL);
   return AsciiStr ;
}

//**********************************************************************
//lint -esym(714, IsCharNum)
//lint -esym(759, IsCharNum)
//lint -esym(765, IsCharNum)
bool IsCharNum(char inchr)
{
   // if (inchr >= '0'  &&  inchr <= '9')
   if (inchr >= '0'  &&  inchr <= '9')
      return true ;
   return false;
}

//**********************************************************************
//lint -esym(714, strip_newlines)
//lint -esym(759, strip_newlines)
//lint -esym(765, strip_newlines)
void strip_newlines(char *rstr)
{
   int slen = (int) strlen(rstr) ;
   while (1) {
      if (slen == 0)
         break;
      if (*(rstr+slen-1) == '\n'  ||  *(rstr+slen-1) == '\r') {
         slen-- ;
         *(rstr+slen) = 0 ;
      } else {
         break;
      }
   }
}

//**********************************************************************
//  NOTE: if string does *not* begin with a space or tab,
//        this function fails to return proper data!!
//**********************************************************************
//lint -esym(714, next_field)
//lint -esym(759, next_field)
//lint -esym(765, next_field)
char *next_field(char *q)
{
   while (*q != ' '  &&  *q != HTAB  &&  *q != 0)
      q++ ; //  walk past non-spaces
   while (*q == ' '  ||  *q == HTAB)
      q++ ; //  walk past all spaces
   return q;
}

//**********************************************************************
//  Modify this to build entire string and print once.
//  This command has several forms:
//
//  - the basic form has too many arguments!!
//    bfr - data buffer to display
//    bytes - number of bytes (of bfr) to display
//    addr  - base address to display at beginning of line.
//            This helps with repeated calls to this function.
//    mode  - 0=output to printf, 1=output to syslog
//
//  - The other two forms take only buffer and length args,
//    and implicitly print to either printf or syslog.
//**********************************************************************
//lint -esym(714, hex_dump)
//lint -esym(759, hex_dump)
//lint -esym(765, hex_dump)
int hex_dump(u8 *bfr, int bytes, unsigned addr)
{
   int j, len ;
   char tail[40] ;
   char pstr[81] ;
   static bool hex_dump_active = false ;
   if (hex_dump_active)
      return 0;
   hex_dump_active = true ;

   tail[0] = 0 ;
   int idx = 0 ;
   int plen = 0 ;
   while (1) {
      int leftovers = bytes - idx ;
      if (leftovers > 16)
          leftovers = 16 ;

      plen = wsprintfA(pstr, "%05X:  ", addr+idx) ;  //lint !e737
      len = 0 ;
      for (j=0; j<leftovers; j++) {
         u8 chr = bfr[idx+j] ;
         plen += wsprintfA(&pstr[plen], "%02X ", chr) ;
         if (chr < 32) {
            len += wsprintfA(tail+len, ".") ;
         } else if (chr < 127) {
            len += wsprintfA(tail+len, "%c", chr) ;
         } else {
            len += wsprintfA(tail+len, "?") ;
         }
      }
      //  last, print fill spaces
      for (; j<16; j++) {
         plen += wsprintfA(&pstr[plen], "   ") ;
         len += wsprintfA(tail+len, " ") ;
      }

      // strcat(pstr, " | ") ;
      // strcat(pstr, tail) ;
      // strcat(pstr, " |") ;
      printf("%s | %s |\n", pstr, tail) ;
      // printf("%s\n", pstr) ;
      // syslog("%s\n", pstr) ;

      idx += leftovers ;
      if (idx >= bytes)
         break;
   }
   hex_dump_active = false ;
   return 0;
}  //lint !e818 

//**************************************************************************
int hex_dump(u8 *bfr, int bytes)
{
   return hex_dump(bfr, bytes, 0) ;
}

//*****************************************************************************
// ULLONG_MAX = 18,446,744,073,709,551,615
//*****************************************************************************
#undef  USE_LLU

char *convert_to_commas(ULONGLONG uli, char *outstr)
{  //lint !e1066
   uint slen, j, inIdx ;
   char *strptr ;
   char temp_ull_str[MAX_ULL_COMMA_LEN+1] ;
   static char local_ull_str[MAX_ULL_COMMA_LEN+1] ;
   if (outstr == NULL) {
       outstr = local_ull_str ;
   }

   // sprintf(temp_ull_str, "%"PRIu64"", uli);
#ifdef  USE_LLU
   sprintf(temp_ull_str, "%llu", uli);
#else   
   sprintf(temp_ull_str, "%I64u", uli);
#endif   
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
   strcpy(outstr, strptr) ;   // NOLINT
   return outstr ;
}

