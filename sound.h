#ifdef WITH_SOUND

#include <stdio.h>
#include <stdlib.h>

#include <sys/soundcard.h>
#include <simple.h>
#define RIFF 1179011410     /* little endian value for ASCII-encoded 'RIFF' */
#define WAVE 1163280727     /* little endian value for ASCII-encoded 'WAVE' */
#define FMT 544501094       /* little endian value for ASCII-encoded 'fmt' */
#define DATA 1635017060     /* little endian value for ASCII-encoded 'data' */

#endif

typedef struct {
    unsigned int chunkID;    /* ASCII: 'RIFF' */
    unsigned int chunkSize;    /* filelength - 8 */
    unsigned int format;    /* ASCII: 'WAVE' */

    unsigned int subChunk1ID;   /* ASCII: 'fmt ' */
    unsigned int subChunk1Size; /* length of sub chunk, 16 for PCM*/
    unsigned short int audioFormat;     /* should be 1 for PCM */
    unsigned short int numberOfChannels;    /* 1 Mono, 2 Stereo */
    unsigned int sampleRate;    /* sample frequency */
    unsigned int byteRate;  /* sampleRate * numberOfChannels * bitsPerSample/8 */

    unsigned short int blockAlign;    /* numberOfChannels * bitsPerSample/8 */
    unsigned short int bitsPerSample; /* 8, 16 bit */ 

    unsigned int subChunk2ID;   /* ASCII: 'data' */
    unsigned int subChunk2Size; /* size of the sample data */
} waveHeader;

typedef struct {
    waveHeader* header;
    unsigned char* sampleData;
} waveFile;


int loadWave(const char* filename, waveFile* waveFile);
void playSound(waveFile* sound);
void killsound();
int initSound(int bitsPerSample, int numberOfChannels, int samplingRate);

