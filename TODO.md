# TODO

A group is done only when it passes the "Done" checklist in `AGENTS.md`: clean
build, run command regenerates `data/`, notebook executes top to bottom, every
red-font figure present, every question answered, results match the reference.

## Setup
- [x] `uv pip install --python .venv/bin/python numpy scipy matplotlib`
- [x] Install MPI (`libopenmpi-dev openmpi-bin`, Open MPI 4.1.6)
- [ ] `uv pip install --python .venv/bin/python tensorflow` (for 11–12)
- [x] Move the professor's simulator into tracked `NSL_SIMULATOR/` (reference code, `._*` files dropped)
- [ ] Copy NSL RNG into `common/` (random.h/.cpp, Primes, seed.in) + blocking helper

## Own simulator (for 04, 06, 07; user writes it)
- [ ] Decide: own simulator or extend the professor's; directory name; Armadillo or not
- [ ] Install Armadillo only if the chosen simulator needs it (`sudo apt install libarmadillo-dev`)
- [ ] MD (Verlet, NVE) + MC (Metropolis, NVT) for Lennard-Jones, Metropolis + Gibbs for 1D Ising
- [ ] Blocking + input-file driven parameters + restart from a saved configuration
- [ ] Cross-check against `NSL_SIMULATOR/` on the same input

## 01 — RNG, CLT, Buffon
- [ ] 01.1.1 `<r>` with blocking vs #throws
- [ ] 01.1.2 `σ²` with blocking vs #throws
- [ ] 01.1.3 χ² test: 100 bins, 100 × 10⁴ numbers; plot χ²_j + histogram vs χ² distribution
- [ ] 01.2 add exponential + Cauchy (inversion) to RNG; S_N histograms N=1,2,10,100 for 3 dice; fits (Gauss / Cauchy)
- [ ] 01.3 Buffon π with blocking, without using π
- [ ] Answer: limits on the choice of N

## 02 — MC integration, random walks
- [ ] 02.1 `I` uniform sampling vs importance sampling, blocking
- [ ] 02.2 3D RW lattice + continuum, √⟨|r|²⟩ vs step 0..100 with errors; fit k√N; comment on diffusion

## 03 — Black–Scholes
- [ ] Call/put direct S(T) sampling, blocking
- [ ] Call/put discretized GBM (100 steps), blocking
- [ ] 4 figures vs analytic C = 14.9758, P = 5.4595

## 04 — MD, Maxwell–Boltzmann (simulator)
- [ ] 04.1 p(v*) histogram with blocking inside the simulator
- [ ] 04.2 gas ρ*=0.05, T*≈2, r_c=5: fcc half-box + δ-velocity start; convergence to MB at T_eff
- [ ] 04.2 comment on entropy and time direction
- [ ] 04.3 time reversal from ~5×10³ and ~5×10⁴ steps; U and T per block both directions

## 05 — Hydrogen Metropolis (master students)
- [ ] ⟨r⟩ for ψ₁₀₀ and ψ₂₁₀, uniform T(x|y), 50% acceptance, M ≥ 10⁶
- [ ] Equilibration; far-from-origin start
- [ ] Block size choice
- [ ] Gaussian T(x|y): equivalent results?
- [ ] 3D scatter of sampled points

## 06 — 1D Ising (simulator)
- [ ] Implement Gibbs sampler; verify restart from a spin configuration
- [ ] Add C, χ, M measurements
- [ ] U, C, χ (h=0), M (h=0.02) vs T ∈ [0.5, 2], N=50, J=1; Metropolis and Gibbs vs exact curves

## 07 — LJ MC vs MD (simulator)
- [ ] 07.1 tail corrections for U and P
- [ ] 07.2 find MD starting T that equilibrates at T*=1.1; MC step for 50% acceptance
- [ ] 07.2 instantaneous U/N, 5×10⁵ steps MC + MD (restart after equilibration)
- [ ] 07.2 autocorrelation of U/N
- [ ] 07.2 error vs block size L=10..5×10³; comment
- [ ] 07.3 g(r) on [0, L/2] with blocking, separate output file
- [ ] 07.4 liquid ρ*=0.8, T*=1.1, r_c=2.5: U/N, P, g(r), MC NVT vs MD NVE

## 08 — Variational Monte Carlo
- [ ] 08.1 analytic kinetic term for the trial ψ; VMC with Metropolis + blocking
- [ ] 08.2 simulated annealing on (σ, μ); ⟨H⟩ vs SA step
- [ ] ⟨H⟩ vs blocks at optimal (σ, μ)
- [ ] Sampled |ψ_T|² histogram vs analytic vs numerical Schrödinger solution (confirm E₀ ≈ −0.46)

## 09 — TSP genetic algorithm
- [ ] Data structures, starting population, `check()` on every individual
- [ ] Selection operator
- [ ] Mutation operators (pair swap, shift, block permutation, inversion); mutation-only random search check
- [ ] Order crossover
- [ ] 34 cities on a circumference: L⁽¹⁾ best + best-half average vs generation; best path plot
- [ ] 34 cities inside a square: same figures

## 10 — Parallel TSP (MPI)
- [ ] **Decide with user:** option 1 (continents + migration) or option 2 (parallel tempering)
- [ ] MPI implementation, one `Primes` line per rank
- [ ] 110 Italian capitals (`cap_prov_ita.dat`): best path plot + L vs generation
- [ ] Comparison asked in 10.2 (vs independent searches / vs serial GA)

## 11 — Keras NN regression (Python, `_new` notebook)
- [ ] 11.1 linear fit vs N_epochs, N_train, σ
- [ ] 11.2 cubic 4−3x−2x²+3x³: layers, neurons, activation, optimizer, loss; test in and outside [−1, 1]
- [ ] 11.2 summary: complexity vs fit vs prediction
- [ ] 11.3 sin(x²+y²) on [−3/2, 3/2]²

## 12 — MNIST DNN/CNN (Python, `_new` notebook)
- [ ] 12.1 more epochs; ≥ 2 optimizers besides SGD; loss/accuracy train vs validation; comment
- [ ] 12.2 complete `create_CNN()` (Conv2D, MaxPooling2D, Dropout, Flatten, Dense, softmax)
- [ ] 12.3 **user provides** 10 handwritten digit images; test the CNN on them

## Submission
- [ ] README.md: compile & run instructions for every group
- [ ] All notebooks committed executed with outputs
- [ ] Push to GitHub ~1 week before the exam
