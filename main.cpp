#include "random_stream_gen.h"
#include "hash_func_gen.h"
#include "hyperloglog.h"
#include "exact_counter.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <iomanip>
#include <numeric>
#include <unordered_set>

struct StepResult {
  double fraction;
  size_t streamSize;
  size_t exactCount;
  double hllEstimate;
  double relativeError;
};

struct StreamResults {
  uint64_t seed;
  size_t uniquePool;
  size_t totalSize;
  std::vector<StepResult> steps;
};

StreamResults runExperiment(size_t uniquePool, size_t totalSize, uint64_t streamSeed, int B, uint32_t hashSeed,
                            double fractionStep) {
  StreamResults results;
  results.seed = streamSeed;
  results.uniquePool = uniquePool;
  results.totalSize = totalSize;

  RandomStreamGen gen(uniquePool, totalSize, streamSeed);
  gen.generate();

  HashFuncGen hashGen(hashSeed);
  auto hashFunc = hashGen.getHashFunc();

  auto fractions = RandomStreamGen::makeFractions(fractionStep);

  for (double frac : fractions) {
    auto prefix = gen.getPrefix(frac);

    size_t exact = ExactCounter::count(prefix);

    HyperLogLog hll(B, hashFunc);
    for (const auto& s : prefix) {
      hll.add(s);
    }
    double est = hll.estimate();

    double relErr = (exact > 0) ? std::abs(est - static_cast<double>(exact)) / exact : 0.0;

    StepResult step;
    step.fraction = frac;
    step.streamSize = prefix.size();
    step.exactCount = exact;
    step.hllEstimate = est;
    step.relativeError = relErr;

    results.steps.push_back(step);
  }

  return results;
}

void saveResultsCSV(const std::string& filename, const std::vector<StreamResults>& allResults) {
  std::ofstream out(filename);
  out << "stream_seed,fraction,stream_size,exact,hll_estimate,relative_error\n";

  for (const auto& sr : allResults) {
    for (const auto& step : sr.steps) {
      out << sr.seed << "," << std::fixed << std::setprecision(4) << step.fraction << "," << step.streamSize << ","
          << step.exactCount << "," << std::fixed << std::setprecision(2) << step.hllEstimate << "," << std::fixed
          << std::setprecision(6) << step.relativeError << "\n";
    }
  }
  out.close();
}

void computeStatistics(const std::vector<StreamResults>& allResults, const std::string& filename) {
  if (allResults.empty())
    return;

  size_t numSteps = allResults[0].steps.size();
  size_t numStreams = allResults.size();

  std::ofstream out(filename);
  out << "fraction,mean_exact,mean_estimate,std_estimate,mean_relative_error\n";

  for (size_t s = 0; s < numSteps; ++s) {
    double sumExact = 0, sumEst = 0, sumRelErr = 0;
    std::vector<double> estimates;

    for (size_t i = 0; i < numStreams; ++i) {
      const auto& step = allResults[i].steps[s];
      sumExact += step.exactCount;
      sumEst += step.hllEstimate;
      sumRelErr += step.relativeError;
      estimates.push_back(step.hllEstimate);
    }

    double meanExact = sumExact / numStreams;
    double meanEst = sumEst / numStreams;
    double meanRelErr = sumRelErr / numStreams;

    // Стандартное отклонение оценки
    double variance = 0;
    for (double e : estimates) {
      variance += (e - meanEst) * (e - meanEst);
    }
    variance /= numStreams;
    double stdDev = std::sqrt(variance);

    out << std::fixed << std::setprecision(4) << allResults[0].steps[s].fraction << "," << std::fixed
        << std::setprecision(2) << meanExact << "," << std::fixed << std::setprecision(2) << meanEst << ","
        << std::fixed << std::setprecision(2) << stdDev << "," << std::fixed << std::setprecision(6) << meanRelErr
        << "\n";
  }
  out.close();
}

