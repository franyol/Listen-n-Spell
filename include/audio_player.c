#include "audio_player.h"
#include "pbPlots.h"
#include "supportLib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// Global variables definitions

// Functions

/* This routine will be called by the PortAudio engine when audio is needed.
** It may be called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
static int recordCallback( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData )
{
    paData *data = (paData*)userData;
    const SAMPLE *rptr = (const SAMPLE*)inputBuffer;
    SAMPLE *wptr = &data->samples[data->frameIndex * NUM_CHANNELS];
    long framesToCalc;
    long i;
    int finished;
    unsigned long framesLeft = data->maxFrameIndex - data->frameIndex;

    (void) outputBuffer; /* Prevent unused variable warnings. */
    (void) timeInfo;
    (void) statusFlags;
    (void) userData;

    if( framesLeft < framesPerBuffer )
    {
        framesToCalc = framesLeft;
        finished = paComplete;
    }
    else
    {
        framesToCalc = framesPerBuffer;
        finished = paContinue;
    }

    if( inputBuffer == NULL )
    {
        for( i=0; i<framesToCalc; i++ )
        {
            *wptr++ = SAMPLE_SILENCE;  /* left */
            if( NUM_CHANNELS == 2 ) *wptr++ = SAMPLE_SILENCE;  /* right */
        }
    }
    else
    {
        for( i=0; i<framesToCalc; i++ )
        {
            *wptr++ = *rptr++;  /* left */
            if( NUM_CHANNELS == 2 ) *wptr++ = *rptr++;  /* right */
        }
    }
    data->frameIndex += framesToCalc;
    return finished;
}

/* This routine will be called by the PortAudio engine when audio is needed.
** It may be called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
static int playCallback( const void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData )
{
    paData *data = (paData*)userData;
    SAMPLE *rptr = &data->samples[data->frameIndex * NUM_CHANNELS];
    SAMPLE *wptr = (SAMPLE*)outputBuffer;
    unsigned int i;
    int finished;
    unsigned int framesLeft = data->maxFrameIndex - data->frameIndex;

    (void) inputBuffer; /* Prevent unused variable warnings. */
    (void) timeInfo;
    (void) statusFlags;
    (void) userData;

    if( framesLeft < framesPerBuffer )
    {
        /* final buffer... */
        for( i=0; i<framesLeft; i++ )
        {
            *wptr++ = *rptr++;  /* left */
            if( NUM_CHANNELS == 2 ) *wptr++ = *rptr++;  /* right */
        }
        for( ; i<framesPerBuffer; i++ )
        {
            *wptr++ = 0;  /* left */
            if( NUM_CHANNELS == 2 ) *wptr++ = 0;  /* right */
        }
        data->frameIndex += framesLeft;
        finished = paComplete;
    }
    else
    {
        for( i=0; i<framesPerBuffer; i++ )
        {
            *wptr++ = *rptr++;  /* left */
            if( NUM_CHANNELS == 2 ) *wptr++ = *rptr++;  /* right */
        }
        data->frameIndex += framesPerBuffer;
        finished = paContinue;
    }
    return finished;
}

paData load_audio_from_file (char* file_name)
    /**
     * @brief Loads data from raw .pcm file or .mp3 files
     * 
     * @return paData struct 
     * frameIndex equals 0 if function failed
     */
{
    paData empty;
    paData data;
    empty.maxFrameIndex = 0;
    size_t s_len = strlen (file_name);
    char* extension = file_name + s_len - 5;

    if (strstr (extension, ".mp3") != NULL) {
        char* ffmpeg_args[] = {
            "ffmpeg", "-y", "-i",
            file_name, "-acodec", "pcm_f32le",
            "-f", "f32le", "-ac",
            "1", "temp.pcm", NULL
        };
        int child_status;
        pid_t child_pid;

        /* Convert the file in a child process */
        child_pid = fork();
        if (child_pid == 0) {
            execvp ("ffmpeg", ffmpeg_args);
            /*  The execvp function returns only if an error occurs.    */
            fprintf (stderr, "an error occurred in execvp\n");
            abort();
        }

        wait (&child_status);
        if (!WIFEXITED (child_status)) {
            printf ("Error converting %s\n", file_name);
            return empty;
        }

        file_name = "data/temp.pcm";
    }
    else if (strstr (extension, ".pcm") != NULL);
    else fprintf (stderr, "The file %s has an unsupported extension\n", file_name);

    /* Get file length and max frame index */
    FILE  *fid;
    int i;
    fid = fopen(file_name, "r");
    fseek(fid, 0, SEEK_END);
    long fsize = ftell(fid);
    fseek(fid, 0, SEEK_SET);

    data.maxFrameIndex = fsize/sizeof(SAMPLE);
    data.samples = (SAMPLE *) malloc( fsize );
    
    if( data.samples == NULL )
    {
        fprintf(stderr, "Could not allocate record array.\n");
        return empty;
    }

    /* Fill samples with zeros */
    for( i=0; i<data.maxFrameIndex; i++ ) data.samples[i] = 0;

    if( fid == NULL )
    {
        fprintf(stderr, "Could not open file.");
    }
    else
    {
        fread( data.samples, NUM_CHANNELS * sizeof(SAMPLE), data.maxFrameIndex, fid );
        fclose( fid );
        fprintf(stdout, "Data loaded\n");
    }

    return data;
}

