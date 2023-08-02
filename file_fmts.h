//*****************************************************************
//  Copyright (c) 1998-2023 Daniel D. Miller                       
//  file_fmts.h - media-file parsing functions
//*****************************************************************

int get_jpeg_info(char *fname, char *mlstr);
int get_gif_info(char *fname, char *mlstr);
int get_bmp_info(char *fname, char *mlstr);
int get_png_info(char *fname, char *mlstr);
int get_ico_cur_info(char *fname, char *mlstr);
int get_ani_info(char *fname, char *mlstr);
int get_sid_info(char *fname, char *mlstr);
int get_webp_info(char *fname, char *mlstr);
int get_mi_info(char *fname, char *mlstr);