void investigateB(size_t uniquePool, size_t totalSize, const std::vector<int>& bValues, int numStreams,
                  const std::string& filename) {
  std::ofstream out(filename);
  out << "B,m,mean_relative_error,std_relative_error,theoretical_1042,theoretical_132\n";

  for (int B : bValues) {
    int m = 1 << B;
    std::vector<double> relErrors;

    for (int i = 0; i < numStreams; ++i) {
      RandomStreamGen gen(uniquePool, totalSize, 1000 + i);
      gen.generate();

      HashFuncGen hashGen(42);
      HyperLogLog hll(B, hashGen.getHashFunc());

      for (const auto& s : gen.getStream()) {
        hll.add(s);
      }

      size_t exact = ExactCounter::count(gen.getStream());
      double est = hll.estimate();
      double relErr = std::abs(est - exact) / exact;
      relErrors.push_back(relErr);
    }

    double meanErr = std::accumulate(relErrors.begin(), relErrors.end(), 0.0) / relErrors.size();
    double varErr = 0;
    for (double e : relErrors) {
      varErr += (e - meanErr) * (e - meanErr);
    }
    varErr /= relErrors.size();
    double stdErr = std::sqrt(varErr);

    double theo1 = 1.04 / std::sqrt(m);
    double theo2 = 1.3 / std::sqrt(m);

    out << B << "," << m << "," << std::fixed << std::setprecision(6) << meanErr << "," << stdErr << "," << theo1 << ","
        << theo2 << "\n";

    std::cout << "B=" << B << " (m=" << m << "): "
              << "mean_err=" << std::fixed << std::setprecision(4) << meanErr << " std_err=" << stdErr
              << " theo_1.04/sqrt(m)=" << theo1 << "\n";
  }
  out.close();
}

int main() {
  std::cout << "=== HyperLogLog Research ===" << std::endl;

  const int B = 10;
  const uint32_t hashSeed = 42;
  const size_t uniquePool = 5000;
  const size_t totalSize = 20000;
  const int numStreams = 10;
  const double fractionStep = 0.1;

  std::cout << "\n--- Investigating B values ---\n";
  investigateB(uniquePool, totalSize, {4, 6, 8, 10}, numStreams, "results/b_investigation.csv");

  std::cout << "\n--- Main experiment (B=" << B << ") ---\n";

  std::vector<StreamResults> allResults;
  for (int i = 0; i < numStreams; ++i) {
    uint64_t seed = 100 + i;
    auto res = runExperiment(uniquePool, totalSize, seed, B, hashSeed, fractionStep);
    allResults.push_back(res);

    std::cout << "Stream " << i << " (seed=" << seed << "): "
              << "final exact=" << res.steps.back().exactCount << " estimate=" << std::fixed << std::setprecision(1)
              << res.steps.back().hllEstimate << " rel_err=" << std::setprecision(4) << res.steps.back().relativeError
              << "\n";
  }

  system("mkdir results 2>nul");

  saveResultsCSV("results/main_results.csv", allResults);
  computeStatistics(allResults, "results/statistics.csv");

  saveResultsCSV("results/single_stream.csv", {allResults[0]});

  std::ofstream single("results/single_stream_simple.csv");
  std::ofstream inter("results/intersection_simple.csv");

  single << "n,actual,estimated,error\n";
  inter << "alpha,actual,estimated,error\n";

  for (const auto& step : allResults[0].steps) {
    single << step.streamSize << "," << step.exactCount << "," << step.hllEstimate << "," << step.relativeError << "\n";
  }

  // Данные для intersection (усреднённые)
  if (!allResults.empty()) {
    size_t numSteps = allResults[0].steps.size();
    for (size_t s = 0; s < numSteps; ++s) {
      double sumExact = 0, sumEst = 0, sumErr = 0;
      for (size_t i = 0; i < allResults.size(); ++i) {
        const auto& step = allResults[i].steps[s];
        sumExact += step.exactCount;
        sumEst += step.hllEstimate;
        sumErr += step.relativeError;
      }
      double meanExact = sumExact / allResults.size();
      double meanEst = sumEst / allResults.size();
      double meanErr = sumErr / allResults.size();

      inter << allResults[0].steps[s].fraction << "," << meanExact << "," << meanEst << "," << meanErr << "\n";
    }
  }

  single.close();
  inter.close();

  std::cout << "\nResults saved to results/ directory.\n";
  std::cout << "Run 'python analysis/plot.py' to generate plots.\n";

  return 0;
}