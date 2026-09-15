#pragma once

class InputFile;
class Random;
struct RunSetup;

// Lennard-Jones fluid: molecular dynamics (NVE, SIMULATION_TYPE 0) or Metropolis
// Monte Carlo (NVT, SIMULATION_TYPE 1).
void run_lj(const InputFile& input, const RunSetup& setup, Random& rnd);
