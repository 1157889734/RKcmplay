#include "shm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <wayland-client.h>
#include <wayland-client-protocol.h>
#include <linux/input-event-codes.h>

#include <QGuiApplication>
#ifdef IMX8
#include "5.10.1/QtGui/qpa/qplatformnativeinterface.h"
#else
#include "5.14.2/QtGui/qpa/qplatformnativeinterface.h"
#endif

#include "mutex.h"

#define CODEC_ALIGN(x, a)   (((x)+(a)-1)&~((a)-1))

typedef struct  T_SHM_RECT_INFO
{
    int    w;
    int    h;
    int    size;
    int    fd;
    char   fd_filename[64];
    uchar *addr;
    rga_info_t rgasrc;
    rga_info_t rgadst;
    struct wl_surface *window_handle;
    struct wl_buffer* buffer;
    QWidget *pWidget;
    CMutexLock lock;
    T_SHM_RECT_INFO()
    {
        size = w = h = 0;
        fd = 0;
        addr = NULL;
        buffer = NULL;
        window_handle = NULL;
        pWidget = NULL;

        memset(fd_filename, 0, sizeof(fd_filename));
        memset(&rgasrc, 0, sizeof(rga_info_t));
        memset(&rgadst, 0, sizeof(rga_info_t));
    }
}*PT_SHM_RECT_INFO;

struct color_yuv {
    unsigned char y;
    unsigned char u;
    unsigned char v;
};

#define MAKE_YUV_601_Y(r, g, b) \
    ((( 66 * (r) + 129 * (g) +  25 * (b) + 128) >> 8) + 16)
#define MAKE_YUV_601_U(r, g, b) \
    (((-38 * (r) -  74 * (g) + 112 * (b) + 128) >> 8) + 128)
#define MAKE_YUV_601_V(r, g, b) \
    (((112 * (r) -  94 * (g) -  18 * (b) + 128) >> 8) + 128)

static QPlatformNativeInterface *native = NULL;
static struct wl_display *display_handle = NULL;

static struct wl_shm        *s_shm = NULL;

static PT_SHM_RECT_INFO create_rect_info(int w, int h)
{
    int stride = w;
    int size = w * h * 2;
    static int tmp_file_index = 0;
    char filename[64] = {0};
    sprintf(filename, "/tmp/hello-%d-wayland-XXXXXX", tmp_file_index++);

    if(s_shm == NULL)
    {
        return NULL;
    }

    int fd = mkstemp(filename);
    if(fd < 0)
    {
        printf("mkstemp failed : %m\n");
        return NULL;
    }

    if(fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | FD_CLOEXEC) < 0)
    {
        close(fd);
        return NULL;
    }
    if(ftruncate(fd, size) < 0)
    {
        close(fd);
        return NULL;
    }

    uchar *shm_data = (uchar*)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shm_data == MAP_FAILED)
    {
        printf("mmap failed : %m\n");
        close(fd);
        return NULL;
    }

    struct wl_shm_pool *pool = wl_shm_create_pool(s_shm, fd, size);
    struct wl_buffer   *buffer = wl_shm_pool_create_buffer(pool, 0, w, h, stride, WL_SHM_FORMAT_NV12);
    wl_shm_pool_destroy(pool);
    pool = NULL;

    PT_SHM_RECT_INFO pRectInfo = new T_SHM_RECT_INFO;
    pRectInfo->addr = shm_data;
    pRectInfo->buffer = buffer;
    pRectInfo->w = w;
    pRectInfo->h = h;
    pRectInfo->fd = fd;
    pRectInfo->size = size;
    strncpy(pRectInfo->fd_filename, filename, sizeof(pRectInfo->fd_filename));

    // rga dst buf set
    pRectInfo->rgasrc.fd = -1;
    pRectInfo->rgasrc.mmuFlag = 1;
    pRectInfo->rgasrc.virAddr = NULL;

    pRectInfo->rgadst.fd = -1;
    pRectInfo->rgadst.mmuFlag = 1;
    pRectInfo->rgadst.virAddr = shm_data;
    rga_set_rect(&pRectInfo->rgadst.rect, 0, 0, w, h, w, h, RK_FORMAT_YCbCr_420_SP);

    return pRectInfo;
}

static void handle_global(void *data, struct wl_registry *registry,
        uint32_t name, const char *interface, uint32_t version) {
    printf("handle_global : %s \n", interface);
    if (strcmp(interface, wl_shm_interface.name) == 0) {
        s_shm = (struct wl_shm *)wl_registry_bind(registry, name, &wl_shm_interface, 1);
    }
}

