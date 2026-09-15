#pragma once

class InputFile;
class Random;
struct RunSetup;

// 1D Ising chain sampled with Metropolis (SIMULATION_TYPE 2 J h) or Gibbs (SIMULATION_TYPE 3 J h).
void run_ising(const InputFile& input, const RunSetup& setup, Random& rnd);
