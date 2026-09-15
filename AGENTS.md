# LSN — Laboratorio di Simulazione Numerica (a.a. 2025/26)

Exam deliverable for Prof. D.E. Galli's Numerical Simulation Laboratory (UniMi).
This repo is what the professor grades: working C++ codes plus one Jupyter
notebook per exercise group. **Living source of truth — keep it updated as
exercises get done.** Progress is tracked in `TODO.md`.

## The exam, in the professor's words (`lectures/LSN_lecture_00.pdf`, slide 13)
- Complete **all** the given exercises (groups 01–12).
- For each group, produce a Jupyter notebook that describes the results with
  description, comments and figures.
- About one week before the exam, submit the working codes on GitHub (commented,
  indented) **with detailed compile instructions**.
- He evaluates the repo, then admits to the oral: questions about *your
  exercises* and key concepts. The user must be able to defend every line and
  every number, so write code that can be explained at the oral.

Remote: `git@github.com:lorenzoliuzzo/LSN.git` (branch `master`). That is the
submission. Don't push unless asked.

## Language split — non-negotiable
- **Simulation = C++** (groups 01–10). Every number shown in a 01–10 notebook
  is produced by a compiled C++ program that writes a data file. The notebook
  only reads, plots, fits, and compares with exact results. Never re-implement
  the simulation in numpy inside a notebook, even when the statement's own
  example cell does (ex01's blocking cell, ex05's scatter cell are illustrations).
- **Analysis = Python** in the notebooks (numpy, scipy, matplotlib). Ex 07.2
  explicitly asks for the autocorrelation and error-vs-block-size analysis "with
  a python script". That is analysis of C++ output, so it belongs in the notebook.
- **Groups 11–12 are "Python Exercises" by course design** (TensorFlow/Keras NN
  regression, MNIST DNN/CNN). They stay in Python. Don't port them to C++.

## Course material (read-only, git-ignored)
`.gitignore` excludes `exercises/`, `lectures/`, `simulator/`, `.venv/`.
**Anything left there is not submitted.** Never edit those files in place; copy
them into the tracked tree first.

- `exercises/LSN_Exercises_NN.ipynb`: the statements. For 11 and 12 use the
  `_new` variants (Keras 3 API with explicit `Input` layer; `11_new` sets noise
  `sigma = 0.2`).
- `exercises/cap_prov_ita.dat`: 110 (longitude, latitude) pairs for ex 10.2.
  `prov_ita.txt` has the matching names, in the same order.
- `lectures/LSN_lecture_NN.pdf`: theory. `LSN_lecture_03_THM.pdf` (supplement to
  lecture 03) and `IsingformulationsofmanyNPproblems.pdf` are extra reading.
- `simulator/LSN_lecture_0{4,6,7}_code.pdf`: walk-throughs of the professor's
  simulator code. The code itself was moved to the tracked `NSL_SIMULATOR/`
  (see below).

## Deliverable layout (tracked)
```
src/                    shared C++ library (core/: RNG, blocking, Vec3, input parsing)
                        and the simulator used by 04, 06, 07 (lj/, ising/, app/); see below
NSL_SIMULATOR/          the professor's simulator (SOURCE/, INPUT/, OUTPUT/),
                        committed as shipped: reference code, see below
exNN/                   one directory per group, NN = 01..12
  Makefile, *.cpp, *.h  standalone C++ code (01–03, 05, 08–10). Reuses src/core by
                        compiling ../src/core/*.cpp with -I../src; don't copy it
  input/                parameter files read by the program (no hard-coded params)
  runs/<name>/          run directories for src/build/simulator (04, 06, 07);
                        their output/ is git-ignored
  data/                 outputs the notebook reads (commit them)
  exNN.ipynb            the report for group NN
README.md               compile & run instructions for every group (course requirement)
```
### The simulator: `src/` (modern sketch) and `NSL_SIMULATOR/` (the professor's)
- **`NSL_SIMULATOR/` is reference only.** Read it to see what the course expects
  (input format, measured properties, output files) and use it as a cross-check.
  Don't build on it or modify it.
- **`src/` is a from-scratch C++17 simulator** that Claude sketched at the
  user's request on 2026-09-15. It needs no Armadillo and already covers
  everything 04, 06, 07 ask for (p(v), Gibbs, C/χ/M, tail corrections, g(r),
  the low-entropy start, time reversal). The user may study it and rewrite parts
  of it to learn. Explain it, review any rewrite against it, and change it only
  when asked. Features added for later groups must not break earlier ones.

