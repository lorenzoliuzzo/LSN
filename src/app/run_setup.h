#pragma once

#include <filesystem>
#include <optional>

class InputFile;
class Random;

enum class Simulation { lj_md, lj_mc, ising_metropolis, ising_gibbs };

// Parameters shared by every simulation type.
struct RunSetup {
  Simulation simulation = Simulation::lj_md;
  std::filesystem::path run_dir;
  std::filesystem::path output_dir;                  // <run_dir>/output, created if missing
  std::optional<std::filesystem::path> restart_dir;  // RESTART_FROM: output dir of an earlier run
  double temp = 0.0;
  int nblocks = 0;
  int nsteps = 0;  // steps per block
  int nequil = 0;  // steps run and discarded before the first block
};

RunSetup read_run_setup(const InputFile& input, const std::filesystem::path& run_dir);

// A fresh run seeds from SEED_FILE; a restart continues the stream saved in seed.out.
Random make_random(const InputFile& input, const RunSetup& setup);
