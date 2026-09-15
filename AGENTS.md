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
- `simulator/NSL_SIMULATOR/`: the professor's starter code (C++11 + Armadillo).
  Walk-throughs in `simulator/LSN_lecture_0{4,6,7}_code.pdf`. `._*` files are
  macOS junk; ignore them.

## Deliverable layout (tracked)
```
common/                 NSL RNG (random.h/.cpp, Primes, seed.in) + blocking helper,
                        shared by every standalone C++ exercise
NSL_SIMULATOR/          tracked copy of the professor's simulator, evolved across
                        groups 04, 06, 07 (SOURCE/, INPUT/, OUTPUT/)
exNN/                   one directory per group, NN = 01..12
  Makefile, *.cpp, *.h  standalone C++ code (01–03, 05, 08–10)
  input/                parameter files read by the program (no hard-coded params)
  data/                 outputs the notebook reads (commit them)
  exNN.ipynb            the report for group NN
README.md               compile & run instructions for every group (course requirement)
```
The simulator is a single evolving copy. Features added for later groups (Gibbs
sampling, tail corrections, g(r)) must not break earlier ones (p(v) from 04).
Keep one input set per run under `exNN/input/` and copy results into
`exNN/data/`. The simulator hard-codes the relative paths `../INPUT` and
`../OUTPUT`, so run it from `NSL_SIMULATOR/SOURCE/`.

## Toolchain (checked 2026-09-15)
- `g++` 13.3. No cmake: use plain Makefiles like the professor's. New code
  builds with `-O3 -std=c++17 -Wall -Wextra`.
- **Armadillo is not installed**, and the simulator links `-larmadillo`.
  Install with `sudo apt install libarmadillo-dev`.
- **MPI is not installed**, and group 10 needs it. Install with
  `sudo apt install libopenmpi-dev openmpi-bin`.
- `.venv/` (Python 3.12) has **only Jupyter**. It still needs `numpy scipy
  matplotlib`, plus `tensorflow pillow` for 11–12. Install with
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
- **RNG: always the NSL generator** (`Random::Rannyu`, `Gauss`), seeded as the
  simulator does it: a pair from `Primes` plus 4 ints from `seed.in` go into
  `SetRandom(seed, p1, p2)`, and `SaveSeed()` runs at the end. No `<random>`, no
  `std::mt19937`, no `std::*_distribution`. Ex 01.1 tests this generator and
  01.2 extends it (exponential and Cauchy by inversion), and later exercises
  build on it. New distributions are sampled by hand (inversion or
  accept–reject), because implementing the sampling is the point. MPI ranks
  (group 10) each take a different `Primes` line.
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
  and the notebook reads from there. `NSL_SIMULATOR.cpp` currently calls
  `write_XYZ` **every step** (the `j%50` guard is commented out), which floods
  `OUTPUT/CONFIG/`. Disable it before production runs.

## Exercise map
| Group | Topic | Code base | Exact reference to check against |
|---|---|---|---|
| 01 | RNG tests, χ², CLT (std/exp/Cauchy dice), Buffon π | standalone | `<r>=1/2`, `σ²=1/12`, χ²≈100 per 100 bins, π |
| 02 | MC integral (uniform + importance sampling); 3D RW lattice/continuum | standalone | `I=1`; `√<r²> ∝ √N` |
| 03 | Black–Scholes call/put, direct and discretized GBM | standalone | C = 14.9758, P = 5.4595 |
| 04 | MD: p(v) histogram; fcc half-box + δ-velocity start; time reversal | NSL_SIMULATOR | Maxwell–Boltzmann at T_eff |
| 05 | Metropolis sampling of H 1s, 2p (uniform & Gaussian T) | standalone | `<r>` = 1.5 a₀, 5 a₀ |
| 06 | 1D Ising: add Gibbs; U, C, χ, M(h=0.02); N=50, J=1, T∈[0.5,2] | NSL_SIMULATOR | exact 1D formulas in statement |
| 07 | tail corrections; U/N autocorrelation; error vs L; g(r); MC NVT vs MD NVE, ρ*=0.8 T*=1.1 r_c=2.5 | NSL_SIMULATOR | MC ↔ MD agreement |
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
- **Inside the professor's simulator, match his style.** It uses C++11,
  Armadillo `vec`/`field`, `using namespace` in headers, and `_member` names.
  Keep it recognizable to him; don't modernize or refactor it. New standalone
  code follows the global C++17 conventions (RAII, no owning raw pointers).
- **Params come from input files**, not recompiles. A run is reproducible from
  `input/` + seed.
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
