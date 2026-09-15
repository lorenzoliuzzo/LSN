#include "app/run_setup.h"

#include <stdexcept>
#include <string>

#include "core/input_file.h"
#include "core/random.h"

namespace {
Simulation to_simulation(int type) {
  switch (type) {
    case 0: return Simulation::lj_md;
    case 1: return Simulation::lj_mc;
    case 2: return Simulation::ising_metropolis;
    case 3: return Simulation::ising_gibbs;
    default:
      throw std::runtime_error("SIMULATION_TYPE must be 0 (LJ MD), 1 (LJ MC), 2 (Ising Metropolis) or 3 (Ising Gibbs)");
  }
}

Random::Engine to_engine(const std::string& name) {
  if (name == "NSL") return Random::Engine::nsl_lcg;
  if (name == "MT19937_64") return Random::Engine::mt19937_64;
  throw std::runtime_error("RNG_ENGINE must be NSL or MT19937_64, got " + name);
}
}  // namespace

RunSetup read_run_setup(const InputFile& input, const std::filesystem::path& run_dir) {
  RunSetup setup;
  setup.simulation = to_simulation(input.get<int>("SIMULATION_TYPE"));
  setup.run_dir = run_dir;
  setup.output_dir = run_dir / "output";
  if (input.has("RESTART_FROM")) {
    setup.restart_dir = run_dir / input.get<std::string>("RESTART_FROM");
    if (!std::filesystem::is_directory(*setup.restart_dir)) {
      throw std::runtime_error("RESTART_FROM directory not found: " + setup.restart_dir->string());
    }
  }
  setup.temp = input.get<double>("TEMP");
  setup.nblocks = input.get<int>("NBLOCKS");
  setup.nsteps = input.get<int>("NSTEPS");
  setup.nequil = input.get_or<int>("NEQUIL", 0);
  if (setup.temp <= 0.0 || setup.nblocks < 1 || setup.nsteps < 1 || setup.nequil < 0) {
    throw std::runtime_error("need TEMP > 0, NBLOCKS >= 1, NSTEPS >= 1 and NEQUIL >= 0");
  }
  std::filesystem::create_directories(setup.output_dir);
  return setup;
}

Random make_random(const InputFile& input, const RunSetup& setup) {
  const Random::Engine engine = to_engine(input.get_or<std::string>("RNG_ENGINE", "NSL"));
  const std::filesystem::path primes = setup.run_dir / input.get_or<std::string>("PRIMES_FILE", "Primes");
  const int primes_line = input.get_or<int>("PRIMES_LINE", 1);
  if (primes_line < 1) throw std::runtime_error("PRIMES_LINE counts from 1");
  // Read even on restart, where seed.out takes over, so that RESTART_FROM can simply be
  // added to the input file of the run being continued.
  const std::filesystem::path seed = setup.run_dir / input.get_or<std::string>("SEED_FILE", "seed.in");
  if (setup.restart_dir) return Random(engine, primes, primes_line, *setup.restart_dir / "seed.out", true);
  return Random(engine, primes, primes_line, seed, false);
}
