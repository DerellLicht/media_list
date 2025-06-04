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
typedef struct ffdata {
   uchar          attrib ;
   FILETIME       ft ;
   ULONGLONG      fsize ;
   TCHAR          *filename ;
   uchar          dirflag ;
#ifdef _lint   
   struct ffdata  *next ;
#endif   
} ffdata_t ;

//  ext_lookup.cpp
int print_media_info(ffdata_t const * const fptr);

