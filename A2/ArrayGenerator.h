#pragma once

#include <vector>
#include <random>
#include <algorithm>
#include <cmath>
#include <iostream>

using namespace std;

class ArrayGenerator {
private:
    const int MAX_SIZE = 100000;
    const int MIN_VAL = 0;
    const int MAX_VAL = 10000;

    vector<long long> masterRandomArray;
    vector<long long> masterReversedArray;
    vector<long long> masterNearlySortedArray;
    mt19937 rng;

    long long getRandomValue() {
        uniform_int_distribution<long long> dist(MIN_VAL, MAX_VAL);
        return dist(rng);
    }

    void generateMasterRandom() {
        masterRandomArray.resize(MAX_SIZE);
        for (int i = 0; i < MAX_SIZE; ++i) {
            masterRandomArray[i] = getRandomValue();
        }
    }

    void generateMasterReversed() {
        masterReversedArray.resize(MAX_SIZE);
        for (int i = 0; i < MAX_SIZE; ++i) {
            masterReversedArray[i] = i;
        }
        reverse(masterReversedArray.begin(), masterReversedArray.end());
    }

    void generateMasterNearlySorted() {
        masterNearlySortedArray.resize(MAX_SIZE);
        for (int i = 0; i < MAX_SIZE; ++i) {
            masterNearlySortedArray[i] = i;
        }

        int swaps = MAX_SIZE / 100;
        uniform_int_distribution<int> dist(0, MAX_SIZE - 1);

        for (int i = 0; i < swaps; ++i) {
            int idx1 = dist(rng);
            int idx2 = dist(rng);
            swap(masterNearlySortedArray[idx1], masterNearlySortedArray[idx2]);
        }
    }

public:
    ArrayGenerator() {
        rng.seed(random_device()());
        generateMasterRandom();
        generateMasterReversed();
        generateMasterNearlySorted();
    }

    enum ArrayType {
        RANDOM,
        REVERSED,
        NEARLY_SORTED
    };

    vector<long long> getArray(ArrayType type, int size) {
        if (size > MAX_SIZE) {
            throw out_of_range("Requested size exceeds MAX_SIZE.");
        }

        const vector<long long>* masterArray = nullptr;
        switch (type) {
            case RANDOM:
                masterArray = &masterRandomArray;
                break;
            case REVERSED:
                masterArray = &masterReversedArray;
                break;
            case NEARLY_SORTED:
                masterArray = &masterNearlySortedArray;
                break;
            default:
                throw invalid_argument("Invalid ArrayType.");
        }

        vector<long long> result(masterArray->begin(), masterArray->begin() + size);
        return result;
    }
};
