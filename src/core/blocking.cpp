#include "core/blocking.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <stdexcept>

#include "core/io.h"

namespace {
int checked_bins(int bins) {
  if (bins < 1) throw std::invalid_argument("a histogram needs at least one bin");
  return bins;
}
}  // namespace

void BlockStats::add_block(double block_value) {
  ++n_;
  sum_ += block_value;
  sum2_ += block_value * block_value;
  last_ = block_value;
}

double BlockStats::mean() const { return n_ > 0 ? sum_ / n_ : 0.0; }

double BlockStats::error() const {
  if (n_ < 2) return 0.0;
  const double average = sum_ / n_;
  // Round-off can push the variance slightly below zero when all blocks agree.
  const double variance = std::max(0.0, sum2_ / n_ - average * average);
  return std::sqrt(variance / (n_ - 1));
}

ScalarObservable::ScalarObservable(const std::filesystem::path& file, const std::string& label)
    : out_(open_output(file)) {
  out_ << '#' << std::setw(7) << "BLOCK:" << std::setw(20) << ("ACTUAL_" + label + ":") << std::setw(20)
       << (label + "_AVE:") << std::setw(20) << "ERROR:" << '\n'
       << std::setprecision(10);
}

void ScalarObservable::add(double value) {
  block_sum_ += value;
  ++block_count_;
}

void ScalarObservable::close_block() {
  if (block_count_ == 0) throw std::logic_error("closing a block that holds no measurements");
  close_block(block_sum_ / block_count_);
}

void ScalarObservable::close_block(double block_value) {
  stats_.add_block(block_value);
  block_sum_ = 0.0;
  block_count_ = 0;
  out_ << std::setw(8) << stats_.blocks() << std::setw(20) << block_value << std::setw(20) << stats_.mean()
       << std::setw(20) << stats_.error() << '\n';
}

BlockedHistogram::BlockedHistogram(int bins, double x_max)
    : width_(x_max / checked_bins(bins)),
      counts_(static_cast<std::size_t>(bins), 0.0),
      stats_(static_cast<std::size_t>(bins)) {
  if (x_max <= 0.0) throw std::invalid_argument("a histogram needs x_max > 0");
}

void BlockedHistogram::fill(double x, double weight) {
  if (x < 0.0) return;
  const auto bin = static_cast<std::size_t>(x / width_);
  if (bin < counts_.size()) counts_[bin] += weight;
}

void BlockedHistogram::write_block(std::ostream& out, int block) const {
  for (int bin = 0; bin < bins(); ++bin) {
    out << block << ' ' << center(bin) << ' ' << stats_[bin].last() << '\n';
  }
}

void BlockedHistogram::write_final(const std::filesystem::path& file, const std::string& x_label,
                                   const std::string& y_label) const {
  std::ofstream out = open_output(file);
  out << "# " << x_label << ": AVE_" << y_label << ": ERROR:\n" << std::setprecision(10);
  for (int bin = 0; bin < bins(); ++bin) {
    out << center(bin) << ' ' << stats_[bin].mean() << ' ' << stats_[bin].error() << '\n';
  }
}

void print_summary(const std::string& name, const ScalarObservable& observable) {
  std::cout << std::setw(8) << name << " = " << observable.stats().mean() << " +- "
            << observable.stats().error() << '\n';
}
