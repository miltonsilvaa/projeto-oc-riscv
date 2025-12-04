#pragma once

#include <stdexcept>
#include <string>

class ExcecaoAmbiente : public std::runtime_error {
public:

    ExcecaoAmbiente(const std::string& msg) : std::runtime_error(msg) {}
};