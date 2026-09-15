#pragma once

#include <cstdint>
#include <filesystem>
#include <random>

// Uniform pseudo-random numbers, plus hand-written samplers for the distributions the
// course uses. Two engines:
//  - nsl_lcg: the course generator (Rannyu), a 48-bit linear congruential generator
//    x -> (a x + c) mod 2^48 with period 2^47, bit-for-bit identical to
//    NSL_SIMULATOR/SOURCE/random.cpp. The increment c comes from one line of the
//    Primes file: each line gives an independent stream (one per MPI rank).
//  - mt19937_64: the standard 64-bit Mersenne Twister, period 2^19937 - 1.
// Only the raw engine output is used. The <random> distributions are
// implementation-defined (different numbers on different compilers), so every
// distribution below is sampled by hand.
class Random {
 public:
  enum class Engine { nsl_lcg, mt19937_64 };

  // primes_line is 1-based. state_file is either a fresh seed (seed.in: four 12-bit
  // integers) or, with is_saved_state, a state written by save_state() (restart).
  Random(Engine engine, const std::filesystem::path& primes_file, int primes_line,
         const std::filesystem::path& state_file, bool is_saved_state);

  double rannyu();                          // uniform in [0, 1)
  double rannyu(double min, double max);    // uniform in [min, max)
  double gauss(double mean, double sigma);  // Box-Muller
  double exponential(double lambda);        // inversion of the cumulative
  double cauchy(double mu, double gamma);   // inversion of the cumulative

  void save_state(const std::filesystem::path& file) const;

 private:
  // The NSL multiplier, written by NSL_SIMULATOR as the base-4096 digits 502 1521 4071 2107.
  static constexpr std::uint64_t lcg_multiplier = 34522712143931ULL;
  static constexpr std::uint64_t lcg_mask = (std::uint64_t{1} << 48) - 1;

  Engine engine_;
  std::uint64_t lcg_state_ = 0;
  std::uint64_t lcg_increment_ = 0;
  std::mt19937_64 mt_;
  bool has_spare_gauss_ = false;
  double spare_gauss_ = 0.0;
};