```
src/core/      Vec3, Random (course LCG or mt19937_64), BlockStats / ScalarObservable /
               BlockedHistogram, InputFile, io
src/lj/        LJSystem (box, minimum image, fcc, forces, virial, tails, g(r), xyz),
               Verlet step + Metropolis sweep, MD/MC driver
src/ising/     IsingChain (Metropolis, Gibbs), driver
src/app/       run setup (simulation type, blocks, restart, RNG) and main()
src/examples/  one run directory per case, sharing Primes and seed.in
```

- **Build and run:** `make -C src`, then `src/build/simulator <run_dir>`. It reads
  `<run_dir>/input.dat` and writes `<run_dir>/output/`.
- **Input keys:** the professor's (`SIMULATION_TYPE` 0 MD | 1 MC | `2 J h` Ising
  Metropolis | `3 J h` Ising Gibbs, `TEMP`, `NPART`, `RHO`, `R_CUT`, `DELTA`,
  `NBLOCKS`, `NSTEPS`) plus `NEQUIL`, `RESTART_FROM <dir>`,
  `RNG_ENGINE NSL|MT19937_64`, `PRIMES_FILE`, `PRIMES_LINE`, `SEED_FILE`,
  `GOFR_BINS`, `POFV_BINS`, `POFV_VMAX`, `INIT_LATTICE FCC|FCC_HALF`,
  `INIT_VELOCITIES GAUSS|DELTA`, `REVERSE_TIME`, `PRINT_INSTANT`, `XYZ_EVERY`.
  A key the run doesn't use is an error (it catches typos). The initial-state
  keys and `SEED_FILE` are still accepted on a restart, so continuing a run only
  means adding `RESTART_FROM`.
- **Outputs:** the professor's file names and `BLOCK ACTUAL AVE ERROR` columns,
  plus `gofr_blocks.dat`, `pofv_blocks.dat` and `instant.dat`. The final state
  (`config.xyz`, `conf-1.xyz` for MD, `config.spin`, `seed.out`) is what
  `RESTART_FROM` reads.
- **Verified 2026-09-15 on `src/examples/`** (re-run these checks after changing
  `src/`):
  - Random numbers are bit-identical to the professor's `Rannyu` over 10⁶ draws.
  - Ising at T = 1 matches the exact formulas within about 1.2σ with both
    samplers.
  - MD conserves E/N to 3·10⁻⁴.
  - MC acceptance is 47% at DELTA 0.11.
  - g(r) peaks at r = 1.09 and tends to 1.
  - p(v) integrates to 1 and approaches Maxwell–Boltzmann at T_eff.
  - MD time reversal returns to the start exact to 10 digits.
- **Deliberate differences from the professor's code**, worth knowing for the
  oral:
  - The block error divides by N−1; his divides by N.
  - MD measures U, K and E at the same time t; he measures U one step after the
    velocities.
  - MC pressure includes ρT. His MC runs have zero velocities, so that term is
    missing from his pressure.
  - Ising Metropolis picks sites at random. A sequential sweep accepts every
    ΔE = 0 flip with certainty, so antiferromagnetic stretches just shift one
    site per sweep and never relax; it gave U/N = +0.12 instead of −0.76 at T = 1.
  - Acceptance is reported per block, not accumulated over the run.

Runs for an exercise live in `exNN/runs/<name>/input.dat`, with `PRIMES_FILE`
and `SEED_FILE` pointing at shared copies. Copy the output files a notebook reads
into `exNN/data/`. The professor's code hard-codes the relative paths `../INPUT`
and `../OUTPUT`, so it runs from `NSL_SIMULATOR/SOURCE/`, and its `OUTPUT/` is
git-ignored.

## Toolchain (checked 2026-09-15)
- `g++` 13.3. No cmake: use plain Makefiles like the professor's. New code
  builds with `-O3 -std=c++17 -Wall -Wextra -Wpedantic`.
- **Armadillo is not installed and `src/` doesn't need it.** Only the professor's
  `NSL_SIMULATOR/` links `-larmadillo`. Install it (`sudo apt install
  libarmadillo-dev`, run by the user) only to build his code for a direct
  cross-check.
