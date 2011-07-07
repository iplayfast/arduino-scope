#ifdef WITH_SOUND
#include "sound.h"

int loadWave(const char* filename, waveFile* waveFile)
{
    FILE* file;
    /* allocate space for the wave header */
    waveFile->header = (waveHeader *) malloc(sizeof(waveHeader));
    /* make sure the file is there and open it read-only (binary) */
    if ((file = fopen(filename, "rb")) == NULL) {
        printf("File not found : %s\n", filename);
        return 0;
    }
    /* read the wave header */
    if (!fread(waveFile->header, sizeof(waveHeader), 1, file)) {
        printf("Error reading file!\n");
        return 0;
    }
    /* check if it is a riff wave file */
    if (waveFile->header->chunkID != RIFF ||
        waveFile->header->format != WAVE ||
        waveFile->header->subChunk1ID != FMT ||
        waveFile->header->subChunk2ID != DATA) {
        printf("Soundfile %s not in wave format!\n", filename);
        return 0;
    }
    /* we can only handle uncompressed, PCM encoded waves! */
    if (waveFile->header->audioFormat != 1) {
        printf("Soundfile not PCM encoded!\n");
        return 0;
    }
    /* we can only handle up to two channels (stereo) */
    if (waveFile->header->numberOfChannels > 2) {
        printf("Soundfile has more than 2 channels!\n");
        return 0;
    }
    waveFile->sampleData = (unsigned char *) malloc(waveFile->header->subChunk2Size);
    fseek(file, sizeof(waveHeader), SEEK_SET);
    if (!fread(waveFile->sampleData, waveFile->header->subChunk2Size, 1,
        file)) {
        printf("Error loading file!\n");
        return 0;
    }
    return 1;
}

pa_simple *s;

void playSound(waveFile* sound) {
  int error;
 pa_simple_write (s,
		sound->sampleData,
		sound->header->subChunk2Size,
		&error);
}

void killsound()
{
 if (s)
   pa_simple_free(s);
}
//	printf("%d %d\n",x,70 + lines[0].GetSample(x));

int initSound(int bitsPerSample, int numberOfChannels, int samplingRate)
{
 pa_sample_spec ss;

 ss.format = PA_SAMPLE_S16NE;
 ss.channels = 2;
 ss.rate = 44100;

 s = pa_simple_new(NULL,               // Use the default server.
                   "Fooapp",           // Our application's name.
                   PA_STREAM_PLAYBACK,
                   NULL,               // Use the default device.
                   "Music",            // Description of our stream.
                   &ss,                // Our sample format.
                   NULL,               // Use default channel map
                   NULL,               // Use default buffering attributes.
                   NULL               // Ignore error code.
                   );

  
    return 0;
}





#else

int loadWave(const char* filename, waveFile* waveFile)
{
    return 1;
};

void playSound(waveFile* sound) {};
void killsound() {};
int initSound(int bitsPerSample, int numberOfChannels, int samplingRate)
{
   return 0;
};

#endif
