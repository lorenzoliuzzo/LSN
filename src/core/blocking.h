#pragma once

#include <filesystem>
#include <fstream>
#include <ostream>
#include <string>
#include <vector>

// Data blocking. The M measurements of a run are split into N blocks, and each block
// average A_k is one estimate. After n blocks the progressive estimate is the mean of
// A_1..A_n, and its uncertainty is the standard deviation of that mean,
// sqrt((<A^2> - <A>^2) / (n - 1)), set to 0 for n = 1 where it is undefined.
class BlockStats {
 public:
  void add_block(double block_value);
  int blocks() const { return n_; }
  double last() const { return last_; }
  double mean() const;
  double error() const;

 private:
  int n_ = 0;
  double sum_ = 0.0;
  double sum2_ = 0.0;
  double last_ = 0.0;
};

// One blocked scalar. Sums the measurements of the current block; at the end of each
// block it appends "BLOCK ACTUAL AVE ERROR" to its file, the NSL_SIMULATOR layout.
class ScalarObservable {
 public:
  ScalarObservable(const std::filesystem::path& file, const std::string& label);

  void add(double value);
  // Closes the block with the average of the values added since the last close.
  void close_block();
  // Closes the block with a value computed elsewhere, for quantities defined only per
  // block, such as fluctuations (heat capacity, susceptibility).
  void close_block(double block_value);

  const BlockStats& stats() const { return stats_; }

 private:
  std::ofstream out_;
  BlockStats stats_;
  double block_sum_ = 0.0;
  long block_count_ = 0;
};

// A distribution estimated with data blocking, e.g. g(r) or p(v). Counts accumulate
// during a block; at the end of the block every bin is divided by its normalization
// and the result enters that bin's own BlockStats.
class BlockedHistogram {
 public:
  BlockedHistogram(int bins, double x_max);

  int bins() const { return static_cast<int>(counts_.size()); }
  double width() const { return width_; }
  double lower(int bin) const { return bin * width_; }
  double center(int bin) const { return (bin + 0.5) * width_; }

  void fill(double x, double weight = 1.0);  // values outside [0, x_max) are dropped

  // normalization(bin) returns the number the block counts of that bin are divided by.
  template <class Normalization>
  void close_block(Normalization normalization);

  void write_block(std::ostream& out, int block) const;  // values of the last block
  void write_final(const std::filesystem::path& file, const std::string& x_label,
                   const std::string& y_label) const;

 private:
  double width_;
  std::vector<double> counts_;
  std::vector<BlockStats> stats_;
};

template <class Normalization>
void BlockedHistogram::close_block(Normalization normalization) {
  for (int bin = 0; bin < bins(); ++bin) {
    stats_[bin].add_block(counts_[bin] / normalization(bin));
    counts_[bin] = 0.0;
  }
}

// "name = mean +- error" on standard output, for the end-of-run summary.
void print_summary(const std::string& name, const ScalarObservable& observable);
