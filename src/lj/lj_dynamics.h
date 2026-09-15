#pragma once

#include <vector>

#include "core/vec3.h"

class LJSystem;
class Random;

// One position Verlet step (m = 1):  r(t+dt) = 2 r(t) - r(t-dt) + F(t) dt^2.
// Velocities come from the central difference v(t) = [r(t+dt) - r(t-dt)] / (2 dt).
// Afterwards pos holds r(t+dt), pos_old holds r(t) and vel holds v(t).
// force is a work buffer, kept by the caller to avoid reallocating it every step.
void verlet_step(LJSystem& sys, std::vector<Vec3>& force, double dt);

// N Metropolis trial moves (NVT) on randomly chosen particles, each displaced uniformly
// in a cube of half-side delta. Returns the number of accepted moves.
int metropolis_sweep(LJSystem& sys, Random& rnd, double beta, double delta);
