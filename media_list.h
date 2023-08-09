//**********************************************************************************
//  Copyright (c) 2023 Daniel D. Miller                       
//  media_list.cpp - list info about various media files
//                                                                 
//  Written by:   Daniel D. Miller
//  
//**********************************************************************************

//lint -esym(759, total_ptime)  header declaration for symbol could be moved from header to module
extern double total_ptime ;

//************************************************************
typedef struct ffdata {
   uchar          attrib ;
   FILETIME       ft ;
   ULONGLONG      fsize ;
   char           *filename ;
   uchar          dirflag ;
   struct ffdata  *next ;
} ffdata_t ;

//  ext_lookup.cpp
int print_media_info(ffdata_t const * const fptr);

