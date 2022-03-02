#ifndef IMAGE_UTIL_H
#define IMAGE_UTIL_H

#ifdef  __cplusplus
extern "C"
{
#endif

int IMG_Unnit();
int IMG_Init(const char *filename, int w, int h);
int IMG_SavePng(const char *filename, unsigned char *data, int w, int h, int ws, int hs);

int img_write_jpeg_file(const char* jpeg_file, unsigned char* rgb_buffer, int width, int height, int quality);

#ifdef  __cplusplus
}
#endif

#endif // IMAGE_UTIL_H
