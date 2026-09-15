#include "lj/lj_system.h"

#include <cmath>
#include <numbers>
#include <print>
#include <stdexcept>
#include <string>

#include "core/blocking.h"
#include "core/io.h"
#include "core/random.h"

namespace {
constexpr double pi = std::numbers::pi;

std::size_t checked_npart(int npart) {
  if (npart < 2) throw std::runtime_error("NPART must be at least 2");
  return static_cast<std::size_t>(npart);
}
}  // namespace

LJSystem::LJSystem(int npart, double rho, double r_cut)
    : pos(checked_npart(npart)),
      pos_old(pos.size()),
      vel(pos.size()),
      rho_(rho),
      side_(std::cbrt(npart / rho)),
      r_cut_(r_cut),
      r_cut2_(r_cut * r_cut) {
  if (rho <= 0.0) throw std::runtime_error("RHO must be positive");
  // The minimum-image convention sees each neighbour once only up to half the box.
  if (r_cut <= 0.0 || r_cut > 0.5 * side_) {
    throw std::runtime_error("R_CUT must be in (0, L/2], with L = " + std::to_string(side_));
  }
}

Vec3 LJSystem::minimum_image(const Vec3& d) const {
  return {d.x - side_ * std::rint(d.x / side_), d.y - side_ * std::rint(d.y / side_),
          d.z - side_ * std::rint(d.z / side_)};
}

void LJSystem::place_fcc(double fill) {
  const int cells = static_cast<int>(std::lround(std::cbrt(size() / 4.0)));
  if (4 * cells * cells * cells != size()) {
    throw std::runtime_error("an fcc lattice needs NPART = 4 k^3 (32, 108, 256, ...)");
  }
  const double lattice_constant = fill * side_ / cells;
  const Vec3 basis[4] = {{0.0, 0.0, 0.0}, {0.5, 0.5, 0.0}, {0.5, 0.0, 0.5}, {0.0, 0.5, 0.5}};
  const Vec3 corner{-0.5 * fill * side_, -0.5 * fill * side_, -0.5 * fill * side_};
  int i = 0;
  for (int ix = 0; ix < cells; ++ix) {
    for (int iy = 0; iy < cells; ++iy) {
      for (int iz = 0; iz < cells; ++iz) {
        for (const Vec3& b : basis) {
          pos[i++] = wrap(corner + Vec3{ix + b.x, iy + b.y, iz + b.z} * lattice_constant);
        }
      }
    }
  }
  pos_old = pos;
}

void LJSystem::init_velocities_gauss(Random& rnd, double temp, double dt) {
  const double sigma = std::sqrt(temp);  // each component of v is N(0, T) for m = k_B = 1
  Vec3 total;
  for (Vec3& v : vel) {
    v = {rnd.gauss(0.0, sigma), rnd.gauss(0.0, sigma), rnd.gauss(0.0, sigma)};
    total += v;
  }
  const Vec3 drift = total * (1.0 / size());
  double sum_v2 = 0.0;
  for (Vec3& v : vel) {
    v -= drift;
    sum_v2 += norm2(v);
  }
  // Rescale so the kinetic energy per particle is exactly 3T/2 (equipartition).
  const double scale = std::sqrt(3.0 * temp * size() / sum_v2);
  for (Vec3& v : vel) v *= scale;
  set_old_positions(dt);
}

void LJSystem::init_velocities_delta(Random& rnd, double temp, double dt) {
  // Every particle gets speed v_T = sqrt(3T) along a single axis, so K/N = 3T/2 as for
  // a thermal state. Particles come in pairs with opposite velocities: zero total momentum.
  if (size() % 2 != 0) throw std::runtime_error("INIT_VELOCITIES DELTA needs an even NPART");
  const double v_t = std::sqrt(3.0 * temp);
  for (int i = 0; i < size(); i += 2) {
    const int axis = static_cast<int>(3.0 * rnd.rannyu());
    Vec3 v;
    v[axis] = rnd.rannyu() < 0.5 ? v_t : -v_t;
    vel[i] = v;
    vel[i + 1] = -v;
  }
  set_old_positions(dt);
}

void LJSystem::set_old_positions(double dt) {
  for (int i = 0; i < size(); ++i) pos_old[i] = wrap(pos[i] - vel[i] * dt);
}

void LJSystem::reverse_time() {
  // Position Verlet is time-reversible: from the state (r(t-dt), r(t)) the next step
  // gives r(t-2dt), i.e. the same trajectory run with inverted velocities.
  pos.swap(pos_old);
  for (Vec3& v : vel) v = -v;
}

