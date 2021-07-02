#pragma once

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <rtmidi17/rtmidi17.hpp>
#include <thread>

class MyMidi {
    private:
        rtmidi::midi_out midi_handler;
    public:
        MyMidi();
        void send_message(int input);
};