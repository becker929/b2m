/* 
PortAudio code adapted from official examples 
http://www.portaudio.com/docs/v19-doxydocs/group__examples__src.html

MIDI code adapted from libremidi tests
https://github.com/jcelerier/libremidi
*/

#include "midiout.h"
#include "portaudio.h"
#include "stft_nn.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <thread>

/* #define SAMPLE_RATE  (17932) // Test failure to open with this value. */
#define SAMPLE_RATE (44100)
#define FRAMES_PER_BUFFER (512)
// #define NUM_SECONDS 1
#define NUM_CHANNELS (2)
/* #define DITHER_FLAG     (paDitherOff) */
#define DITHER_FLAG (0) /**/
/** Set to 1 if you want to capture the recording to a file. */
#define WRITE_TO_FILE (0)

/* Select sample format. */
#if 1
#  define PA_SAMPLE_TYPE paFloat32
typedef float SAMPLE;
#  define SAMPLE_SILENCE (0.0f)
#  define PRINTF_S_FORMAT "%.8f"
#elif 1
#  define PA_SAMPLE_TYPE paInt16
typedef short SAMPLE;
#  define SAMPLE_SILENCE (0)
#  define PRINTF_S_FORMAT "%d"
#elif 0
#  define PA_SAMPLE_TYPE paInt8
typedef char SAMPLE;
#  define SAMPLE_SILENCE (0)
#  define PRINTF_S_FORMAT "%d"
#else
#  define PA_SAMPLE_TYPE paUInt8
typedef unsigned char SAMPLE;
#  define SAMPLE_SILENCE (128)
#  define PRINTF_S_FORMAT "%d"
#endif

typedef struct
{
  int frameIndex; /* Index into sample array. */
  int maxFrameIndex;
  SAMPLE* recordedSamples;
} paTestData;

