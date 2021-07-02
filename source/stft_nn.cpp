#include "stft_nn.h"

#include <iostream>

Model::Model()
{
  try
  {
    // Deserialize the ScriptModule from a file using torch::jit::load().
    module = torch::jit::load("traced_model_cnn_0.pt");
    // module = torch::jit::load("traced_model_512_1.pt");

    // // Create a tensor from the input then wrap in a std::vector
    // at::Tensor input_tensor = torch::zeros({512});
    // input_tensor = input_tensor.reshape({1, 512});

    // // Execute the model and turn its output into a tensor, then get the pred.
    // std::vector<torch::jit::IValue> inputs;
    // inputs.push_back(input_tensor);
    // at::Tensor output_tensor = module.forward(inputs).toTensor().argmax(1);
    // std::cout << output_tensor << '\n';
  }
  catch (const c10::Error& e)
  {
    std::cerr << "!!!!!!!!!!!! error loading the model !!!!!!!!!\n";
  }
  std::cout << "******************* successfully loaded model **********\n";
}

int Model::predict(std::vector<float> input)
{
  // std::cout << "predicting\n";
  // Create a tensor from the input then wrap in a std::vector
  at::Tensor input_tensor = torch::from_blob((float*)(input.data()), input.size());
  // std::cout << "input_tensor: " << input_tensor.sizes() <<"\n";
  input_tensor = input_tensor.reshape({1, 512});
  // std::cout << "input_tensor: " << input_tensor.sizes() <<"\n";

  // Execute the model and turn its output into a tensor, then get the pred.
  std::vector<torch::jit::IValue> inputs;
  inputs.push_back(input_tensor);
  // std::cout << "inputs vector: " << inputs.size() << "\n";
  at::Tensor output_tensor = module.forward(inputs).toTensor().argmax(1);
  // std::cout << "output_tensor: " << output_tensor.sizes() <<"\n";

  return output_tensor.item().to<int64_t>();
}