double LJSystem::particle_energy(int i, const Vec3& r) const {
  double energy = 0.0;
  for (int j = 0; j < size(); ++j) {
    if (j == i) continue;
    const double r2 = norm2(minimum_image(r - pos[j]));
    if (r2 < r_cut2_) {
      const double inv_r6 = 1.0 / (r2 * r2 * r2);
      energy += 4.0 * inv_r6 * (inv_r6 - 1.0);
    }
  }
  return energy;
}

PairSums LJSystem::pair_sums(const std::vector<Vec3>& positions, BlockedHistogram* gofr) const {
  PairSums sums;
  const double half_side2 = 0.25 * side_ * side_;
  for (int i = 0; i < size() - 1; ++i) {
    for (int j = i + 1; j < size(); ++j) {
      const double r2 = norm2(minimum_image(positions[i] - positions[j]));
      // The pair is at distance r from i and from j: it counts twice in g(r).
      if (gofr != nullptr && r2 < half_side2) gofr->fill(std::sqrt(r2), 2.0);
      if (r2 < r_cut2_) {
        const double inv_r6 = 1.0 / (r2 * r2 * r2);
        sums.potential += 4.0 * inv_r6 * (inv_r6 - 1.0);
        sums.virial += 48.0 * inv_r6 * (inv_r6 - 0.5);
      }
    }
  }
  return sums;
}

void LJSystem::compute_forces(std::vector<Vec3>& force) const {
  force.assign(pos.size(), Vec3{});
  for (int i = 0; i < size() - 1; ++i) {
    for (int j = i + 1; j < size(); ++j) {
      const Vec3 d = minimum_image(pos[i] - pos[j]);
      const double r2 = norm2(d);
      if (r2 < r_cut2_) {
        const double inv_r2 = 1.0 / r2;
        const double inv_r6 = inv_r2 * inv_r2 * inv_r2;
        // F_ij = -du/dr d/r = 48 d (r^-14 - 0.5 r^-8)
        const Vec3 f = d * (48.0 * inv_r2 * inv_r6 * (inv_r6 - 0.5));
        force[i] += f;
        force[j] -= f;  // Newton's third law: each pair is computed once
      }
    }
  }
}

double LJSystem::kinetic_energy() const {
  double energy = 0.0;
  for (const Vec3& v : vel) energy += 0.5 * norm2(v);
  return energy;
}

double LJSystem::tail_energy_per_particle() const {
  // U_tail / N = (rho / 2) int_rc^inf 4 pi r^2 u(r) dr = (8/3) pi rho (rc^-9 / 3 - rc^-3)
  const double inv_rc3 = 1.0 / (r_cut_ * r_cut_ * r_cut_);
  return 8.0 / 3.0 * pi * rho_ * (inv_rc3 * inv_rc3 * inv_rc3 / 3.0 - inv_rc3);
}

double LJSystem::tail_pressure() const {
  // P_tail = -(2/3) pi rho^2 int_rc^inf r^3 u'(r) dr = (16/3) pi rho^2 (2/3 rc^-9 - rc^-3)
  const double inv_rc3 = 1.0 / (r_cut_ * r_cut_ * r_cut_);
  return 16.0 / 3.0 * pi * rho_ * rho_ * (2.0 / 3.0 * inv_rc3 * inv_rc3 * inv_rc3 - inv_rc3);
}

std::vector<Vec3> LJSystem::read_xyz(const std::filesystem::path& file) const {
  std::ifstream in = open_input(file);
  int count = 0;
  in >> count;
  if (count != size()) {
    throw std::runtime_error(file.string() + " holds " + std::to_string(count) + " particles, NPART is " +
                             std::to_string(size()));
  }
  std::string line;
  std::getline(in, line);  // end of the count line
  std::getline(in, line);  // comment line
  std::vector<Vec3> positions(pos.size());
  for (Vec3& r : positions) {
    std::string name;
    Vec3 scaled;
    if (!(in >> name >> scaled.x >> scaled.y >> scaled.z)) {
      throw std::runtime_error("cannot read the particle coordinates in " + file.string());
    }
    r = wrap(scaled * side_);
  }
  return positions;
}

void LJSystem::write_xyz(const std::filesystem::path& file, const std::vector<Vec3>& positions) const {
  std::ofstream out = open_output(file);
  std::println(out, "{}\n# positions in units of the box side", size());
  // {} prints the shortest decimal that reads back as the same double, so a restart is exact.
  for (const Vec3& r : positions) std::println(out, "LJ {} {} {}", r.x / side_, r.y / side_, r.z / side_);
}
