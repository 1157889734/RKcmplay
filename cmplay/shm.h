#ifndef SHM_H
#define SHM_H

#include <QWidget>

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "mutex.h"
#include "debugout/debug.h"

#define H264_CODE 0
#define H265_CODE 1

#include "rockchip/mpp_common.h"
#include "rockchip/mpp_frame.h"
#include "rockchip/rk_mpi.h"
#include "rga/RockchipRga.h"
#include "rga/RgaUtils.h"

typedef void* SHM_HANDLE;
int SHM_Init();
int SHM_Uinit();
SHM_HANDLE SHM_AddRect(QWidget *);

int SHM_AttchWnd(SHM_HANDLE hShmHandle);
int SHM_DetchWnd(SHM_HANDLE hShmHandle);
int SHM_FreeRect(SHM_HANDLE hShmHandle);

int SHM_FillRect(SHM_HANDLE hShmHandle, uint32_t color);
int SHM_Display(SHM_HANDLE hPlaneHandle, MppFrame frame);

int SHM_RkRgaBlit1(MppFrame tSrcMppFrame, rga_info_t &rgasrc, rga_info_t &rgadst);

#endif
