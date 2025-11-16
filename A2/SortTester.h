#pragma once

#include <iostream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <numeric>
#include "ArrayGenerator.h"

using namespace std;

// --- Функции сортировки, скопированные из A2i.cpp для использования в SortTester ---

/**
 * @brief Сортировка вставками (Insertion Sort) для подмассива.
 */
void insertionSort(vector<long long>& arr, int l, int r) {
    for (int i = l + 1; i <= r; i++) {
        long long key = arr[i];
        int j = i - 1;
        while (j >= l && arr[j] > key) {
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = key;
    }
}

/**
 * @brief Слияние двух отсортированных подмассивов.
 */
void merge(vector<long long>& arr, int l, int m, int r) {
    int n1 = m - l + 1;
    int n2 = r - m;

    // Создание временных массивов
    vector<long long> L(n1);
    vector<long long> R(n2);

    // Копирование данных во временные массивы L[] и R[]
    for (int i = 0; i < n1; i++)
        L[i] = arr[l + i];
    for (int j = 0; j < n2; j++)
        R[j] = arr[m + 1 + j];

    // Слияние временных массивов обратно в arr[l..r]
    int i = 0;
    int j = 0;
    int k = l;

    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            arr[k] = L[i];
            i++;
        } else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }

    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;
    }
}

/**
 * @brief Стандартный алгоритм MERGE SORT.
 */
void standardMergeSort(vector<long long>& arr, int l, int r) {
    if (l < r) {
        int m = l + (r - l) / 2;
        standardMergeSort(arr, l, m);
        standardMergeSort(arr, m + 1, r);
        merge(arr, l, m, r);
    }
}

/**
 * @brief Гибридный алгоритм MERGE+INSERTION SORT.
 */
void hybridMergeInsertionSort(vector<long long>& arr, int l, int r, int K) {
    if (l < r) {
        if (r - l + 1 <= K) {
            insertionSort(arr, l, r);
        } else {
            int m = l + (r - l) / 2;
            hybridMergeInsertionSort(arr, l, m, K);
            hybridMergeInsertionSort(arr, m + 1, r, K);
            merge(arr, l, m, r);
        }
    }
}

// Перегрузки для удобства вызова
void standardMergeSort(vector<long long>& arr) {
    if (arr.empty()) return;
    standardMergeSort(arr, 0, arr.size() - 1);
}

void hybridMergeInsertionSort(vector<long long>& arr, int K) {
    if (arr.empty()) return;
    hybridMergeInsertionSort(arr, 0, arr.size() - 1, K);
}

// --- Класс SortTester ---

/**
 * @brief Класс для проведения эмпирических замеров времени работы алгоритмов сортировки.
 */
class SortTester {
private:
    // Количество замеров для усреднения
    const int NUM_RUNS = 5;

    /**
     * @brief Проводит один замер времени работы алгоритма.
     * @param arr Массив для сортировки (будет изменен).
     * @param sortFunc Функция сортировки.
     * @return Время выполнения в микросекундах.
     */
    long long measureTime(vector<long long>& arr, void (*sortFunc)(vector<long long>&)) {
        auto start = chrono::high_resolution_clock::now();
        sortFunc(arr);
        auto end = chrono::high_resolution_clock::now();
        return chrono::duration_cast<chrono::microseconds>(end - start).count();
    }

    /**
     * @brief Проводит один замер времени работы гибридного алгоритма.
     * @param arr Массив для сортировки (будет изменен).
     * @param K Пороговое значение.
     * @return Время выполнения в микросекундах.
     */
    long long measureTimeHybrid(vector<long long>& arr, int K) {
        auto start = chrono::high_resolution_clock::now();
        hybridMergeInsertionSort(arr, K);
        auto end = chrono::high_resolution_clock::now();
        return chrono::duration_cast<chrono::microseconds>(end - start).count();
    }

    /**
     * @brief Вычисляет медиану из вектора замеров.
     */
    long long calculateMedian(vector<long long>& times) {
        if (times.empty()) return 0;
        sort(times.begin(), times.end());
        size_t size = times.size();
        if (size % 2 == 0) {
            return (times[size / 2 - 1] + times[size / 2]) / 2;
        } else {
            return times[size / 2];
        }
    }

public:
    /**
     * @brief Тестирует стандартный MERGE SORT.
     * @param originalArray Исходный массив для тестирования.
     * @return Медиана времени выполнения в микросекундах.
     */
    long long testStandardMergeSort(const vector<long long>& originalArray) {
        vector<long long> times;
        for (int i = 0; i < NUM_RUNS; ++i) {
            // Копируем массив для каждого замера, чтобы избежать сортировки уже отсортированного
            vector<long long> arr = originalArray;
            times.push_back(measureTime(arr, standardMergeSort));
        }
        return calculateMedian(times);
    }

    /**
     * @brief Тестирует гибридный MERGE+INSERTION SORT.
     * @param originalArray Исходный массив для тестирования.
     * @param K Пороговое значение.
     * @return Медиана времени выполнения в микросекундах.
     */
    long long testHybridMergeInsertionSort(const vector<long long>& originalArray, int K) {
        vector<long long> times;
        for (int i = 0; i < NUM_RUNS; ++i) {
            // Копируем массив для каждого замера
            vector<long long> arr = originalArray;
            times.push_back(measureTimeHybrid(arr, K));
        }
        return calculateMedian(times);
    }
};
