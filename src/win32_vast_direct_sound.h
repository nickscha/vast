#ifndef WIN32_VAST_DIRECT_SOUND_H
#define WIN32_VAST_DIRECT_SOUND_H

/* #############################################################################
 * # [SECTION] Dependencies
 * #############################################################################
 */
#include "vast_types.h"     /* Basic types (u8, u32, ...)    */
#include "win32_vast_api.h" /* Win32 API calls               */

/* #############################################################################
 * # [SECTION] Direct Sound Declarations
 * #############################################################################
 */
#define DS_OK 0x00000000L
#define DSERR_BUFFERLOST 0x88780096L
#define DS_FAILED(hr) (((i32)(hr)) < 0)
#define DS_SUCCEEDED(hr) (((i32)(hr)) >= 0)
#define WAVE_FORMAT_PCM 1
#define DSSCL_NORMAL 1
#define DSSCL_PRIORITY 2
#define DSSCL_EXCLUSIVE 3
#define DSSCL_WRITEPRIMARY 4
#define DSBCAPS_PRIMARYBUFFER 0x00000001u
#define DSBCAPS_STATIC 0x00000002u
#define DSBCAPS_LOCHARDWARE 0x00000004u
#define DSBCAPS_LOCSOFTWARE 0x00000008u
#define DSBCAPS_CTRL3D 0x00000010u
#define DSBCAPS_CTRLFREQUENCY 0x00000020u
#define DSBCAPS_CTRLPAN 0x00000040u
#define DSBCAPS_CTRLVOLUME 0x00000080u
#define DSBCAPS_CTRLPOSITIONNOTIFY 0x00000100u
#define DSBCAPS_CTRLFX 0x00000200u
#define DSBCAPS_CTRLDEFAULT 0x000000E0u
#define DSBCAPS_GLOBALFOCUS 0x00008000u
#define DSBCAPS_GETCURRENTPOSITION2 0x00010000u
#define DSBCAPS_MUTE3DATMAXDISTANCE 0x00020000u
#define DSBCAPS_LOCDEFER 0x00040000u
#define DSBPLAY_LOOPING 0x00000001u
#define DSBPLAY_LOCHARDWARE 0x00000002u
#define DSBPLAY_LOCSOFTWARE 0x00000004u
#define DSBPLAY_TERMINATEBY_TIME 0x00000008u
#define DSBPLAY_TERMINATEBY_DISTANCE 0x00000010u
#define DSBPLAY_TERMINATEBY_PRIORITY 0x00000020u
#define DSBLOCK_FROMWRITECURSOR 0x00000001u
#define DSBLOCK_ENTIREBUFFER 0x00000002u
#define DSBSTATUS_PLAYING 0x00000001u
#define DSBSTATUS_BUFFERLOST 0x00000002u
#define DSBSTATUS_LOOPING 0x00000004u
#define DSBSTATUS_LOCHARDWARE 0x00000008u
#define DSBSTATUS_LOCSOFTWARE 0x00000010u
#define DSBSTATUS_TERMINATED 0x00000020u

typedef struct GUID
{
    u32 Data1;
    u16 Data2;
    u16 Data3;
    u8 Data4[8];

} GUID;

typedef struct WAVEFORMATEX
{
    u16 wFormatTag;
    u16 nChannels;
    u32 nSamplesPerSec;
    u32 nAvgBytesPerSec;
    u16 nBlockAlign;
    u16 wBitsPerSample;
    u16 cbSize;

} WAVEFORMATEX;

typedef struct DSBUFFERDESC
{
    u32 dwSize;
    u32 dwFlags;
    u32 dwBufferBytes;
    u32 dwReserved;
    WAVEFORMATEX *lpwfxFormat;
    GUID guid3DAlgorithm;

} DSBUFFERDESC;

typedef struct IDirectSound IDirectSound;
typedef struct IDirectSoundBuffer IDirectSoundBuffer;

