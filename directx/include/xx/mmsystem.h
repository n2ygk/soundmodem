
#ifndef _mmsystem_h
#define _mmsystem_h

/* Multimedia header file 
    -- not really part of DirectX, but not included in the Win32 API headers.
*/

/* Result code definition */
typedef UINT MMRESULT;
typedef DWORD MCIERROR;

/* Various error values */
#define MAXERRORLENGTH		256	/* Maximum length of error message */
#define MMSYSERR_NOERROR	0
#define TIMERR_NOERROR		MMSYSERR_NOERROR
#define TIMERR_NOCANDO		97

/* Wave out device handle */
typedef HANDLE HWAVEOUT;

/* MCI device handle */
typedef UINT MCIDEVICEID;

/* Have the system choose a wave device */
#define WAVE_MAPPER	((UINT)-1)

/* Specify the type of wave event callback */
#define CALLBACK_FUNCTION	0x00030000
#define CALLBACK_EVENT		0x00050000

/* Messages sent to the waveOut callback function */
#define WOM_DONE	0x3BD

/* The wave buffer header used by waveOut functions */
typedef struct WAVEHDR {
	LPSTR	lpData;
	DWORD	dwBufferLength;
	DWORD	dwBytesRecorded;
	DWORD	dwUser;
	DWORD	dwFlags;
	DWORD	dwLoops;
	struct WAVEHDR *lpNext;
	DWORD	reserved;
} WAVEHDR;

/* WAVEHDR.dwFlags */
#define WHDR_DONE	0x00000001
#define WHDR_BEGINLOOP	0x00000004
#define WHDR_ENDLOOP	0x00000008
#define WHDR_INQUEUE	0x00000010

/* Structure used in querying the capabilities of a wave device */
typedef struct {
	WORD	wMid;
	WORD	wPid;
	UINT	vDriverVersion;
	CHAR	szPname[32];
	DWORD	dwFormats;
	WORD	wReserved1;
	DWORD	dwSupport;
} WAVEOUTCAPS;

/* The only kind of sound data we handle here */
#define WAVE_FORMAT_PCM	1

/* Old wave format structure */
typedef struct {
	WORD    wFormatTag;
	WORD    nChannels;
	DWORD   nSamplesPerSec;
	DWORD   nAvgBytesPerSec;
	WORD    nBlockAlign;
} WAVEFORMAT, *LPWAVEFORMAT;

/* PCM wave format structure */
typedef struct {
	WAVEFORMAT  wf;
	WORD        wBitsPerSample;
} PCMWAVEFORMAT;

/* Wave format structure (used by dsound.h) */
typedef struct {
	WORD  wFormatTag;
	WORD  nChannels;
	DWORD nSamplesPerSec;
	DWORD nAvgBytesPerSec;
	WORD  nBlockAlign;
	WORD  wBitsPerSample;
	WORD  cbSize;
} WAVEFORMATEX, *LPWAVEFORMATEX;

/* Constants used as arguments and flags to mciSendCommand() */

/* CD device type */
#define MCI_STRING_OFFSET	512
#define MCI_DEVTYPE_CD_AUDIO	(MCI_STRING_OFFSET + 4)

/* MCI commands */
#define MCI_OPEN	0x0803
#define MCI_CLOSE	0x0804
#define MCI_PLAY	0x0806
#define MCI_STOP	0x0808
#define MCI_PAUSE	0x0809
#define MCI_SET		0x080D
#define MCI_STATUS	0x0814
#define MCI_RESUME	0x0855

/* Flags for MCI commands */
#define MCI_NOTIFY			0x00000001
#define MCI_WAIT			0x00000002
#define MCI_FROM			0x00000004
#define MCI_TO				0x00000008
#define MCI_TRACK			0x00000010

/* Flags for MCI Play command */
#define MCI_OPEN_SHAREABLE		0x00000100
#define MCI_OPEN_ELEMENT		0x00000200
#define MCI_OPEN_TYPE_ID		0x00001000
#define MCI_OPEN_TYPE			0x00002000

/* Flags for MCI Status command */
#define MCI_STATUS_ITEM			0x00000100
#define MCI_STATUS_LENGTH		0x00000001
#define MCI_STATUS_POSITION		0x00000002
#define MCI_STATUS_NUMBER_OF_TRACKS	0x00000003
#define MCI_STATUS_MODE			0x00000004
#define MCI_STATUS_MEDIA_PRESENT	0x00000005
#define MCI_STATUS_TIME_FORMAT		0x00000006
#define MCI_STATUS_READY		0x00000007
#define MCI_STATUS_CURRENT_TRACK	0x00000008

/* Flags for MCI Set command */
#define MCI_SET_DOOR_OPEN		0x00000100
#define MCI_SET_DOOR_CLOSED		0x00000200
#define MCI_SET_TIME_FORMAT		0x00000400

