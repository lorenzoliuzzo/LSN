#pragma once

#include <filesystem>
#include <fstream>

// Open a file or throw: a missing input must stop the run, not produce silent zeros.
std::ifstream open_input(const std::filesystem::path& file);
std::ofstream open_output(const std::filesystem::path& file);
