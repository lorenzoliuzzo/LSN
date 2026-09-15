#include "core/input_file.h"

#include "core/io.h"

InputFile::InputFile(const std::filesystem::path& file) : file_(file) {
  std::ifstream in = open_input(file);
  bool terminated = false;
  for (std::string line; std::getline(in, line);) {
    std::istringstream words(line);
    std::string key;
    if (!(words >> key) || key[0] == '#') continue;
    if (key == "ENDINPUT") {
      terminated = true;
      break;
    }
    std::vector<std::string> values;
    for (std::string value; words >> value;) values.push_back(value);
    if (!entries_.emplace(key, values).second) {
      throw std::runtime_error(file_.string() + ": key " + key + " appears twice");
    }
  }
  if (!terminated) throw std::runtime_error(file_.string() + ": missing ENDINPUT");
}

void InputFile::reject_unused() const {
  std::string unused;
  for (const auto& entry : entries_) {
    if (used_.count(entry.first) == 0) unused += " " + entry.first;
  }
  if (!unused.empty()) {
    throw std::runtime_error(file_.string() + ": keys not used by this simulation:" + unused);
  }
}
