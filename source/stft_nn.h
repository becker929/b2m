#pragma once

#include <torch/script.h> // One-stop header.
#include <memory>

class Model {
    private:
        torch::jit::script::Module module;

    public:
        Model();
        int predict(std::vector<float> input);
};