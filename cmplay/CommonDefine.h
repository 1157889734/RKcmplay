#ifndef _COMMON_DEFINE_H_
#define _COMMON_DEFINE_H_

typedef enum _E_NET_TYPE
{
	TCP = 0,
	UDP
} E_NET_TYPE;

typedef enum _E_PLAY_STATE
{
	E_PLAY_STATE_PLAY = 1,
	E_PLAY_STATE_PAUSE,
	E_PLAY_STATE_STOP,
	E_PLAY_STATE_FAST_FORWARD,
	E_PLAY_STATE_FAST_REVERSE,
	E_PLAY_STATE_DRAG_POS,
} E_PLAY_STATE;


typedef enum _E_OPEN_MEDIA_STATE
{
	E_OPEN_MEDIA_SUCC = 0,     
	E_OPEN_MEDIA_LOGIN_FAI = -1, 
	E_OPEN_MEDIA_LOGIN_SUCC = 1, 
	E_OPEN_MEDIA_DESCRIBE_FAI = -2, 
	E_OPEN_MEDIA_SETUP_FAIL = -3,
	E_OPEN_MEDIA_PLAY_FAIL = -4,

}E_OPEN_MEDIA_STATE;

typedef struct _T_VIDEO_INFO 
{
	int iHeight;
	int iWidth;
	const char *pCodecName;
}T_VIDEO_INFO, *PT_VIDEO_INFO;

typedef struct _T_FRAME_INFO_
{
	unsigned char *pcFrame;
	int iFrameLen;
	timeval Timestamp;
	int iFrameRate;
}T_FRAME_INFO,*PT_FRAME_INFO;

typedef int (*PF_RTP_LOGIN_CALLBACK)(int resultCode, char* resultString,PT_VIDEO_INFO ptVideoInfo, void *pUserData);
typedef int (*PF_RTP_SET_VIDEO_DATA_CALLBACK)(PT_FRAME_INFO ptFrameInfo, PT_VIDEO_INFO ptVideoInfo,void *pUserData);


#endif