paData load_audio_from_samples (SAMPLE *samples, int length)
    /**
     * @brief Loads data from a sample array (float)
     * 
     * length is the number of samples in the array
     * 
     * @return paData struct
     */
{
    int i;
    paData response;
    
    response.samples = (SAMPLE *) malloc( sizeof (SAMPLE) * length );

    response.frameIndex = 0;
    response.maxFrameIndex = length;
    for (i = 0; i<length; i++) {
        response.samples[i] = samples[i];        
    }

    return response;
}

int play_audio (paData *data)
    /**
     * @brief Plays the audio saved in the data
     * 
     * returns when finished 
     * 
     */
{
    PaStreamParameters  inputParameters,
                        outputParameters;
    PaStream*           stream;
    PaError             err = paNoError;

    inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
    if (inputParameters.device == paNoDevice) {
        fprintf(stderr,"Error: No default input device.\n");
        return 0;
    }
    inputParameters.channelCount = NUM_CHANNELS;
    inputParameters.sampleFormat = PA_SAMPLE_TYPE;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    /* Playback recorded data.  -------------------------------------------- */
    /* Copyright (c) 1999-2000 Ross Bencina and Phil Burk */
    data->frameIndex = 0;

    outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
    if (outputParameters.device == paNoDevice) {
        fprintf(stderr,"Error: No default output device.\n");
        goto done;
    }
    outputParameters.channelCount = NUM_CHANNELS;
    outputParameters.sampleFormat =  PA_SAMPLE_TYPE;
    outputParameters.suggestedLatency = 0.008707;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    printf("\n=== Now playing back. ===\n"); fflush(stdout);
    err = Pa_OpenStream(
              &stream,
              NULL, /* no input */
              &outputParameters,
              SAMPLE_RATE,
              FRAMES_PER_BUFFER,
              paClipOff,      /* we won't output out of range samples so don't bother clipping them */
              playCallback,
              data );
    if( err != paNoError ) goto done;

    if( stream )
    {
        err = Pa_StartStream( stream );
        if( err != paNoError ) goto done;

        printf("Waiting for playback to finish.\n"); fflush(stdout);

        while( ( err = Pa_IsStreamActive( stream ) ) == 1 ) Pa_Sleep(100);
        if( err < 0 ) goto done;

        err = Pa_CloseStream( stream );
        if( err != paNoError ) goto done;

        printf("Done.\n"); fflush(stdout);
    }

done:
    if( err != paNoError )
    {
        fprintf( stderr, "An error occurred while using the portaudio stream\n" );
        fprintf( stderr, "Error number: %d\n", err );
        fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
        err = 1;          /* Always return 0 or 1, but no other return codes. */
    }
    return err;
}

paData record_audio (int seconds)
    /**
     * @brief Records audio from device
     * 
     */
{
    PaStreamParameters  inputParameters,
                        outputParameters;
    PaStream*           stream;
    PaError             err = paNoError;
    paData              data, empty;

    /* If an error occurs, return 0 index */
    empty.maxFrameIndex = 0;

    inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
    if (inputParameters.device == paNoDevice) {
        fprintf(stderr,"Error: No default input device.\n");
        return empty;
    }
    inputParameters.channelCount = NUM_CHANNELS;
    inputParameters.sampleFormat = PA_SAMPLE_TYPE;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    data.maxFrameIndex = seconds * SAMPLE_RATE; /* Record for a few seconds. */
    data.frameIndex = 0;

    data.samples = (SAMPLE *) malloc (data.maxFrameIndex*sizeof(SAMPLE));

    /* Record some audio. -------------------------------------------- */
    err = Pa_OpenStream(
              &stream,
              &inputParameters,
              NULL,                  /* &outputParameters, */
              SAMPLE_RATE,
              FRAMES_PER_BUFFER,
              paClipOff,      /* we won't output out of range samples so don't bother clipping them */
              recordCallback,
              &data );
    if( err != paNoError ) goto done;

    err = Pa_StartStream( stream );
    if( err != paNoError ) goto done;
    printf("\n=== Now recording!! Please speak into the microphone. ===\n"); fflush(stdout);

    while( ( err = Pa_IsStreamActive( stream ) ) == 1 )
    {
        Pa_Sleep(1000);
        printf("index = %d\n", data.frameIndex ); fflush(stdout);
    }
    if( err < 0 ) goto done;

    err = Pa_CloseStream( stream );
    if( err != paNoError ) goto done;

done:
    if( err != paNoError )
    {
        fprintf( stderr, "An error occurred while using the portaudio stream\n" );
        fprintf( stderr, "Error number: %d\n", err );
        fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
        return empty;
    }
    return data;
}

