//**********************************************************************************
//  Copyright (c) 2025 Derell Licht
//  media_list.cpp - list info about various media files
//**********************************************************************************

#define  MAX_EXT_SIZE   8

//lint -esym(759, total_ptime)  header declaration for symbol could be moved from header to module
extern double total_ptime ;

//lint -esym(552, base_len)   Symbol not accessed
//lint -esym(759, base_len)   header declaration for symbol could be moved from header to module
extern TCHAR base_path[MAX_FILE_LEN+1] ;
extern unsigned base_len ;  //  length of base_path

//************************************************************
//lint -esym(1401, ffdata::ft)  member not initialized by constructor
struct ffdata {
   uchar          attrib {};
   FILETIME       ft {};
   ULONGLONG      fsize {};      //equivalent to = 0;
   wchar_t        *filename {};  //or = nullptr;
   bool          dirflag {};
   // struct ffdata  *next ;  //  no longer needed, with vector
} ;
typedef ffdata ffdata_t, *ffdata_p ;

//  ext_lookup.cpp
int print_media_info(ffdata_t const * const fptr);

