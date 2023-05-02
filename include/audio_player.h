#include "portaudio.h"
#include <stdlib.h>

#ifndef _AUDIO_PLAYER_H_
#define _AUDIO_PLAYER_H_

    // Constant definitions

    #define SAMPLE_RATE  (44100)
    #define FRAMES_PER_BUFFER (512)
    #define NUM_CHANNELS    (1)

    #define PA_SAMPLE_TYPE  paFloat32
    #define SAMPLE_SILENCE  (0.0f)
    #define PRINTF_S_FORMAT "%.8f"

    #define PEAK_SUPPRESS 10
    #define SENSIBILITY_DEFAULT 0.15

    // Type definitions

    typedef float SAMPLE;

    typedef struct
    {
        int          frameIndex;  /* Index into sample array. */
        int          maxFrameIndex;
        SAMPLE      *samples;
    }
    paData;

    // Global variables

    // Function prototypes

    void plot_audio (paData* data, char* filename, int height, int width);
    /**
     * @brief Creates a png image from paData
     * 
     */
    
    paData load_audio_from_file (char* file_name);
    /**
     * @brief Loads data from raw .pcm file or .mp3 files
     * 
     * @return paData struct
     */

    paData load_audio_from_samples (SAMPLE *samples, int length);
    /**
     * @brief Loads data from a sample array (float)
     * 
     * length is the number of samples in the array
     * 
     * @return paData struct
     */

    int play_audio (paData *data);
    /**
     * @brief Plays the audio saved in the data
     * 
     * returns when finished 
     * 
     */

    paData record_audio (int seconds);
    /**
     * @brief Records audio from device
     * 
     */

    paData trim_audio (paData *audio, int seconds, int from_index, float sensibility, int *trim_index);
    /**
     * @brief returns an trim from {audio} with minimum duration: {seconds}
     *        starting from {from_index}.
     * 
     * @param sensibility is a float number in the range from 0 to 1, set
     *        it to a negative number to set the default sensibility: 0.15
     * 
     * @return paData with the trimmed audio, also, the index of the original
     *          audio where the trim ocurred is written on {*trim_index}
     * 
     */

    /* This routine will be called by the PortAudio engine when audio is needed.
    ** It may be called at interrupt level on some machines so don't do anything
    ** that could mess up the system like calling malloc() or free().
    **
    ** Copyright (c) 1999-2000 Ross Bencina and Phil Burk
    */
    static int playCallback( const void *inputBuffer, void *outputBuffer,
                             unsigned long framesPerBuffer,
                             const PaStreamCallbackTimeInfo* timeInfo,
                             PaStreamCallbackFlags statusFlags,
                             void *userData );

    /* This routine will be called by the PortAudio engine when audio is needed.
    ** It may be called at interrupt level on some machines so don't do anything
    ** that could mess up the system like calling malloc() or free().
    **
    ** Copyright (c) 1999-2000 Ross Bencina and Phil Burk
    */
    static int recordCallback( const void *inputBuffer, void *outputBuffer,
                               unsigned long framesPerBuffer,
                               const PaStreamCallbackTimeInfo* timeInfo,
                               PaStreamCallbackFlags statusFlags,
                               void *userData );

#endif