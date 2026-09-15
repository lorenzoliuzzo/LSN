#pragma once

#include <filesystem>
#include <vector>

#include "core/vec3.h"

class BlockedHistogram;
class Random;

// Sums over the pairs i < j closer than the cut-off, from one pass over the pairs.
struct PairSums {
  double potential = 0.0;  // sum of u(r) = 4 (r^-12 - r^-6)
  double virial = 0.0;     // sum of r_ij . F_ij = 48 (r^-12 - 0.5 r^-6)
};

// N Lennard-Jones particles in a cubic box with periodic boundary conditions.
// Reduced units: lengths in sigma, energies in epsilon, mass m = 1, k_B = 1.
class LJSystem {
 public:
  LJSystem(int npart, double rho, double r_cut);

  int size() const { return static_cast<int>(pos.size()); }
  double side() const { return side_; }
  double rho() const { return rho_; }
  double volume() const { return side_ * side_ * side_; }

  // Minimum-image convention: the periodic copy of a displacement closest to the origin.
  Vec3 minimum_image(const Vec3& d) const;
  // Folds a position back into the box [-L/2, L/2].
  Vec3 wrap(const Vec3& r) const { return minimum_image(r); }

  // fcc lattice filling a cube of side fill * L centered in the box (fill = 0.5 for ex 04).
  void place_fcc(double fill);
  // Maxwell-Boltzmann velocities with zero total momentum, rescaled to temperature temp.
  void init_velocities_gauss(Random& rnd, double temp, double dt);
  // Low-entropy start of ex 04.2: speed sqrt(3T) along one random axis per particle.
  void init_velocities_delta(Random& rnd, double temp, double dt);
  // Swaps r(t) and r(t - dt): the next Verlet steps retrace the trajectory backwards.
  void reverse_time();

  // Energy of particle i placed at r, interacting with all the others (MC moves).
  double particle_energy(int i, const Vec3& r) const;
  // Potential energy and virial of a configuration; fills the g(r) counts when given.
  PairSums pair_sums(const std::vector<Vec3>& positions, BlockedHistogram* gofr) const;
  void compute_forces(std::vector<Vec3>& force) const;
  double kinetic_energy() const;

  // Contributions of the pairs beyond r_cut, assuming g(r) = 1 there.
  double tail_energy_per_particle() const;
  double tail_pressure() const;

  // xyz files hold coordinates in units of the box side, like NSL_SIMULATOR's.
  std::vector<Vec3> read_xyz(const std::filesystem::path& file) const;
  void write_xyz(const std::filesystem::path& file, const std::vector<Vec3>& positions) const;

  std::vector<Vec3> pos;      // positions at time t
  std::vector<Vec3> pos_old;  // positions at time t - dt (position Verlet); unused by MC
  std::vector<Vec3> vel;      // velocities (MD only)

 private:
  void set_old_positions(double dt);  // r(t - dt) = r(t) - v dt

  double rho_;
  double side_;
  double r_cut_;
  double r_cut2_;
};