- MPI (group 10): Open MPI 4.1.6 (`mpicxx`, `mpirun`). The CPU has 10 physical
  cores / 16 hardware threads, and Open MPI counts cores, so 11 ranks need
  `mpirun --use-hwthread-cpus -np 11`. Build with `-DOMPI_SKIP_MPICXX`: Open MPI's
  deprecated C++ bindings header otherwise emits `-Wcast-function-type`
  warnings under `-Wextra`. Use the C API (`MPI_Init`, `MPI_Send`, ...).
- `sudo` needs the user's password and doesn't work from the `!` prefix, so
  the user runs system installs in their own terminal.
- `.venv/` (Python 3.12): Jupyter, `numpy`, `scipy`, `matplotlib`, `pillow`
  installed. Still needs `tensorflow` for 11–12. Install with
  `uv pip install --python .venv/bin/python ...` (not plain pip).
- System installs (`sudo apt`) and any Python package beyond the list above:
  ask first.
- Kernel: `python3` (the `.venv` one). Don't use `qsim`, which belongs to
  another project.
- Execute a notebook end to end:
  `.venv/bin/jupyter nbconvert --to notebook --execute --inplace exNN/exNN.ipynb`

## Course rules every exercise must honor
- **Professor's guideline #1 is graded:** code must be correct, simple, well
  organized, indented, **commented**, use self-explanatory names, and be
  numerically efficient. This overrides the global "no what-comments" rule:
  comment the physics and the algorithm (which quantity, which formula, which
  units, why this proposal or step size). Still don't narrate trivial C++.
- **RNG: always through the course `Random` interface** (`SetRandom`, `Rannyu`,
  `Gauss`, `SaveSeed`), seeded from input files, never from the clock or
  `std::random_device`, so every run is reproducible. MPI ranks (group 10) each
  take a different `Primes` line.
  - **Group 01 uses the course generator itself**: a 48-bit LCG with period
    2⁴⁷, where a pair from `Primes` plus 4 ints from `seed.in` go into
    `SetRandom(seed, p1, p2)`. Ex 01.1 tests it and 01.2 extends it
    (exponential and Cauchy by inversion).
  - **From group 02 on, the engine behind that interface may be a `<random>`
    engine.** Prefer `std::mt19937_64`; `std::ranlux48` is fine for
    cross-checks; never `std::minstd_rand` (31-bit LCG, worse than the course
    one). Each notebook states which engine was used.
  - **Never `std::*_distribution`.** Their algorithms are implementation-defined
    (libstdc++ and libc++ give different numbers for the same seed), and
    sampling by hand (inversion, accept–reject, Box–Muller) is the point. Map
    raw engine output to [0,1) yourself, e.g. `(x >> 11) * 0x1.0p-53` for a
    64-bit engine.
  - `std::linear_congruential_engine<std::uint64_t, 34522712143931ULL,
    p1*4096 + p2, 1ULL << 48>`, scaled by 2⁻⁴⁸, reproduces `Rannyu` bit for bit
    (checked for 10⁷ draws), so the two engines can be swapped for comparisons.
- **Data blocking is mandatory**, and a result without an uncertainty counts as
  incomplete. For M steps in N blocks, report progressive averages with error
  `sqrt((<A²> − <A>²)/(n−1))`, set to 0 for the first block, plotted with error
  bars versus the number of throws, blocks or steps. When an exact value
  exists, plot the deviation from it (as the professor does with `<r> − 1/2`).
- **Equilibrate before measuring** (05, 06, 07, 08), and show the equilibration
  in the notebook.
- **Metropolis acceptance ≈ 50%** wherever a step size is tunable (05, 07 MC,
  08). Report the acceptance you got.
- **Units:** LJ reduced (04, 07), Bohr radius a₀ (05), ħ = m = 1 (08),
  k_B = μ_B = 1 (06).
- **Required figures are the red-font items in each statement.** Every one
  must appear. **Every question in the statement gets an explicit written
  answer** in the notebook, e.g.:
  - 01: the limits on the choice of N
  - 04: entropy and time reversal
  - 05: starting far from the origin; block size; Gaussian vs uniform T(x|y)
  - 07.2: what the error vs L shows
  - 10.2: improvement vs independent searches
  - 11.2: model complexity vs generalization
