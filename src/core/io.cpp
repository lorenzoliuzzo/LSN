#include "core/io.h"

#include <stdexcept>

std::ifstream open_input(const std::filesystem::path& file) {
  std::ifstream in(file);
  if (!in) throw std::runtime_error("cannot open input file " + file.string());
  return in;
}

std::ofstream open_output(const std::filesystem::path& file) {
  std::ofstream out(file);
  if (!out) throw std::runtime_error("cannot open output file " + file.string());
  return out;
}
