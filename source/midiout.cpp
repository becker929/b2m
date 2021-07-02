#include "midiout.h"

// This function should be embedded in a try/catch block in case of
// an exception.  It offers the user a choice of MIDI ports to open.
MyMidi::MyMidi() {
  std::string portName;
  unsigned int i = 0, nPorts = midi_handler.get_port_count();
  if (nPorts == 0)
  {
    std::cout << "No output ports available!" << std::endl;
  }

  if (nPorts == 1)
  {
    std::cout << "\nOpening " << midi_handler.get_port_name() << std::endl;
  }
  else
  {
    for (i = 0; i < nPorts; i++)
    {
      portName = midi_handler.get_port_name(i);
      std::cout << "  Output port #" << i << ": " << portName << '\n';
    }

    do
    {
      std::cout << "\nChoose a port number: ";
      std::cin >> i;
    } while (i >= nPorts);
  }

  std::cout << "\n";
  midi_handler.open_port(i);
}

void MyMidi::send_message(int input) {
  try
  {
    using namespace std::literals;
    std::vector<unsigned char> message;

    // Send out a series of MIDI messages.
    message.push_back(0);
    message.push_back(0);
    message.push_back(0);
    // Note On: 144, 64, 90
    message[0] = 144;
    message[1] = input;
    message[2] = 90;
    midi_handler.send_message(message);

    std::this_thread::sleep_for(45ms);
    // Note Off: 128, 64, 40
    message[0] = 128;
    message[1] = input;
    message[2] = 40;
    midi_handler.send_message(message);
  }
  catch (const rtmidi::midi_exception& error)
  {
    std::cerr << error.what() << std::endl;
    exit(EXIT_FAILURE);
  }
}

// int main(void) {
//   send_message(58);
// }