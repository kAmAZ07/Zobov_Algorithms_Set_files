#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include "ArrayGenerator.h"
#include "SortTester.h"

using namespace std;

const int MIN_SIZE = 500;
const int MAX_SIZE = 100000;

const vector<int> K_VALUES = {5, 10, 15, 20, 30, 50};

const map<ArrayGenerator::ArrayType, string> TYPE_NAMES = {
    {ArrayGenerator::RANDOM, "Random"},
    {ArrayGenerator::REVERSED, "Reversed"},
    {ArrayGenerator::NEARLY_SORTED, "NearlySorted"}
};

void processSize(int size, ArrayGenerator::ArrayType type, const string& typeName,
                 ArrayGenerator& generator, SortTester& tester, ofstream& outfile) {
    // 1. Генерация массива
    vector<long long> arr = generator.getArray(type, size);

    // 2. Тестирование Standard MERGE SORT
    long long time_std = tester.testStandardMergeSort(arr);
    outfile << size << "," << typeName << "," << "StandardMergeSort" << "," << 0 << "," << time_std << "\n";
    
    // 3. Тестирование Hybrid MERGE+INSERTION SORT с разными K
    for (int K : K_VALUES) {
        long long time_hybrid = tester.testHybridMergeInsertionSort(arr, K);
        outfile << size << "," << typeName << "," << "HybridMergeInsertionSort" << "," << K << "," << time_hybrid << "\n";
    }

    if (size % 10000 == 0) {
        cout << "  Processed size: " << size << endl;
    }
}

/**
 * @brief Основная функция для проведения эксперимента.
 */
void runExperiment() {
    ArrayGenerator generator;
    SortTester tester;

    // Файл для записи результатов
    ofstream outfile("experiment_results.csv");
    if (!outfile.is_open()) {
        cerr << "Error: Could not open experiment_results.csv for writing." << endl;
        return;
    }

    // Заголовок CSV файла
    outfile << "Size,ArrayType,Algorithm,K,Time_us\n";

    // Итерация по типам массивов
    for (const auto& pair : TYPE_NAMES) {
        ArrayGenerator::ArrayType type = pair.first;
        string typeName = pair.second;

        cout << "Running experiment for " << typeName << " arrays..." << endl;

        // Шаг 1: от 500 до 10000 с шагом 100
        for (int size = MIN_SIZE; size <= 10000; size += 100) {
            processSize(size, type, typeName, generator, tester, outfile);
        }

        // Шаг 2: от 15000 до 100000 с шагом 5000
        for (int size = 15000; size <= MAX_SIZE; size += 5000) {
            processSize(size, type, typeName, generator, tester, outfile);
        }
    }

    outfile.close();
    cout << "Experiment finished. Results saved to experiment_results.csv" << endl;
}

int main() {
    // Ускорение ввода/вывода
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    runExperiment();

    return 0;
}
