#include "ising/ising_chain.h"

#include <cmath>
#include <print>
#include <stdexcept>
#include <string>

#include "core/io.h"
#include "core/random.h"

namespace {
std::size_t checked_nspin(int nspin) {
  if (nspin < 3) throw std::runtime_error("an Ising chain needs NPART >= 3");
  return static_cast<std::size_t>(nspin);
}
}  // namespace

IsingChain::IsingChain(int nspin, double coupling, double field)
    : spins_(checked_nspin(nspin), 1), coupling_(coupling), field_(field) {}

double IsingChain::local_field(int i) const {
  const int n = size();
  return coupling_ * (spins_[(i + n - 1) % n] + spins_[(i + 1) % n]) + field_;
}

double IsingChain::energy() const {
  double energy = 0.0;
  for (int i = 0; i < size(); ++i) {
    energy += -coupling_ * spins_[i] * spins_[(i + 1) % size()] - field_ * spins_[i];
  }
  return energy;
}

int IsingChain::magnetization() const {
  int sum = 0;
  for (int s : spins_) sum += s;
  return sum;
}

void IsingChain::randomize(Random& rnd) {
  for (int& s : spins_) s = rnd.rannyu() < 0.5 ? 1 : -1;
}

int IsingChain::metropolis_sweep(Random& rnd, double beta) {
  int accepted = 0;
  for (int attempt = 0; attempt < size(); ++attempt) {
    // Random sites, as in NSL_SIMULATOR. A sequential sweep accepts every dE = 0 flip with
    // certainty: an antiferromagnetic pattern then only shifts by one site per sweep and
    // never relaxes, so the chain does not reach the Boltzmann distribution.
    const int i = static_cast<int>(rnd.rannyu() * size());
    const double delta_energy = 2.0 * spins_[i] * local_field(i);
    if (delta_energy <= 0.0 || rnd.rannyu() < std::exp(-beta * delta_energy)) {
      spins_[i] = -spins_[i];
      ++accepted;
    }
  }
  return accepted;
}

void IsingChain::gibbs_sweep(Random& rnd, double beta) {
  // Heat bath: given its neighbours, s_i = +1 with probability
  //   exp(beta h_i) / (exp(beta h_i) + exp(-beta h_i)) = 1 / (1 + exp(-2 beta h_i)),
  // independently of its current value, so no move is ever rejected.
  for (int i = 0; i < size(); ++i) {
    const double p_up = 1.0 / (1.0 + std::exp(-2.0 * beta * local_field(i)));
    spins_[i] = rnd.rannyu() < p_up ? 1 : -1;
  }
}

void IsingChain::read_spins(const std::filesystem::path& file) {
  std::ifstream in = open_input(file);
  for (int& s : spins_) {
    if (!(in >> s) || (s != 1 && s != -1)) {
      throw std::runtime_error(file.string() + " must hold " + std::to_string(size()) + " spins, each +1 or -1");
    }
  }
}

void IsingChain::write_spins(const std::filesystem::path& file) const {
  std::ofstream out = open_output(file);
  for (int s : spins_) std::print(out, "{} ", s);
  std::print(out, "\n");
}
