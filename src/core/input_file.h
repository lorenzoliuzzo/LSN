#pragma once

#include <filesystem>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

// Parameter file in the NSL_SIMULATOR style: one "KEY value [value ...]" per line,
// read up to ENDINPUT. Blank lines and lines starting with '#' are skipped.
// The simulation must read every key it is given (reject_unused), so a misspelled
// key stops the run instead of silently leaving a default in place.
class InputFile {
 public:
  explicit InputFile(const std::filesystem::path& file);

  bool has(const std::string& key) const { return entries_.count(key) > 0; }

  // token selects one of the values on the line, e.g. SIMULATION_TYPE 2 1.0 0.0.
  template <class T>
  T get(const std::string& key, std::size_t token = 0) const;

  template <class T>
  T get_or(const std::string& key, const T& fallback, std::size_t token = 0) const {
    return has(key) ? get<T>(key, token) : fallback;
  }

  // Throws if the file holds keys that no get() asked for.
  void reject_unused() const;

 private:
  std::filesystem::path file_;
  std::map<std::string, std::vector<std::string>> entries_;
  mutable std::set<std::string> used_;
};

template <class T>
T InputFile::get(const std::string& key, std::size_t token) const {
  const auto entry = entries_.find(key);
  if (entry == entries_.end()) throw std::runtime_error(file_.string() + ": missing key " + key);
  used_.insert(key);
  if (token >= entry->second.size()) {
    throw std::runtime_error(file_.string() + ": " + key + " needs at least " +
                             std::to_string(token + 1) + " value(s)");
  }
  const std::string& text = entry->second[token];
  if constexpr (std::is_same_v<T, std::string>) {
    return text;
  } else {
    std::istringstream in(text);
    T value{};
    char trailing = 0;
    if (!(in >> value) || (in >> trailing)) {
      throw std::runtime_error(file_.string() + ": cannot read '" + text + "' as the value of " + key);
    }
    return value;
  }
}