- Long runs never happen inside a notebook. The C++ program writes to `data/`
  and the notebook reads from there. The professor's
  `NSL_SIMULATOR.cpp` calls `write_XYZ` **every step** (the `j%50` guard is
  commented out), which floods `OUTPUT/CONFIG/`. Disable it before any long run
  with that code.

## Exercise map
| Group | Topic | Code base | Exact reference to check against |
|---|---|---|---|
| 01 | RNG tests, χ², CLT (std/exp/Cauchy dice), Buffon π | standalone | `<r>=1/2`, `σ²=1/12`, χ²≈100 per 100 bins, π |
| 02 | MC integral (uniform + importance sampling); 3D RW lattice/continuum | standalone | `I=1`; `√<r²> ∝ √N` |
| 03 | Black–Scholes call/put, direct and discretized GBM | standalone | C = 14.9758, P = 5.4595 |
| 04 | MD: p(v) histogram; fcc half-box + δ-velocity start; time reversal | simulator | Maxwell–Boltzmann at T_eff |
| 05 | Metropolis sampling of H 1s, 2p (uniform & Gaussian T) | standalone | `<r>` = 1.5 a₀, 5 a₀ |
| 06 | 1D Ising: add Gibbs; U, C, χ, M(h=0.02); N=50, J=1, T∈[0.5,2] | simulator | exact 1D formulas in statement |
| 07 | tail corrections; U/N autocorrelation; error vs L; g(r); MC NVT vs MD NVE, ρ*=0.8 T*=1.1 r_c=2.5 | simulator | MC ↔ MD agreement |
| 08 | VMC 1D, V=x⁴−5/2x², trial ψ with σ, μ; simulated annealing | standalone | numerical diagonalization in statement; E₀ ≈ −0.46 |
| 09 | TSP genetic algorithm; 34 cities on a circle / in a square | standalone | circle: optimum is the perimeter order |
| 10 | GA parallelized with MPI; 110 Italian provincial capitals | standalone + MPI | vs serial / independent searches |
| 11 | Keras NN fits: linear, cubic, sin(x²+y²) | Python | — |
| 12 | Keras DNN/CNN on MNIST; own handwritten digits | Python | — |

Per-group notes:
- **05** is marked "only for master students". Treat it as required unless the
  user says otherwise.
- **09:** the first city is fixed at position 1. Write a `check()` that
  validates every individual (each city exactly once, first city fixed) and
  call it whenever an individual is created. Fitness is L⁽¹⁾. Verify the GA can
  do a mutation-only random search before adding crossover.
- **10.1** says "choose ONLY ONE": either continents with migration, or
  parallel tempering. **That choice is the user's. Ask before writing code.**
- **12.3** needs 10 digit images the user draws (the statement suggests gimp).
  Ask for them rather than generating them.

## Working method in this repo
- **Don't restyle the professor's simulator.** It uses C++11, Armadillo
  `vec`/`field`, `using namespace` in headers, and `_member` names. If it is
  ever extended, keep it recognizable to him. New code (`src/`, the exercises,
  any rewrite by the user) follows the global C++17 conventions (RAII, no owning
  raw pointers) and the style of `src/`.
- **Params come from input files**, not recompiles. A run is reproducible from
  its input file (`input.dat` or `exNN/input/`) plus the seed.
- **No unit-test framework here.** This overrides the global "always pytest"
  rule. Correctness is shown by agreement with exact or known results, within
  error bars, in the notebook, plus runtime checks where the statement asks for
  them (09's `check()`).
- **"Done" for a group means all of the following hold:**
  - `make` builds clean with no warnings.
  - The documented run command regenerates `data/`.
  - `nbconvert --execute` runs the notebook top to bottom.
  - Every red-font figure is present, every question is answered, and the
    results agree with the reference column above.

  Report which of these you actually ran.
- **Commit notebooks executed, with outputs.** GitHub renders the figures, and
  that is how they get read. Keep `data/` files small enough to commit: thin or
  aggregate rather than dumping every configuration.
- Notebook prose is in English (like the statements) unless the user asks
  otherwise. Each notebook opens with a short description of the method, then
  each sub-exercise gets: what was simulated, parameters, figure(s), comment.
- Commit per group: imperative subject, e.g. `Add exercise 03: Black-Scholes MC`.