static void handle_global_remove(void *data, struct wl_registry *registry,
        uint32_t name) {
    // Who cares
}

static const struct wl_registry_listener registry_listener = {
    .global = handle_global,
    .global_remove = handle_global_remove,
};


int SHM_Init()
{
    printf("SHM_Init 0 \n");
    if(native == NULL)
        native = QGuiApplication::platformNativeInterface();
    if(display_handle == NULL)
        display_handle = (struct wl_display *)native->nativeResourceForWindow("display", NULL);

    printf("SHM_Init begin \n");
    struct wl_registry *registry = wl_display_get_registry(display_handle);
    wl_registry_add_listener(registry, &registry_listener, NULL);
    wl_display_roundtrip(display_handle);

    printf("SHM_Init ok \n");
    return 0;
}
int SHM_Uinit()
{
    return 0;
}

SHM_HANDLE SHM_AddRect(QWidget *pWnd)
{
    int w = pWnd->width();
    int h = pWnd->height();

    printf("SHM_AddRect pWnd w:%d h:%d \n", w, h);

    w = CODEC_ALIGN(w, 16);
    h = CODEC_ALIGN(h, 16);

    printf("SHM_AddRect 16 ALIGN w:%d h:%d \n", w, h);

    PT_SHM_RECT_INFO pShmRectInfo = create_rect_info(w, h);
//    QWidget *pWidget = new QWidget(pWnd);
//    pWidget->setGeometry(0,0,pWnd->width(), pWnd->height());
    pShmRectInfo->pWidget = pWnd;
    pShmRectInfo->pWidget->hide();
    //if(bo->format == DRM_FORMAT_NV12)
    {
        memset(pShmRectInfo->addr, 0x10, w * h);
        memset(pShmRectInfo->addr + w * h, 0x80, w * h * 0.5);
    }


    printf("add rect end \n");
    return pShmRectInfo;
}

int SHM_FreeRect(SHM_HANDLE hShmHandle)
{
    PT_SHM_RECT_INFO pShmRectInfo = (PT_SHM_RECT_INFO)hShmHandle;
    if(pShmRectInfo == NULL)
    {
        return -1;
    }
    pShmRectInfo->lock.Lock();
    wl_buffer_destroy(pShmRectInfo->buffer);
    munmap(pShmRectInfo->addr, pShmRectInfo->size);
    close(pShmRectInfo->fd);
    unlink(pShmRectInfo->fd_filename);
//    delete pShmRectInfo->pWidget;
    pShmRectInfo->lock.Unlock();
    delete pShmRectInfo;
    pShmRectInfo = NULL;

    return 0;
}

int SHM_AttchWnd(SHM_HANDLE hShmHandle)
{
    PT_SHM_RECT_INFO pShmRectInfo = (PT_SHM_RECT_INFO)hShmHandle;
    if(pShmRectInfo == NULL)
    {
        return -1;
    }

    if(pShmRectInfo->pWidget->isVisible())
    {
        return 0;
    }
    pShmRectInfo->pWidget->show();
    pShmRectInfo->pWidget->winId();
    if(pShmRectInfo->window_handle)
    {
        //SHM_DetchWnd(hShmHandle);

        return 0;
    }
    struct wl_surface *window_handle  = NULL;
    window_handle = (struct wl_surface *)native->nativeResourceForWindow("surface",
                                  pShmRectInfo->pWidget->windowHandle());
    if(window_handle == NULL)
    {

        printf("window_handle NULL \n");
        return 0;
    }
    pShmRectInfo->lock.Lock();
//    pShmRectInfo->pWidget->setUpdatesEnabled(false);
    wl_surface_attach(window_handle, pShmRectInfo->buffer, 0, 0);
    wl_surface_commit(window_handle);
//    wl_display_flush(display_handle);
    pShmRectInfo->window_handle = window_handle;
    pShmRectInfo->lock.Unlock();
    return 0;
}

