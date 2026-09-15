#include "lj/lj_dynamics.h"

#include <cmath>

#include "core/random.h"
#include "lj/lj_system.h"

void verlet_step(LJSystem& sys, std::vector<Vec3>& force, double dt) {
  sys.compute_forces(force);
  const double dt2 = dt * dt;
  for (int i = 0; i < sys.size(); ++i) {
    // r and r_old may sit on opposite sides of a boundary; the shift is a multiple of L,
    // which wrap() and minimum_image() remove.
    const Vec3 r_new = sys.wrap(2.0 * sys.pos[i] - sys.pos_old[i] + force[i] * dt2);
    sys.vel[i] = sys.minimum_image(r_new - sys.pos_old[i]) * (1.0 / (2.0 * dt));
    sys.pos_old[i] = sys.pos[i];
    sys.pos[i] = r_new;
  }
}

int metropolis_sweep(LJSystem& sys, Random& rnd, double beta, double delta) {
  int accepted = 0;
  for (int attempt = 0; attempt < sys.size(); ++attempt) {
    const int i = static_cast<int>(rnd.rannyu() * sys.size());
    const Vec3 shift{rnd.rannyu(-delta, delta), rnd.rannyu(-delta, delta), rnd.rannyu(-delta, delta)};
    const Vec3 trial = sys.wrap(sys.pos[i] + shift);
    const double delta_energy = sys.particle_energy(i, trial) - sys.particle_energy(i, sys.pos[i]);
    // The uniform trial move is symmetric, so Metropolis accepts with min(1, exp(-beta dE)).
    if (delta_energy <= 0.0 || rnd.rannyu() < std::exp(-beta * delta_energy)) {
      sys.pos[i] = trial;
      ++accepted;
    }
  }
  return accepted;
}
