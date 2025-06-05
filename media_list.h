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

//  DDM: the squiggly-braces after the field names, are non-static data member initializers.
// from n1ghtyunso  06/04/25 on reddit:
// There is even a benefit in using non-static data member initializers.
// For types with multiple constructors, the compiler will make sure that any member 
// not set in the constructor still gets the specified default value.
// When you do this in the default constructor instead, another constructor is still 
// able to leave some members uninitialized.

struct ffdata {
   uchar       attrib {};
   FILETIME    ft {};
   u64         fsize {};      //  equivalent to = 0;
   wchar_t     *filename {nullptr};
   bool        dirflag {};
   // struct ffdata  *next ;  //  no longer needed, with vector
} ;
typedef ffdata ffdata_t, *ffdata_p ;

//  ext_lookup.cpp
int print_media_info(ffdata_t const * const fptr);

