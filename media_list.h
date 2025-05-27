//**********************************************************************************
//  Copyright (c) 2025 Derell Licht
//  media_list.cpp - list info about various media files
//**********************************************************************************

//lint -esym(759, total_ptime)  header declaration for symbol could be moved from header to module
extern double total_ptime ;

//************************************************************
typedef struct ffdata {
   uchar          attrib ;
   FILETIME       ft ;
   ULONGLONG      fsize ;
   TCHAR          *filename ;
   uchar          dirflag ;
   struct ffdata  *next ;
} ffdata_t ;

//  ext_lookup.cpp
int print_media_info(ffdata_t const * const fptr);