/* MCI device status flags */
#define MCI_MODE_NOT_READY	(MCI_STRING_OFFSET + 12)
#define MCI_MODE_STOP		(MCI_STRING_OFFSET + 13)
#define MCI_MODE_PLAY		(MCI_STRING_OFFSET + 14)
#define MCI_MODE_RECORD		(MCI_STRING_OFFSET + 15)
#define MCI_MODE_SEEK		(MCI_STRING_OFFSET + 16)
#define MCI_MODE_PAUSE		(MCI_STRING_OFFSET + 17)
#define MCI_MODE_OPEN		(MCI_STRING_OFFSET + 18)


/* Constants used to specify MCI time formats */
#define MCI_FORMAT_MILLISECONDS	0
#define MCI_FORMAT_HMS		1
#define MCI_FORMAT_MSF		2
#define MCI_FORMAT_FRAMES	3
#define MCI_FORMAT_BYTES	8
#define MCI_FORMAT_SAMPLES	9
#define MCI_FORMAT_TMSF		10

#define MCI_MSF_MINUTE(msf)	((BYTE)(msf))
#define MCI_MSF_SECOND(msf)	((BYTE)(((WORD)(msf)) >> 8))
#define MCI_MSF_FRAME(msf)	((BYTE)((msf)>>16))

#define MCI_MAKE_MSF(m, s, f)	\
	((DWORD)(((BYTE)(m)|((WORD)(s)<<8))|(((DWORD)(BYTE)(f))<<16)))


/* Structures passed as arguments to mciSendCommand() */
typedef struct {
	DWORD  dwCallback;
	MCIDEVICEID wDeviceID;
	LPCSTR lpstrDeviceType;
	LPCSTR lpstrElementName;
	LPCSTR lpstrAlias;
} MCI_OPEN_PARMS;

typedef struct {
	DWORD dwCallback;
	DWORD dwTimeFormat;
	DWORD dwAudio;
} MCI_SET_PARMS;

typedef struct {
	DWORD dwCallback;
	DWORD dwFrom;
	DWORD dwTo;
} MCI_PLAY_PARMS;

typedef struct {
	DWORD dwCallback;
	DWORD dwReturn;
	DWORD dwItem;
	DWORD dwTrack;
} MCI_STATUS_PARMS;

/* Convert a string to a 4 byte multimedia code */
#ifndef mmioFOURCC
#define mmioFOURCC(c0, c1, c2, c3)	\
	((DWORD)(c0)|((DWORD)(c1)<<8)|((DWORD)(c2)<<16)|((DWORD)(c3)<<24))
#endif

/* timer callback function prototype */
typedef void (CALLBACK TIMECALLBACK)(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);

/* What type of timer to set */
#define TIME_ONESHOT    0x0000   /* program timer for single event */
#define TIME_PERIODIC   0x0001   /* program for continuous periodic event */


/* Multimedia timer function declarations */
extern MMRESULT WINAPI timeSetEvent(UINT uDelay, UINT uResolution,
			TIMECALLBACK *fptc, DWORD dwUser, UINT fuEvent);
extern MMRESULT WINAPI timeKillEvent(UINT uTimerID);
extern MMRESULT WINAPI timeBeginPeriod(UINT uPeriod);
extern MMRESULT WINAPI timeEndPeriod(UINT uPeriod);

/* The waveOut* function declarations */
extern MMRESULT WINAPI waveOutOpen(HWAVEOUT *phwo, UINT uDeviceID,
	WAVEFORMATEX *pwfx, DWORD dwCallback, DWORD dwInstance, DWORD fdwOpen);
extern MMRESULT WINAPI waveOutClose(HWAVEOUT hwo);
extern MMRESULT WINAPI waveOutPrepareHeader(HWAVEOUT hwo, WAVEHDR *pwh, UINT cbwh);
extern MMRESULT WINAPI waveOutUnprepareHeader(HWAVEOUT hwo, WAVEHDR *pwh, UINT cbwh);
extern MMRESULT WINAPI waveOutWrite(HWAVEOUT hwo, WAVEHDR *pwh, UINT cbwh);
extern MMRESULT WINAPI waveOutPause(HWAVEOUT hwo);
extern MMRESULT WINAPI waveOutRestart(HWAVEOUT hwo);
extern MMRESULT WINAPI waveOutReset(HWAVEOUT hwo);
extern MMRESULT WINAPI waveOutBreakLoop(HWAVEOUT hwo);
#define waveOutGetDevCaps	waveOutGetDevCapsA
extern MMRESULT WINAPI waveOutGetDevCapsA(UINT uDeviceID, WAVEOUTCAPS *pwoc, UINT cbwoc);
#define waveOutGetErrorText	waveOutGetErrorTextA
extern MMRESULT WINAPI waveOutGetErrorTextA(MMRESULT mmrError, LPSTR pszText, UINT cchText);

/* The MCI command interface */
#define mciSendCommand		mciSendCommandA
extern MCIERROR WINAPI mciSendCommandA(MCIDEVICEID mciId, UINT uMsg, DWORD dwParam1, DWORD dwParam2);
#define mciGetErrorString	mciGetErrorStringA
extern int WINAPI mciGetErrorStringA(DWORD hErr, LPCSTR lpBuf, DWORD dwLen);

#endif /* _mmsystem_h */
