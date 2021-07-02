# Beatbox-2-MIDI

This project is WIPPOCJFF (Work-In-Progress, Proof-of-Concept, Just-for-Fun).

Training of the neural net model is done in Python for fast iteration and ease of use. In order to reach usability as a real-time musical instrument, the model is loaded into a C++ app for integration with audio and MIDI capabilities. 

## How to train a model
The dataset has not been provided in this repo. Have a look into the IPython notebooks provided under the notebooks folder. The main thing to understand is that I have put together a simple Convoutional Neural Net (CNN) and trained it from a combination of both individual sample files as well as some longer files with samples selected using a markers.csv file. The model is trained in Python and then exported as traced_model_cnn_0.pt. 

## How to build
This has only been tested on a recent version of macOS. 

### You will need to place libtorch inside the source folder. It's too big for me to push, apparently.

See https://pytorch.org/cppdocs/installing.html for details.


Start from enclosing folder of this Readme.



$ cd source

$ cmake ./libtorch/share/cmake/Torch .

$ make

## How to run
Starting from within source folder



$ ./test seconds_to_run audio_device_index



e.g. the below will run for 30 seconds using the default audio device.



$ ./test 30 0



This will send a series of MIDI events at notes 68 (for a bass drum sound) and 74 (for a snare drum sound). At this time, inference is not very good. More iteration is needed on the model training.

Make sure that you are running from a command line that has access to your audio device.  Running from a VS Code CLI will not work and will throw an audio error.