/* This routine will be called by the PortAudio engine when audio is needed.
** It may be called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
static int recordCallback(
    const void* inputBuffer,
    void* outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData)
{
  paTestData* data = (paTestData*)userData;
  const SAMPLE* rptr = (const SAMPLE*)inputBuffer;
  SAMPLE* wptr = &data->recordedSamples[data->frameIndex * NUM_CHANNELS];
  long framesToCalc;
  long i;
  int finished;
  unsigned long framesLeft = data->maxFrameIndex - data->frameIndex;

  (void)outputBuffer; /* Prevent unused variable warnings. */
  (void)timeInfo;
  (void)statusFlags;
  (void)userData;

  if (framesLeft < framesPerBuffer)
  {
    framesToCalc = framesLeft;
    finished = paComplete;
  }
  else
  {
    framesToCalc = framesPerBuffer;
    finished = paContinue;
  }

  if (inputBuffer == NULL)
  {
    for (i = 0; i < framesToCalc; i++)
    {
      *wptr++ = SAMPLE_SILENCE; /* left */
      if (NUM_CHANNELS == 2)
        *wptr++ = SAMPLE_SILENCE; /* right */
    }
  }
  else
  {
    for (i = 0; i < framesToCalc; i++)
    {
      *wptr++ = *rptr++; /* left */
      if (NUM_CHANNELS == 2)
        *wptr++ = *rptr++; /* right */
    }
  }
  data->frameIndex += framesToCalc;
  return finished;
}
/*******************************************************************/
void dbg(int msgNum) 
{
  printf("%d\n", msgNum);
  fflush(stdout);
}
/*******************************************************************/
int main(int argc, char** argv)
{
  int msgNum = 0;

  if (!argv[1] || !argv[2]) 
  {
   fprintf(stderr, "Usage: ./test seconds_to_run device_number\n");
   return 1;
  }
  int NUM_SECONDS = std::stoi(std::string(argv[1]));
  int DEVICE_NUM = std::stoi(std::string(argv[2]));
  PaStreamParameters inputParameters, outputParameters;
  PaStream* stream;
  PaError err = paNoError;
  paTestData data;
  int i;
  int totalFrames;
  int numSamples;
  int numBytes;
  SAMPLE max, val;
  double average;

  fprintf(stdout, "Allocating audio array.\n");
  data.maxFrameIndex = totalFrames = NUM_SECONDS * SAMPLE_RATE; /* Record for a few seconds. */
  data.frameIndex = 0;
  numSamples = totalFrames * NUM_CHANNELS;
  numBytes = numSamples * sizeof(SAMPLE);
  data.recordedSamples
      = (SAMPLE*)malloc(numBytes); /* From now on, recordedSamples is initialised. */
  if (data.recordedSamples == NULL)
  {
    fprintf(stderr, "Could not allocate record array.\n");
    Pa_Terminate();
    if (data.recordedSamples) /* Sure it is NULL or valid. */
      free(data.recordedSamples);
    if (err != paNoError)
    {
      fprintf(stderr, "Error number: %d\n", err);
      fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
      err = 1; /* Always return 0 or 1, but no other return codes. */
    }
    return err;
  }
  for (i = 0; i < numSamples; i++)
    data.recordedSamples[i] = 0;

  fprintf(stdout, "Initializing PortAudio.\n");
  err = Pa_Initialize();
  if (err != paNoError)
  {
    fprintf(stderr, "An error occured while initializing the portaudio stream\n");
    Pa_Terminate();
    if (data.recordedSamples) /* Sure it is NULL or valid. */
      free(data.recordedSamples);
    if (err != paNoError)
    {
      fprintf(stderr, "An error occured while terminating the portaudio stream\n");
      fprintf(stderr, "Error number: %d\n", err);
      fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
      err = 1; /* Always return 0 or 1, but no other return codes. */
    }
    return err;
  }
  int x = Pa_GetHostApiCount();
  printf("hostApiCount:%d\n", x);
  fflush(stdout);
  x = Pa_GetDeviceCount();
  printf("deviceCount:%d\n", x);
  fflush(stdout);

  inputParameters.device = DEVICE_NUM;       // Pa_GetDefaultInputDevice(); /* default input device */
  inputParameters.channelCount = 2; /* stereo input */
  inputParameters.sampleFormat = PA_SAMPLE_TYPE;
  inputParameters.suggestedLatency
      = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
  inputParameters.hostApiSpecificStreamInfo = NULL;

  fprintf(stdout, "requested input channels: %d\n", inputParameters.channelCount);
  fprintf(stdout, "device max input channels: %d\n", Pa_GetDeviceInfo(inputParameters.device)->maxInputChannels);
  fprintf(stdout, "device name: %s\n", Pa_GetDeviceInfo(inputParameters.device)->name);

  /* Record some audio. -------------------------------------------- */
  fprintf(stdout, "Opening stream.\n");
  err = Pa_OpenStream(
      &stream, &inputParameters, NULL, /* &outputParameters, */
      SAMPLE_RATE, FRAMES_PER_BUFFER,
      paClipOff, /* we won't output out of range samples so don't bother clipping them */
      recordCallback, &data);
  if (err != paNoError)
  {
    fprintf(stderr, "An error occured while opening the portaudio stream\n");
    Pa_Terminate();
    if (data.recordedSamples) /* Sure it is NULL or valid. */
      free(data.recordedSamples);
    if (err != paNoError)
    {fprintf(stderr, "Error number: %d\n", err);
      fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
      err = 1; /* Always return 0 or 1, but no other return codes. */
    }
    return err;
  }
  fprintf(stdout, "Starting stream.\n");
  err = Pa_StartStream(stream);
  if (err != paNoError)
  {
    fprintf(stderr, "An error occured while starting the portaudio stream\n");
    Pa_Terminate();
    if (data.recordedSamples) /* Sure it is NULL or valid. */
      free(data.recordedSamples);
    if (err != paNoError)
    { 
      fprintf(stderr, "Error number: %d\n", err);
      fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
      err = 1; /* Always return 0 or 1, but no other return codes. */
    }
    return err;
  }

  /*******************************************************************/
  /*******************************************************************/
  /*******************************************************************/
  /*******************************************************************/
  std::cout << "************************* loading torch model ****\n";
  Model model;
  MyMidi midi;

  std::cout << "=== Now recording! Please beatbox. ===\n";

  std::vector<int> indices;
  int frameLength = 512;
  int shiftLength = 256;
  int nextIndex = frameLength;
  int max_cooldown = 8;
  int cooldown = 0;

  std::vector<int> predictions;
  std::vector<std::vector<float>> audio_vectors;
  // while( ( err = Pa_IsStreamActive( stream ) ) == 1 )
  for (int ix = 0; ix < NUM_SECONDS * 1000; ix++)
  {
    // indices.push_back(nextIndex);
    if (data.frameIndex >= nextIndex)
    {
      // std::cout << nextIndex << ", " << data.frameIndex << "\n";
      std::vector<float> audio_vector{
          data.recordedSamples + nextIndex - frameLength, data.recordedSamples + nextIndex};
      nextIndex += shiftLength;
      if (cooldown > 0)
      {
        cooldown -= 1;
      }
      else
      {
        int prediction = model.predict(audio_vector);
        if (prediction != 2)
          cooldown = max_cooldown;
        if (prediction == 0)
        {
          midi.send_message(68);
        }
        if (prediction == 1)
        {
          midi.send_message(74);
        }
        predictions.push_back(prediction);
        audio_vectors.push_back(audio_vector);
      }
    }
    else if (data.frameIndex >= data.maxFrameIndex)
    {
      break;
    }
    else
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }

  std::cout << "=== Done. finalizing ===\n";

  std::ofstream audio_vector_file;
  audio_vector_file.open("audio_vectors.csv");
  for (int i = 0; i < audio_vectors.size(); i++)
  {
    for (int j = 0; j < audio_vectors[i].size(); j++)
    {
      audio_vector_file << audio_vectors[i][j] << ",";
    }
    audio_vector_file << "\n";
  }
  audio_vector_file.close();

  std::ofstream prediction_file;
  prediction_file.open("predictions.csv");
  for (int i = 0; i < predictions.size(); i++)
  {
    prediction_file << predictions[i] << ",";
  }
  prediction_file.close();

  std::ofstream audio_file;
  audio_file.open("audio.csv");
  for (int i = 0; i < data.frameIndex; i++)
  {
    audio_file << data.recordedSamples[i] << ",";
  }
  audio_file.close();

  /*******************************************************************/
  /*******************************************************************/
  /*******************************************************************/
  /*******************************************************************/

  if (err < 0)
  {
    Pa_Terminate();
    if (data.recordedSamples) /* Sure it is NULL or valid. */
      free(data.recordedSamples);
    if (err != paNoError)
    {
      fprintf(stderr, "An error occured while using the portaudio stream\n");
      fprintf(stderr, "Error number: %d\n", err);
      fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
      err = 1; /* Always return 0 or 1, but no other return codes. */
    }
    return err;
  }

  err = Pa_CloseStream(stream);
  if (err != paNoError)
  {
    Pa_Terminate();
    if (data.recordedSamples) /* Sure it is NULL or valid. */
      free(data.recordedSamples);
    if (err != paNoError)
    {
      fprintf(stderr, "An error occured while using the portaudio stream\n");
      fprintf(stderr, "Error number: %d\n", err);
      fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
      err = 1; /* Always return 0 or 1, but no other return codes. */
    }
    return err;
  }
}
