#pragma once

#include <filesystem>
#include <vector>

class Random;

// 1D Ising chain with periodic boundary conditions (s_{N+1} = s_1), k_B = mu_B = 1:
//   H = -J sum_i s_i s_{i+1} - h sum_i s_i
class IsingChain {
 public:
  IsingChain(int nspin, double coupling, double field);

  int size() const { return static_cast<int>(spins_.size()); }
  double energy() const;
  int magnetization() const;  // sum_i s_i

  void randomize(Random& rnd);
  // One Monte Carlo step = N trial flips on randomly chosen spins. Returns accepted flips.
  int metropolis_sweep(Random& rnd, double beta);
  // One Monte Carlo step = every spin redrawn from its conditional probability.
  void gibbs_sweep(Random& rnd, double beta);

  void read_spins(const std::filesystem::path& file);
  void write_spins(const std::filesystem::path& file) const;

 private:
  // Field felt by spin i: J (s_{i-1} + s_{i+1}) + h. Flipping s_i changes H by 2 s_i times it.
  double local_field(int i) const;

  std::vector<int> spins_;
  double coupling_;
  double field_;
};
