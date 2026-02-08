#include <iostream>
#include <fstream>
#include <iomanip>
#include <unordered_set>
#include <cmath>
#include <chrono>
#include "hyperloglog.h"
#include "hyperloglog_improved.h"
#include "stream_generator.h"
#include "hash_function.h"

struct ExperimentResult {
    size_t step;
    size_t true_count;
    double hll_standard;
    double hll_improved;
    double hll_compact;
};

std::vector<ExperimentResult> runComparison(
    const std::vector<std::string>& stream,
    HyperLogLog& hll_std,
    HyperLogLogImproved& hll_imp,
    HyperLogLogCompact& hll_cmp,
    const HashFuncGen& hash_func,
    double step_percentage) {
    
    std::vector<ExperimentResult> results;
    std::unordered_set<std::string> unique_set;
    
    size_t step_size = static_cast<size_t>(stream.size() * step_percentage);
    if (step_size == 0) step_size = 1;
    
    for (size_t i = 0; i < stream.size(); ++i) {
        const auto& item = stream[i];
        uint32_t hash_val = hash_func.hash(item);
        
        unique_set.insert(item);
        hll_std.add(hash_val);
        hll_imp.add(hash_val);
        hll_cmp.add(hash_val);
        
        if ((i + 1) % step_size == 0 || i == stream.size() - 1) {
            ExperimentResult result;
            result.step = i + 1;
            result.true_count = unique_set.size();
            result.hll_standard = hll_std.estimate();
            result.hll_improved = hll_imp.estimate();
            result.hll_compact = hll_cmp.estimate();
            results.push_back(result);
        }
    }
    
    return results;
}

void saveComparisonResults(const std::string& filename, 
                           const std::vector<std::vector<ExperimentResult>>& all_results) {
    std::ofstream file(filename);
    file << "experiment,step,true_count,hll_standard,hll_improved,hll_compact\n";
    
    for (size_t exp = 0; exp < all_results.size(); ++exp) {
        for (const auto& result : all_results[exp]) {
            file << exp << "," 
                 << result.step << "," 
                 << result.true_count << ","
                 << std::fixed << std::setprecision(2) 
                 << result.hll_standard << ","
                 << result.hll_improved << ","
                 << result.hll_compact << "\n";
        }
    }
}