typedef struct IDirectSoundBufferVtbl
{
    /* IUnknown */
    i32 (*QueryInterface)(IDirectSoundBuffer *This, GUID *riid, void **ppvObj);
    u32 (*AddRef)(IDirectSoundBuffer *This);
    u32 (*Release)(IDirectSoundBuffer *This);

    /* IDirectSoundBuffer */
    i32 (*GetCaps)(IDirectSoundBuffer *This, void *pDSBufferCaps);
    i32 (*GetCurrentPosition)(IDirectSoundBuffer *This, u32 *pdwCurrentPlayCursor, u32 *pdwCurrentWriteCursor);
    i32 (*GetFormat)(IDirectSoundBuffer *This, WAVEFORMATEX *pwfxFormat, u32 dwSizeAllocated, u32 *pdwSizeWritten);
    i32 (*GetVolume)(IDirectSoundBuffer *This, i32 *plVolume);
    i32 (*GetPan)(IDirectSoundBuffer *This, i32 *plPan);
    i32 (*GetFrequency)(IDirectSoundBuffer *This, u32 *pdwFrequency);
    i32 (*GetStatus)(IDirectSoundBuffer *This, u32 *pdwStatus);
    i32 (*Initialize)(IDirectSoundBuffer *This, IDirectSound *pDirectSound, DSBUFFERDESC *pcDSBufferDesc);
    i32 (*Lock)(IDirectSoundBuffer *This, u32 dwOffset, u32 dwBytes, void **ppvAudioPtr1, u32 *pdwAudioBytes1, void **ppvAudioPtr2, u32 *pdwAudioBytes2, u32 dwFlags);
    i32 (*Play)(IDirectSoundBuffer *This, u32 dwReserved1, u32 dwPriority, u32 dwFlags);
    i32 (*SetCurrentPosition)(IDirectSoundBuffer *This, u32 dwNewPosition);
    i32 (*SetFormat)(IDirectSoundBuffer *This, WAVEFORMATEX *pcfxFormat);
    i32 (*SetVolume)(IDirectSoundBuffer *This, i32 lVolume);
    i32 (*SetPan)(IDirectSoundBuffer *This, i32 lPan);
    i32 (*SetFrequency)(IDirectSoundBuffer *This, u32 dwFrequency);
    i32 (*Stop)(IDirectSoundBuffer *This);
    i32 (*Unlock)(IDirectSoundBuffer *This, void *pvAudioPtr1, u32 dwAudioBytes1, void *pvAudioPtr2, u32 dwAudioBytes2);
    i32 (*Restore)(IDirectSoundBuffer *This);

} IDirectSoundBufferVtbl;

typedef struct IDirectSoundBuffer
{
    IDirectSoundBufferVtbl *lpVtbl;

} IDirectSoundBuffer;

typedef struct IDirectSoundVtbl
{
    /* IUnknown */
    i32 (*QueryInterface)(IDirectSound *This, GUID *riid, void **ppvObj);
    u32 (*AddRef)(IDirectSound *This);
    u32 (*Release)(IDirectSound *This);

    /* IDirectSound */
    i32 (*CreateSoundBuffer)(IDirectSound *This, DSBUFFERDESC *pcDSBufferDesc, IDirectSoundBuffer **ppDSBuffer, void *pUnkOuter);
    i32 (*GetCaps)(IDirectSound *This, void *pDSCaps);
    i32 (*DuplicateSoundBuffer)(IDirectSound *This, IDirectSoundBuffer *pDSBufferOriginal, IDirectSoundBuffer **ppDSBufferDuplicate);
    i32 (*SetCooperativeLevel)(IDirectSound *This, void *hwnd, u32 dwLevel);
    i32 (*Compact)(IDirectSound *This);
    i32 (*GetSpeakerConfig)(IDirectSound *This, u32 *pdwSpeakerConfig);
    i32 (*SetSpeakerConfig)(IDirectSound *This, u32 dwSpeakerConfig);
    i32 (*Initialize)(IDirectSound *This, GUID *pcGuidDevice);

} IDirectSoundVtbl;

struct IDirectSound
{
    IDirectSoundVtbl *lpVtbl;
};

/* #############################################################################
 * # [SECTION] Direct Sound Loader
 * #############################################################################
 */
typedef i32 (*direct_sound_create)(GUID *pcGuidDevice, IDirectSound **ppDS, void *pUnkOuter);

static direct_sound_create DirectSoundCreate = 0;

VAST_API u8 direct_sound_load(void)
{
    void *dsound_lib = LoadLibraryA("dsound.dll");

    if (!dsound_lib)
    {
        return 0;
    }

    DirectSoundCreate = (direct_sound_create)GetProcAddress(dsound_lib, "DirectSoundCreate");

    if (!DirectSoundCreate)
    {
        return 0;
    }

    return 1;
}

#endif /* WIN32_VAST_DIRECT_SOUND_H */