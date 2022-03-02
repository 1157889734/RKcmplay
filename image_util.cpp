#include "image_util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <math.h>
#include <sys/time.h>
#include <time.h>

// jpeg库头文件必须放到stdio.h后面
#include "jpeglib.h"
#include "jerror.h"

#include "png.h"

#include <QImage>

static unsigned long currentTimeMSec()
{
    struct  timeval start;
    gettimeofday(&start,NULL);
    return (start.tv_sec*1000 + start.tv_usec/1000);
}

#if 0
struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */

  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

/*
 * Here's the routine that will replace the standard error_exit method:
 */

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}

// 读取JPG图片数据，并解压到内存中，*rgb_buffer需要自行释放
int read_jpeg_file(const char* jpeg_file, unsigned char** rgb_buffer, int* size, int* width, int* height)
{
    struct jpeg_decompress_struct cinfo;
    struct my_error_mgr jerr;
    FILE* fp;

    JSAMPARRAY buffer;
    int row_stride = 0;
    unsigned char* tmp_buffer = NULL;
    int rgb_size;

    fp = fopen(jpeg_file, "rb");
    if (fp == NULL)
    {
        printf("open file %s failed.\n", jpeg_file);
        return -1;
    }

    cinfo.err = jpeg_std_error(&jerr.pub);
    //jerr.pub.error_exit = my_error_exit;

    if (setjmp(jerr.setjmp_buffer))
    {
        jpeg_destroy_decompress(&cinfo);
        fclose(fp);
        return -1;
    }

    jpeg_create_decompress(&cinfo);

    jpeg_stdio_src(&cinfo, fp);

    jpeg_read_header(&cinfo, TRUE);

    //cinfo.out_color_space = JCS_RGB; //JCS_YCbCr;  // 设置输出格式

    jpeg_start_decompress(&cinfo);

    row_stride = cinfo.output_width * cinfo.output_components;
    *width = cinfo.output_width;
    *height = cinfo.output_height;

    rgb_size = row_stride * cinfo.output_height; // 总大小
    *size = rgb_size;

    buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

    *rgb_buffer = (unsigned char *)malloc(sizeof(char) * rgb_size);    // 分配总内存

    printf("debug--:\nrgb_size: %d, size: %d w: %d h: %d row_stride: %d \n", rgb_size,
                cinfo.image_width*cinfo.image_height*3,
                cinfo.image_width,
                cinfo.image_height,
                row_stride);
    tmp_buffer = *rgb_buffer;
    while (cinfo.output_scanline < cinfo.output_height) // 解压每一行
    {
        jpeg_read_scanlines(&cinfo, buffer, 1);
        // 复制到内存
        memcpy(tmp_buffer, buffer[0], row_stride);
        tmp_buffer += row_stride;
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    fclose(fp);

    return 0;
}
#endif

int img_write_jpeg_file(const char* jpeg_file, unsigned char* rgb_buffer, int width, int height, int quality)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    int row_stride = 0;
    FILE* fp = NULL;
    JSAMPROW row_pointer[1];

    printf("time:%ld \n", currentTimeMSec());
    fp = fopen(jpeg_file, "wb");
    if (fp == NULL)
    {
        printf("open file %s failed.\n", jpeg_file);
        return -1;
    }

    cinfo.err = jpeg_std_error(&jerr);

    jpeg_create_compress(&cinfo);

    /* Step 2: specify data destination (eg, a file) */
    /* Note: steps 2 and 3 can be done in either order. */
    jpeg_stdio_dest(&cinfo, fp);

    /* Step 3: set parameters for compression */
    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    jpeg_set_defaults(&cinfo);

    jpeg_set_quality(&cinfo, quality, TRUE);

    /* Step 4: Start compressor */
    jpeg_start_compress(&cinfo, TRUE);
    row_stride = width * cinfo.input_components;

    while (cinfo.next_scanline < cinfo.image_height)
    {
        row_pointer[0] = &rgb_buffer[cinfo.next_scanline * row_stride];
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }
    /* Step 6: Finish compression */
    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    fclose(fp);
    printf("time:%ld \n", currentTimeMSec());

    return 0;
}

static png_structp png_ptr = NULL;//libpng的结构体
static png_infop  info_ptr = NULL;//libpng的信息
static FILE *fle = NULL;
int IMG_Init(const char *filename, int w, int h)
{
    fle = fopen(filename, "wb");
    if(fle == NULL)
    {
        return -1;
    }
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
    if(!png_ptr)
    {
        printf("png_create_write_struct error \n");
        goto INIT_DONE;
    }
    info_ptr = png_create_info_struct(png_ptr);
    if(!info_ptr)
    {
        printf("png_create_info_struct error \n");
        goto INIT_DONE;
    }
    //3: 设置错误返回点
    if (setjmp(png_jmpbuf(png_ptr))) {
        printf("error during init_io ...\n");
        goto INIT_DONE;
    }
    png_init_io(png_ptr, fle);

    //设置PNG文件头
    png_set_IHDR(png_ptr,info_ptr,
        w,h,//尺寸
        8,//颜色深度，也就是每个颜色成分占用位数（8表示8位红8位绿8位蓝，如果有透明通道则还会有8位不透明度）
        PNG_COLOR_TYPE_RGB,//颜色类型，PNG_COLOR_TYPE_RGB表示24位真彩深色，PNG_COLOR_TYPE_RGBA表示32位带透明通道真彩色
        PNG_INTERLACE_NONE,//不交错。PNG_INTERLACE_ADAM7表示这个PNG文件是交错格式。交错格式的PNG文件在网络传输的时候能以最快速度显示出图像的大致样子。
        PNG_COMPRESSION_TYPE_BASE,//压缩方式
        PNG_FILTER_TYPE_BASE);//这个不知道，总之填写PNG_FILTER_TYPE_BASE即可。
    png_set_packing(png_ptr);//设置打包信息
    png_write_info(png_ptr,info_ptr);//写入文件头
    return 0;
INIT_DONE:
    IMG_Unnit();
    return -1;
}
int IMG_Unnit()
{
    if(fle)
    {
        fclose(fle);
        fle = NULL;
    }
    if(png_ptr)
    {
        png_destroy_write_struct(&png_ptr,NULL);
        png_ptr = NULL;
    }
    if(info_ptr)
    {
        png_destroy_info_struct(png_ptr, &info_ptr);
        info_ptr = NULL;
    }
    return 0;
}

int IMG_SavePng(const char *filename, unsigned char *data, int w, int h, int ws, int hs)
{
    static int mmm = 0;
    if(mmm == 0)
    {
        mmm = 1;
        printf("w:%d, h:%d, ws:%d, hs:%d \n", w, h, ws, hs);
        for(int i = 0; i < h; i++)
        {
            png_write_row(png_ptr, (png_const_bytep)data+i*4*w);
        }

        //png_write_image(png_ptr, (png_bytepp)data);
        printf("111 w:%d, h:%d, ws:%d, hs:%d \n", w, h, ws, hs);

        png_write_end(png_ptr, info_ptr);
        printf("222 w:%d, h:%d, ws:%d, hs:%d \n", w, h, ws, hs);
    }

    return 0;

    QImage img(data, ws, hs, ws*3, QImage::Format_RGB888);
    if(img.save(filename, "png"))
    {
        return 0;
    }
    return -1;
}
