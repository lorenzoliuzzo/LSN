#include "core/random.h"

#include <cmath>
#include <stdexcept>
#include <string>

#include "core/io.h"

namespace {
constexpr double pi = 3.14159265358979323846;
constexpr std::uint64_t seed_base = 4096;  // NSL seeds and increments are 12-bit digits
}  // namespace

Random::Random(Engine engine, const std::filesystem::path& primes_file, int primes_line,
               const std::filesystem::path& state_file, bool is_saved_state)
    : engine_(engine) {
  std::ifstream primes = open_input(primes_file);
  std::uint64_t p1 = 0;
  std::uint64_t p2 = 0;
  for (int line = 1; line <= primes_line; ++line) {
    if (!(primes >> p1 >> p2)) {
      throw std::runtime_error(primes_file.string() + " has fewer than " + std::to_string(primes_line) +
                               " lines");
    }
  }

  std::ifstream state = open_input(state_file);
  if (engine_ == Engine::mt19937_64 && is_saved_state) {
    if (!(state >> mt_)) {
      throw std::runtime_error("cannot read a Mersenne Twister state from " + state_file.string());
    }
    return;
  }
  std::uint64_t digit[4];
  for (std::uint64_t& d : digit) {
    if (!(state >> d)) throw std::runtime_error("cannot read four seed integers from " + state_file.string());
  }
  if (engine_ == Engine::nsl_lcg) {
    lcg_state_ = ((digit[0] * seed_base + digit[1]) * seed_base + digit[2]) * seed_base + digit[3];
    lcg_increment_ = p1 * seed_base + p2;
  } else {
    std::seed_seq sequence{digit[0], digit[1], digit[2], digit[3], p1, p2};
    mt_.seed(sequence);
  }
}

double Random::rannyu() {
  if (engine_ == Engine::nsl_lcg) {
    // Unsigned overflow wraps modulo 2^64, a multiple of 2^48, so masking the low
    // 48 bits gives (a x + c) mod 2^48 exactly.
    lcg_state_ = (lcg_multiplier * lcg_state_ + lcg_increment_) & lcg_mask;
    return static_cast<double>(lcg_state_) * 0x1.0p-48;
  }
  // The top 53 bits fill a double's mantissa exactly: uniform on the grid k / 2^53.
  return static_cast<double>(mt_() >> 11) * 0x1.0p-53;
}

double Random::rannyu(double min, double max) { return min + (max - min) * rannyu(); }

double Random::gauss(double mean, double sigma) {
  // Box-Muller turns two uniforms into two independent standard normals; the second
  // one is kept for the next call instead of being thrown away.
  if (has_spare_gauss_) {
    has_spare_gauss_ = false;
    return mean + sigma * spare_gauss_;
  }
  const double radius = std::sqrt(-2.0 * std::log(1.0 - rannyu()));  // 1 - u is in (0, 1]
  const double angle = 2.0 * pi * rannyu();
  spare_gauss_ = radius * std::sin(angle);
  has_spare_gauss_ = true;
  return mean + sigma * radius * std::cos(angle);
}

double Random::exponential(double lambda) {
  // F(x) = 1 - exp(-lambda x)  =>  x = -ln(1 - u) / lambda
  return -std::log(1.0 - rannyu()) / lambda;
}

double Random::cauchy(double mu, double gamma) {
  // F(x) = 1/2 + arctan((x - mu) / gamma) / pi  =>  x = mu + gamma tan(pi (u - 1/2))
  return mu + gamma * std::tan(pi * (rannyu() - 0.5));
}

void Random::save_state(const std::filesystem::path& file) const {
  std::ofstream out = open_output(file);
  if (engine_ == Engine::mt19937_64) {
    out << mt_ << '\n';
    return;
  }
  // Same four-digit format as seed.in, so a saved state can also seed a fresh run.
  out << (lcg_state_ >> 36) << ' ' << ((lcg_state_ >> 24) & 4095) << ' ' << ((lcg_state_ >> 12) & 4095)
      << ' ' << (lcg_state_ & 4095) << '\n';
}