paData trim_audio (paData *audio, int seconds, int from_index, float sensibility, int *trim_index)
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
{
    SAMPLE* trimmed;
    register SAMPLE actual_sample;
    int to_index, max_index;
    int i;
    float max = 0;
    float silence_trigger_secs = 0.5;
    int silence_trigger_samples = silence_trigger_secs * SAMPLE_RATE; 
    int silence_acc = 0;
    int peaks_acc = 0;

    int audio_length = audio->maxFrameIndex;

    paData trimmed_audio;

    to_index = (int) (SAMPLE_RATE * seconds + from_index);

    /* Just a temporal variable we still don't know the output's length
       but we know it can't be greater than the source audio */
    trimmed = (SAMPLE*) malloc (audio_length*sizeof(SAMPLE));

    if (sensibility < 0)
        sensibility = SENSIBILITY_DEFAULT;
    if (to_index > audio_length) {
        max_index = to_index = audio_length;
    }
    else {
        max_index = to_index+(SAMPLE_RATE*5);
        if (max_index > audio_length) {
            max_index = audio_length;
        }
    }

    for (i=from_index; i<max_index; i++) {
        actual_sample = audio->samples[i];
        trimmed[i-from_index] = actual_sample;
        if (i<to_index) {
            if (max < actual_sample)
                max = actual_sample;
        }
        else {
            if (fabs(actual_sample) < (max/sensibility)) {
                peaks_acc = 0;
                if (++silence_acc > silence_trigger_samples) {
                    *trim_index = i;
                    break;
                }
            }
            else 
                if (++peaks_acc > PEAK_SUPPRESS)
                    silence_acc = 0;
        }
    }
    
    /* load_audio_from samples creates a copy from the data
       and saves them in the required length, so go ahead and
       free trimmed samples afterwards */
    trimmed_audio = load_audio_from_samples (trimmed, i - from_index);
    free (trimmed);

    return trimmed_audio;
}


void plot_audio (paData* data, char* filename, int height, int width)
    /**
     * @brief Creates a png image from paData
     * 
     */
{
    int i;
    double *x;
    double *y;

    x = (double*) malloc (data->maxFrameIndex*sizeof(double));
    y = (double*) malloc (data->maxFrameIndex*sizeof(double));
    for (i=0; i<data->maxFrameIndex; i++) {
        x[i] = (double) i/SAMPLE_RATE;
        y[i] = (double) data->samples[i];
    }

    ScatterPlotSeries *series = GetDefaultScatterPlotSeriesSettings();
    series->xs = x;
    series->xsLength = data->maxFrameIndex;
    series->ys = y;
    series->ysLength = data->maxFrameIndex;
    series->linearInterpolation = true;
    series->lineType = L"solid";
    series->lineTypeLength = wcslen(series->lineType);

    ScatterPlotSettings *settings = GetDefaultScatterPlotSettings();
    settings->width = width;
    settings->height = height;
    settings->autoBoundaries = true;
    settings->autoPadding = true;
    ScatterPlotSeries *s [] = {series};
    settings->scatterPlotSeries = s;
    settings->scatterPlotSeriesLength = 1;

    RGBABitmapImageReference *canvasReference = CreateRGBABitmapImageReference();
    DrawScatterPlotFromSettings(canvasReference, settings);

    size_t length;
    DrawScatterPlot(canvasReference, width, height, x, data->maxFrameIndex, y, data->maxFrameIndex);
    double *pngdata = ConvertToPNG(&length, canvasReference->image);
    WriteToFile(pngdata, length, filename);
	DeleteImage(canvasReference->image);

    return;
}