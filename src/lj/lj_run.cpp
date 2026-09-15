#include "lj/lj_run.h"

#include <cmath>
#include <numbers>
#include <optional>
#include <print>
#include <stdexcept>
#include <string>
#include <vector>

#include "app/run_setup.h"
#include "core/blocking.h"
#include "core/input_file.h"
#include "core/io.h"
#include "core/random.h"
#include "lj/lj_dynamics.h"
#include "lj/lj_system.h"

namespace {
constexpr double pi = std::numbers::pi;

double lattice_fill(const std::string& lattice) {
  if (lattice == "FCC") return 1.0;
  if (lattice == "FCC_HALF") return 0.5;  // ex 04.2: crystal in half of the box per dimension
  throw std::runtime_error("INIT_LATTICE must be FCC or FCC_HALF, got " + lattice);
}
}  // namespace

void run_lj(const InputFile& input, const RunSetup& setup, Random& rnd) {
  const bool md = setup.simulation == Simulation::lj_md;
  const int npart = input.get<int>("NPART");
  const double rho = input.get<double>("RHO");
  const double r_cut = input.get<double>("R_CUT");
  // MD: the time step dt. MC: the half-side of the uniform trial displacement.
  const double delta = input.get<double>("DELTA");
  const int gofr_bins = input.get_or<int>("GOFR_BINS", 100);
  const bool print_instant = input.get_or<bool>("PRINT_INSTANT", false);
  const int xyz_every = input.get_or<int>("XYZ_EVERY", 0);

  // The initial-state keys are read even on restart, where the saved configuration takes
  // over, so that RESTART_FROM can simply be added to the input file of the run being continued.
  const double fill = lattice_fill(input.get_or<std::string>("INIT_LATTICE", "FCC"));
  const std::string velocities = md ? input.get_or<std::string>("INIT_VELOCITIES", "GAUSS") : "";
  if (md && velocities != "GAUSS" && velocities != "DELTA") {
    throw std::runtime_error("INIT_VELOCITIES must be GAUSS or DELTA, got " + velocities);
  }

  LJSystem sys(npart, rho, r_cut);
  if (setup.restart_dir) {
    sys.pos = sys.read_xyz(*setup.restart_dir / "config.xyz");
    if (md) {
      sys.pos_old = sys.read_xyz(*setup.restart_dir / "conf-1.xyz");
      if (input.get_or<bool>("REVERSE_TIME", false)) sys.reverse_time();
    }
  } else {
    sys.place_fcc(fill);
    if (md && velocities == "GAUSS") sys.init_velocities_gauss(rnd, setup.temp, delta);
    if (md && velocities == "DELTA") sys.init_velocities_delta(rnd, setup.temp, delta);
  }
  std::optional<BlockedHistogram> pofv;
  if (md) {
    pofv.emplace(input.get_or<int>("POFV_BINS", 50), input.get_or<double>("POFV_VMAX", 4.0 * std::sqrt(setup.temp)));
  }
  input.reject_unused();

  const std::filesystem::path& out = setup.output_dir;
  ScalarObservable potential(out / "potential_energy.dat", "U/N");
  ScalarObservable pressure(out / "pressure.dat", "P");
  std::optional<ScalarObservable> kinetic;
  std::optional<ScalarObservable> total;
  std::optional<ScalarObservable> temperature;
  std::ofstream pofv_blocks;
  if (md) {
    kinetic.emplace(out / "kinetic_energy.dat", "K/N");
    total.emplace(out / "total_energy.dat", "E/N");
    temperature.emplace(out / "temperature.dat", "T");
    pofv_blocks = open_output(out / "pofv_blocks.dat");
    std::println(pofv_blocks, "# BLOCK: VELOCITY: POFV:");
  }
  BlockedHistogram gofr(gofr_bins, 0.5 * sys.side());
  std::ofstream gofr_blocks = open_output(out / "gofr_blocks.dat");
  std::println(gofr_blocks, "# BLOCK: DISTANCE: GOFR:");
  std::ofstream acceptance = open_output(out / "acceptance.dat");
  std::println(acceptance, "# BLOCK: ACCEPTANCE:");
  std::ofstream instant;
  if (print_instant) {
    instant = open_output(out / "instant.dat");
    std::println(instant, "{}", md ? "# STEP: U/N: T:" : "# STEP: U/N:");
  }
  if (xyz_every > 0) std::filesystem::create_directories(out / "CONFIG");

  const double beta = 1.0 / setup.temp;
  std::vector<Vec3> force;
  // One time step (MD) or one sweep of N trial moves (MC); returns the accepted moves.
  auto advance = [&]() -> int {
    if (!md) return metropolis_sweep(sys, rnd, beta, delta);
    verlet_step(sys, force, delta);
    return sys.size();
  };
  for (int step = 0; step < setup.nequil; ++step) advance();

  long step = 0;
  for (int block = 1; block <= setup.nblocks; ++block) {
    long accepted = 0;
    for (int i = 0; i < setup.nsteps; ++i) {
      ++step;
      accepted += advance();
      // After a Verlet step pos_old holds r(t) and vel holds v(t): measure both at t,
      // so that E = K + U is evaluated on one instant of the trajectory.
      const std::vector<Vec3>& conf = md ? sys.pos_old : sys.pos;
      const PairSums pairs = sys.pair_sums(conf, &gofr);
      const double u = pairs.potential / npart + sys.tail_energy_per_particle();
      double t_now = setup.temp;  // MC samples the canonical ensemble at fixed T
      potential.add(u);
      if (md) {
        const double k = sys.kinetic_energy() / npart;
        t_now = 2.0 / 3.0 * k;  // equipartition: K/N = 3T/2
        kinetic->add(k);
        total->add(k + u);
        temperature->add(t_now);
        for (const Vec3& v : sys.vel) pofv->fill(norm(v));
      }
      // Virial theorem: P = rho T + W / (3V), W = sum over pairs of r_ij . F_ij, plus the tail.
      pressure.add(rho * t_now + pairs.virial / (3.0 * sys.volume()) + sys.tail_pressure());
      if (print_instant) {
        if (md) {
          std::println(instant, "{} {:.10g} {:.10g}", step, u, t_now);
        } else {
          std::println(instant, "{} {:.10g}", step, u);
        }
      }
      if (xyz_every > 0 && step % xyz_every == 0) {
        sys.write_xyz(out / "CONFIG" / ("config_" + std::to_string(step) + ".xyz"), conf);
      }
    }

    const double samples = setup.nsteps;
    potential.close_block();
    pressure.close_block();
    // g(r): pairs in the shell [r, r + dr] relative to an ideal gas at the same density.
    gofr.close_block([&](int bin) {
      const double r_in = gofr.lower(bin);
      const double r_out = r_in + gofr.width();
      const double shell_volume = 4.0 / 3.0 * pi * (r_out * r_out * r_out - r_in * r_in * r_in);
      return samples * rho * npart * shell_volume;
    });
    gofr.write_block(gofr_blocks, block);
    if (md) {
      kinetic->close_block();
      total->close_block();
      temperature->close_block();
      // p(v): fraction of particles per unit speed, so the histogram integrates to 1.
      pofv->close_block([&](int) { return samples * npart * pofv->width(); });
      pofv->write_block(pofv_blocks, block);
    }
    std::println(acceptance, "{} {}", block, static_cast<double>(accepted) / (samples * npart));
  }

  gofr.write_final(out / "gofr.dat", "DISTANCE", "GOFR");
  if (md) pofv->write_final(out / "pofv.dat", "VELOCITY", "POFV");
  // Final state for RESTART_FROM. MD keeps the pair (r(t+dt), r(t)) position Verlet needs.
  sys.write_xyz(out / "config.xyz", sys.pos);
  if (md) sys.write_xyz(out / "conf-1.xyz", sys.pos_old);
  rnd.save_state(out / "seed.out");

  std::println("{}: {} blocks x {} steps", md ? "LJ molecular dynamics (NVE)" : "LJ Monte Carlo (NVT)",
               setup.nblocks, setup.nsteps);
  print_summary("U/N", potential);
  if (md) {
    print_summary("K/N", *kinetic);
    print_summary("E/N", *total);
    print_summary("T", *temperature);
  }
  print_summary("P", pressure);
}