int SHM_DetchWnd(SHM_HANDLE hShmHandle)
{
    PT_SHM_RECT_INFO pShmRectInfo = (PT_SHM_RECT_INFO)hShmHandle;
    if(pShmRectInfo == NULL)
    {
        return -1;
    }
    if(!pShmRectInfo->window_handle)
    {
        return -1;
    }
    if(!pShmRectInfo->pWidget->isVisible())
    {
        return 0;
    }

    pShmRectInfo->lock.Lock();
    pShmRectInfo->pWidget->hide();

    //pShmRectInfo->pWidget->setUpdatesEnabled(true);

    //wl_surface_attach(pShmRectInfo->window_handle, 0, 0, 0);
    //wl_surface_commit(pShmRectInfo->window_handle);
//    printf("wl_surface_DEttach %0x \n", pShmRectInfo->window_handle);
//    wl_display_flush(display_handle);
    pShmRectInfo->window_handle = NULL;
    pShmRectInfo->lock.Unlock();

    return 0;
}

int SHM_FillRect(SHM_HANDLE hShmHandle, uint32_t color)
{
    PT_SHM_RECT_INFO pShmRectInfo = (PT_SHM_RECT_INFO)hShmHandle;
    if(pShmRectInfo == NULL)
    {
        return -1;
    }
    if(!pShmRectInfo->window_handle)
    {
        return -1;
    }


    uint8_t rgba[4];
    memcpy(rgba, &color, 4);
    color_yuv yuv_value;
    yuv_value.y = MAKE_YUV_601_Y(rgba[3], rgba[2], rgba[1]);
    yuv_value.v = MAKE_YUV_601_U(rgba[3], rgba[2], rgba[1]);
    yuv_value.u = MAKE_YUV_601_V(rgba[3], rgba[2], rgba[1]);

//    printf("rgb:%d,%d,%d, yuv:%x,%x,%x\n", rgba[3], rgba[2], rgba[1],
//           yuv_value.y,yuv_value.u,yuv_value.v);


    pShmRectInfo->lock.Lock();
    memset(pShmRectInfo->addr, yuv_value.y, pShmRectInfo->w * pShmRectInfo->h);
    memset(pShmRectInfo->addr + pShmRectInfo->w * pShmRectInfo->h, yuv_value.u, pShmRectInfo->w * pShmRectInfo->h * 0.5);
    memset(pShmRectInfo->addr + (int)(pShmRectInfo->w * pShmRectInfo->h * 1.5), yuv_value.v, pShmRectInfo->w * pShmRectInfo->h * 0.5);
    wl_surface_attach(pShmRectInfo->window_handle, pShmRectInfo->buffer, 0, 0);
    wl_surface_damage (pShmRectInfo->window_handle, 0, 0,
        pShmRectInfo->w, pShmRectInfo->h);
    wl_surface_commit(pShmRectInfo->window_handle);
//    wl_display_flush(display_handle);
    pShmRectInfo->lock.Unlock();

    return 0;
}
int SHM_RkRgaBlit1(MppFrame tSrcMppFrame, rga_info_t &rgasrc, rga_info_t &rgadst)
{
    int ret = 0;
    int srcWidth,srcHeight,srcFormat, src_h_stride, src_v_stride;

    MppBuffer buffer  = NULL;
    RK_U8*    srcAddr = NULL;

    srcFormat = RK_FORMAT_YCbCr_420_SP;
    srcWidth  =   mpp_frame_get_width(tSrcMppFrame);
    srcHeight =   mpp_frame_get_height(tSrcMppFrame);
    src_h_stride = mpp_frame_get_hor_stride(tSrcMppFrame);
    src_v_stride = mpp_frame_get_ver_stride(tSrcMppFrame);
    //MppFrameFormat ft = mpp_frame_get_fmt(tSrcMppFrame);
    buffer = mpp_frame_get_buffer(tSrcMppFrame);
    srcAddr = (RK_U8 *)mpp_buffer_get_ptr(buffer);

    //printf("srcWidth:%d, srcHeight:%d, src_h_stride:%d,src_v_stride:%d,ft:%ld\n",
    //       srcWidth, srcHeight, src_h_stride, src_v_stride, ft);
    //printf("dstWidth:%d, dstHeight:%d, dstWidth:%d \n", dstWidth, dstHeight, dstWidth);


    rgasrc.fd = -1;
    rgasrc.mmuFlag = 1;
    rgasrc.virAddr = srcAddr;
    rga_set_rect(&rgasrc.rect, 0, 0, srcWidth, srcHeight, src_h_stride, src_v_stride, srcFormat);

    /*
    rgadst.fd = -1;
    rgadst.mmuFlag = 1;
    rgadst.virAddr = pShmRectInfo->addr;
    rga_set_rect(&rgadst.rect, 0, 0, dstWidth, dstHeight, dstWidth, dstHeight, dstFormat);
    */

    /********** call rga_Interface **********/
    RockchipRga &rkRga = RockchipRga::get();
    ret = rkRga.RkRgaBlit(&rgasrc, &rgadst, NULL);
    return ret;
}

