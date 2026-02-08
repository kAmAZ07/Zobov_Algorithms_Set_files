#include <iostream>
#include <fstream>
#include <iomanip>
#include <unordered_set>
#include <cmath>
#include "hyperloglog.h"
#include "stream_generator.h"
#include "hash_function.h"

struct ExperimentResult {
    size_t step;
    size_t true_count;
    double hll_estimate;
};

std::vector<ExperimentResult> runExperiment(
    const std::vector<std::string>& stream,
    HyperLogLog& hll,
    const HashFuncGen& hash_func,
    double step_percentage) {
    
    std::vector<ExperimentResult> results;
    std::unordered_set<std::string> unique_set;
    
    size_t step_size = static_cast<size_t>(stream.size() * step_percentage);
    if (step_size == 0) step_size = 1;
    
    for (size_t i = 0; i < stream.size(); ++i) {
        const auto& item = stream[i];
        unique_set.insert(item);
        hll.add(hash_func.hash(item));
        
        if ((i + 1) % step_size == 0 || i == stream.size() - 1) {
            ExperimentResult result;
            result.step = i + 1;
            result.true_count = unique_set.size();
            result.hll_estimate = hll.estimate();
            results.push_back(result);
        }
    }
    
    return results;
}

void saveResults(const std::string& filename, 
                 const std::vector<std::vector<ExperimentResult>>& all_results) {
    std::ofstream file(filename);
    file << "experiment,step,true_count,hll_estimate\n";
    
    for (size_t exp = 0; exp < all_results.size(); ++exp) {
        for (const auto& result : all_results[exp]) {
            file << exp << "," 
                 << result.step << "," 
                 << result.true_count << "," 
                 << std::fixed << std::setprecision(2) << result.hll_estimate << "\n";
        }
    }
}

int main() {
    const uint32_t B = 10;
    const size_t num_experiments = 10;
    const size_t stream_size = 100000;
    const double step_percentage = 0.05;
    
    std::cout << "=== HyperLogLog Эксперимент ===" << std::endl;
    std::cout << "Параметр B: " << B << " (регистров: " << (1 << B) << ")" << std::endl;
    std::cout << "Теоретическая погрешность: " << (1.04 / std::sqrt(1 << B)) * 100 << "%" << std::endl;
    std::cout << "Размер потока: " << stream_size << std::endl;
    std::cout << "Количество экспериментов: " << num_experiments << std::endl;
    std::cout << std::endl;
    
    RandomStreamGen stream_gen;
    HashFuncGen hash_func = HashFuncGen::random();
    
    std::vector<std::vector<ExperimentResult>> all_results;
    
    for (size_t exp = 0; exp < num_experiments; ++exp) {
        std::cout << "Эксперимент " << (exp + 1) << "/" << num_experiments << "..." << std::endl;
        
        auto stream = stream_gen.generateStream(stream_size);
        HyperLogLog hll(B);
        
        auto results = runExperiment(stream, hll, hash_func, step_percentage);
        all_results.push_back(results);
    }
    
    saveResults("results.csv", all_results);
    std::cout << "\nРезультаты сохранены в results.csv" << std::endl;
    
    size_t num_steps = all_results[0].size();
    std::vector<double> mean_estimates(num_steps, 0.0);
    std::vector<double> std_estimates(num_steps, 0.0);
    std::vector<size_t> true_counts(num_steps, 0);
    
    for (size_t step = 0; step < num_steps; ++step) {
        for (size_t exp = 0; exp < num_experiments; ++exp) {
            mean_estimates[step] += all_results[exp][step].hll_estimate;
            if (exp == 0) true_counts[step] = all_results[exp][step].true_count;
        }
        mean_estimates[step] /= num_experiments;
        
        for (size_t exp = 0; exp < num_experiments; ++exp) {
            double diff = all_results[exp][step].hll_estimate - mean_estimates[step];
            std_estimates[step] += diff * diff;
        }
        std_estimates[step] = std::sqrt(std_estimates[step] / num_experiments);
    }
    
    std::ofstream stats_file("statistics.csv");
    stats_file << "step,true_count,mean_estimate,std_estimate,relative_error\n";
    for (size_t i = 0; i < num_steps; ++i) {
        double rel_error = std::abs(mean_estimates[i] - true_counts[i]) / true_counts[i] * 100;
        stats_file << (i + 1) << ","
                   << true_counts[i] << ","
                   << std::fixed << std::setprecision(2) << mean_estimates[i] << ","
                   << std_estimates[i] << ","
                   << rel_error << "\n";
    }
    std::cout << "Статистика сохранена в statistics.csv" << std::endl;
    
    return 0;
}
