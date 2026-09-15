#include <exception>
#include <filesystem>
#include <iostream>

#include "app/run_setup.h"
#include "core/input_file.h"
#include "core/random.h"
#include "ising/ising_run.h"
#include "lj/lj_run.h"

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "usage: " << argv[0] << " <run_dir>\n"
              << "  reads <run_dir>/input.dat, writes results to <run_dir>/output/\n";
    return 2;
  }
  try {
    const std::filesystem::path run_dir = argv[1];
    const InputFile input(run_dir / "input.dat");
    const RunSetup setup = read_run_setup(input, run_dir);
    Random rnd = make_random(input, setup);
    switch (setup.simulation) {
      case Simulation::lj_md:
      case Simulation::lj_mc:
        run_lj(input, setup, rnd);
        break;
      case Simulation::ising_metropolis:
      case Simulation::ising_gibbs:
        run_ising(input, setup, rnd);
        break;
    }
  } catch (const std::exception& error) {
    std::cerr << "error: " << error.what() << '\n';
    return 1;
  }
  return 0;
}