int SHM_RkRgaBlit(MppFrame tSrcMppFrame, PT_SHM_RECT_INFO pShmRectInfo)
{
    rga_info_t &rgasrc = pShmRectInfo->rgasrc;
    rga_info_t &rgadst = pShmRectInfo->rgadst;

    int ret = 0;
    int srcWidth,srcHeight,srcFormat, src_h_stride, src_v_stride;

    /*
    int dstWidth,dstHeight,dstFormat;
    dstWidth  = pShmRectInfo->w;
    dstHeight = pShmRectInfo->h;
    dstFormat = RK_FORMAT_YCbCr_420_SP;
    */

    MppBuffer buffer  = NULL;
    RK_U8*    srcAddr = NULL;

    srcFormat = RK_FORMAT_YCbCr_420_SP;
    srcWidth  =   mpp_frame_get_width(tSrcMppFrame);
    srcHeight =   mpp_frame_get_height(tSrcMppFrame);
    src_h_stride = mpp_frame_get_hor_stride(tSrcMppFrame);
    src_v_stride = mpp_frame_get_ver_stride(tSrcMppFrame);
    //MppFrameFormat ft = mpp_frame_get_fmt(tSrcMppFrame);
    buffer = mpp_frame_get_buffer(tSrcMppFrame);
    srcAddr = (RK_U8 *)mpp_buffer_get_ptr(buffer);

    //printf("srcWidth:%d, srcHeight:%d, src_h_stride:%d,src_v_stride:%d,ft:%ld\n",
    //       srcWidth, srcHeight, src_h_stride, src_v_stride, ft);
    //printf("dstWidth:%d, dstHeight:%d, dstWidth:%d \n", dstWidth, dstHeight, dstWidth);


    rgasrc.fd = -1;
    rgasrc.mmuFlag = 1;
    rgasrc.virAddr = srcAddr;
    rga_set_rect(&rgasrc.rect, 0, 0, srcWidth, srcHeight, src_h_stride, src_v_stride, srcFormat);

    /*
    rgadst.fd = -1;
    rgadst.mmuFlag = 1;
    rgadst.virAddr = pShmRectInfo->addr;
    rga_set_rect(&rgadst.rect, 0, 0, dstWidth, dstHeight, dstWidth, dstHeight, dstFormat);
    */

    /********** call rga_Interface **********/
    RockchipRga &rkRga = RockchipRga::get();
    ret = rkRga.RkRgaBlit(&rgasrc, &rgadst, NULL);
    return ret;
}

int SHM_Display(SHM_HANDLE hPlaneHandle, MppFrame frame)
{
    PT_SHM_RECT_INFO pShmRectInfo = (PT_SHM_RECT_INFO)hPlaneHandle;
    if(pShmRectInfo == NULL)
    {
        printf("pShmRectInfo err \n");
        return -1;
    }
    int err = -1;
    pShmRectInfo->lock.Lock();
	err = SHM_RkRgaBlit(frame, pShmRectInfo);
    if(err < 0)
    {
        printf("rga_blit err \n");
        pShmRectInfo->lock.Unlock();
        return -1;
    }
    if(pShmRectInfo->window_handle == NULL)
    {
        pShmRectInfo->lock.Unlock();
        return -1;
    }
    if(pShmRectInfo->pWidget->isVisible() == 0)
    {
        pShmRectInfo->lock.Unlock();
        return -1;
    }
    if(pShmRectInfo->buffer == NULL)
    {
        pShmRectInfo->lock.Unlock();
        return -1;
    }
    //printf("*******pShmRectInfo->window_handle=%d------%d-----%d\n",pShmRectInfo->window_handle,pShmRectInfo->pWidget->isVisible(),__LINE__);
    wl_surface_attach(pShmRectInfo->window_handle, pShmRectInfo->buffer, 0, 0);
    wl_surface_damage (pShmRectInfo->window_handle, 0, 0,
        pShmRectInfo->w, pShmRectInfo->h);
    wl_surface_commit(pShmRectInfo->window_handle);
//    wl_display_flush(display_handle);
    pShmRectInfo->lock.Unlock();
//    printf("******%s---%d\n",__FUNCTION__,__LINE__);
    return 0;
}
