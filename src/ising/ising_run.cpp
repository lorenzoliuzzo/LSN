#include "ising/ising_run.h"

#include <fstream>
#include <print>

#include "app/run_setup.h"
#include "core/blocking.h"
#include "core/input_file.h"
#include "core/io.h"
#include "core/random.h"
#include "ising/ising_chain.h"

void run_ising(const InputFile& input, const RunSetup& setup, Random& rnd) {
  const bool gibbs = setup.simulation == Simulation::ising_gibbs;
  // SIMULATION_TYPE 2|3 J h, as in NSL_SIMULATOR.
  const double coupling = input.get<double>("SIMULATION_TYPE", 1);
  const double field = input.get<double>("SIMULATION_TYPE", 2);
  const int nspin = input.get<int>("NPART");
  input.reject_unused();

  IsingChain chain(nspin, coupling, field);
  if (setup.restart_dir) {
    chain.read_spins(*setup.restart_dir / "config.spin");
  } else {
    chain.randomize(rnd);
  }

  // Everything per spin, as in the exact formulas of exercise 06.
  const std::filesystem::path& out = setup.output_dir;
  ScalarObservable energy(out / "total_energy.dat", "U/N");
  ScalarObservable heat_capacity(out / "specific_heat.dat", "C/N");
  ScalarObservable susceptibility(out / "susceptibility.dat", "CHI/N");
  ScalarObservable magnetization(out / "magnetization.dat", "M/N");
  std::ofstream acceptance = open_output(out / "acceptance.dat");
  std::println(acceptance, "# BLOCK: ACCEPTANCE:");

  const double beta = 1.0 / setup.temp;
  auto sweep = [&]() -> int {
    if (!gibbs) return chain.metropolis_sweep(rnd, beta);
    chain.gibbs_sweep(rnd, beta);
    return chain.size();
  };
  for (int step = 0; step < setup.nequil; ++step) sweep();

  for (int block = 1; block <= setup.nblocks; ++block) {
    long accepted = 0;
    double sum_h = 0.0;
    double sum_h2 = 0.0;
    double sum_m2 = 0.0;
    for (int step = 0; step < setup.nsteps; ++step) {
      accepted += sweep();
      const double h = chain.energy();
      const double m = chain.magnetization();
      sum_h += h;
      sum_h2 += h * h;
      sum_m2 += m * m;
      energy.add(h / nspin);
      magnetization.add(m / nspin);
    }
    const double samples = setup.nsteps;
    const double mean_h = sum_h / samples;
    energy.close_block();
    magnetization.close_block();
    // Fluctuation formulas, estimated within each block:
    //   C = beta^2 (<H^2> - <H>^2),   chi = beta <M^2>  (the h = 0 form, where <M> = 0).
    heat_capacity.close_block(beta * beta * (sum_h2 / samples - mean_h * mean_h) / nspin);
    susceptibility.close_block(beta * (sum_m2 / samples) / nspin);
    std::println(acceptance, "{} {}", block, static_cast<double>(accepted) / (samples * nspin));
  }

  chain.write_spins(out / "config.spin");
  rnd.save_state(out / "seed.out");

  std::println("1D Ising, {}, T = {}: {} blocks x {} steps", gibbs ? "Gibbs" : "Metropolis", setup.temp,
               setup.nblocks, setup.nsteps);
  print_summary("U/N", energy);
  print_summary("C/N", heat_capacity);
  print_summary("CHI/N", susceptibility);
  print_summary("M/N", magnetization);
}