int main() {
    const uint32_t B = 10;
    const size_t num_experiments = 10;
    const size_t stream_size = 100000;
    const double step_percentage = 0.05;
    
    std::cout << "========================================" << std::endl;
    std::cout << "  Сравнение версий HyperLogLog" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Параметр B: " << B << " (регистров: " << (1 << B) << ")" << std::endl;
    std::cout << "Размер потока: " << stream_size << std::endl;
    std::cout << "Количество экспериментов: " << num_experiments << std::endl;
    std::cout << "Теоретическая погрешность: " << (1.04 / std::sqrt(1 << B)) * 100 << "%" << std::endl;
    std::cout << std::endl;
    
    RandomStreamGen stream_gen;
    HashFuncGen hash_func = HashFuncGen::random();
    
    HyperLogLog hll_test(B);
    HyperLogLogImproved hll_imp_test(B);
    HyperLogLogCompact hll_cmp_test(B);
    
    std::cout << "Потребление памяти:" << std::endl;
    std::cout << "  Стандартный:    " << (1 << B) << " байт" << std::endl;
    std::cout << "  Улучшенный:     " << hll_imp_test.getMemoryUsage() << " байт" << std::endl;
    std::cout << "  Компактный:     " << hll_cmp_test.getMemoryUsage() << " байт" << std::endl;
    std::cout << "  Экономия (компактный): " 
              << std::fixed << std::setprecision(1)
              << (1.0 - static_cast<double>(hll_cmp_test.getMemoryUsage()) / (1 << B)) * 100 
              << "%" << std::endl << std::endl;
    
    std::vector<std::vector<ExperimentResult>> all_results;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    for (size_t exp = 0; exp < num_experiments; ++exp) {
        std::cout << "Эксперимент " << (exp + 1) << "/" << num_experiments << "... ";
        std::cout.flush();
        
        auto stream = stream_gen.generateStream(stream_size);
        HyperLogLog hll_std(B);
        HyperLogLogImproved hll_imp(B);
        HyperLogLogCompact hll_cmp(B);
        
        auto results = runComparison(stream, hll_std, hll_imp, hll_cmp, hash_func, step_percentage);
        all_results.push_back(results);
        
        std::cout << "✓" << std::endl;
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "\nВремя выполнения: " << duration.count() / 1000.0 << " сек" << std::endl;
    
    saveComparisonResults("comparison_results.csv", all_results);
    std::cout << "Результаты сохранены в comparison_results.csv" << std::endl;
    
    size_t num_steps = all_results[0].size();
    std::vector<double> mean_std(num_steps, 0.0);
    std::vector<double> mean_imp(num_steps, 0.0);
    std::vector<double> mean_cmp(num_steps, 0.0);
    std::vector<double> std_std(num_steps, 0.0);
    std::vector<double> std_imp(num_steps, 0.0);
    std::vector<double> std_cmp(num_steps, 0.0);
    std::vector<size_t> true_counts(num_steps, 0);
    
    for (size_t step = 0; step < num_steps; ++step) {
        for (size_t exp = 0; exp < num_experiments; ++exp) {
            mean_std[step] += all_results[exp][step].hll_standard;
            mean_imp[step] += all_results[exp][step].hll_improved;
            mean_cmp[step] += all_results[exp][step].hll_compact;
            if (exp == 0) true_counts[step] = all_results[exp][step].true_count;
        }
        mean_std[step] /= num_experiments;
        mean_imp[step] /= num_experiments;
        mean_cmp[step] /= num_experiments;
    }
    
    for (size_t step = 0; step < num_steps; ++step) {
        for (size_t exp = 0; exp < num_experiments; ++exp) {
            std_std[step] += std::pow(all_results[exp][step].hll_standard - mean_std[step], 2);
            std_imp[step] += std::pow(all_results[exp][step].hll_improved - mean_imp[step], 2);
            std_cmp[step] += std::pow(all_results[exp][step].hll_compact - mean_cmp[step], 2);
        }
        std_std[step] = std::sqrt(std_std[step] / num_experiments);
        std_imp[step] = std::sqrt(std_imp[step] / num_experiments);
        std_cmp[step] = std::sqrt(std_cmp[step] / num_experiments);
    }
    
    std::ofstream stats_file("comparison_statistics.csv");
    stats_file << "step,true_count,mean_std,std_std,error_std,mean_imp,std_imp,error_imp,mean_cmp,std_cmp,error_cmp\n";
    
    for (size_t i = 0; i < num_steps; ++i) {
        double err_std = std::abs(mean_std[i] - true_counts[i]) / true_counts[i] * 100;
        double err_imp = std::abs(mean_imp[i] - true_counts[i]) / true_counts[i] * 100;
        double err_cmp = std::abs(mean_cmp[i] - true_counts[i]) / true_counts[i] * 100;
        
        stats_file << (i + 1) << ","
                   << true_counts[i] << ","
                   << std::fixed << std::setprecision(2)
                   << mean_std[i] << "," << std_std[i] << "," << err_std << ","
                   << mean_imp[i] << "," << std_imp[i] << "," << err_imp << ","
                   << mean_cmp[i] << "," << std_cmp[i] << "," << err_cmp << "\n";
    }
    stats_file.close();
    
    std::cout << "Статистика сохранена в comparison_statistics.csv" << std::endl;
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "  Сводные результаты" << std::endl;
    std::cout << "========================================" << std::endl;
    
    double avg_err_std = 0, avg_err_imp = 0, avg_err_cmp = 0;
    double max_err_std = 0, max_err_imp = 0, max_err_cmp = 0;
    double avg_std_std = 0, avg_std_imp = 0, avg_std_cmp = 0;
    
    for (size_t i = 0; i < num_steps; ++i) {
        double err_std = std::abs(mean_std[i] - true_counts[i]) / true_counts[i] * 100;
        double err_imp = std::abs(mean_imp[i] - true_counts[i]) / true_counts[i] * 100;
        double err_cmp = std::abs(mean_cmp[i] - true_counts[i]) / true_counts[i] * 100;
        
        avg_err_std += err_std;
        avg_err_imp += err_imp;
        avg_err_cmp += err_cmp;
        
        max_err_std = std::max(max_err_std, err_std);
        max_err_imp = std::max(max_err_imp, err_imp);
        max_err_cmp = std::max(max_err_cmp, err_cmp);
        
        avg_std_std += std_std[i] / true_counts[i] * 100;
        avg_std_imp += std_imp[i] / true_counts[i] * 100;
        avg_std_cmp += std_cmp[i] / true_counts[i] * 100;
    }
    
    avg_err_std /= num_steps;
    avg_err_imp /= num_steps;
    avg_err_cmp /= num_steps;
    avg_std_std /= num_steps;
    avg_std_imp /= num_steps;
    avg_std_cmp /= num_steps;
    
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "\nТочность (погрешность):" << std::endl;
    std::cout << "  Стандартный:  avg=" << avg_err_std << "%, max=" << max_err_std << "%" << std::endl;
    std::cout << "  Улучшенный:   avg=" << avg_err_imp << "%, max=" << max_err_imp << "%";
    if (avg_err_imp < avg_err_std) {
        std::cout << "(улучшение на " << ((avg_err_std - avg_err_imp) / avg_err_std * 100) << "%)";
    }
    std::cout << std::endl;
    std::cout << "  Компактный:   avg=" << avg_err_cmp << "%, max=" << max_err_cmp << "%" << std::endl;
    
    std::cout << "\nСтабильность (относительное σ):" << std::endl;
    std::cout << "  Стандартный:  " << avg_std_std << "%" << std::endl;
    std::cout << "  Улучшенный:   " << avg_std_imp << "%";
    if (avg_std_imp < avg_std_std) {
        std::cout << " (улучшение на " << ((avg_std_std - avg_std_imp) / avg_std_std * 100) << "%)";
    }
    std::cout << std::endl;
    std::cout << "  Компактный:   " << avg_std_cmp << "%" << std::endl;
    
    std::cout << "\nТеоретические пределы:" << std::endl;
    std::cout << "  1.04/√(2^B) = " << (1.04 / std::sqrt(1 << B)) * 100 << "%" << std::endl;
    std::cout << "  1.3/√(2^B)  = " << (1.3 / std::sqrt(1 << B)) * 100 << "%" << std::endl;
    
    std::cout << "\nЭксперимент завершен успешно!" << std::endl;
    
    return 0;
}
