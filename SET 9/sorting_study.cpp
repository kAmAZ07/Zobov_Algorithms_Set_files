#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <chrono>
#include <fstream>
#include <iomanip>

unsigned long long char_comparisons = 0;

bool compare_strings(const std::string& a, const std::string& b) {
    size_t len = std::min(a.length(), b.length());
    for (size_t i = 0; i < len; ++i) {
        char_comparisons++;
        if (a[i] != b[i]) return a[i] < b[i];
    }
    char_comparisons++;
    return a.length() < b.length();
}

class StringGenerator {
public:
    static std::vector<std::string> generate_random(int n, int min_len = 10, int max_len = 200) {
        const std::string charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#%:;^&*()-.";
        std::vector<std::string> result;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> len_dist(min_len, max_len);
        std::uniform_int_distribution<> char_dist(0, charset.length() - 1);

        for (int i = 0; i < n; ++i) {
            int len = len_dist(gen);
            std::string s;
            for (int j = 0; j < len; ++j) {
                s += charset[char_dist(gen)];
            }
            result.push_back(s);
        }
        return result;
    }

    static std::vector<std::string> generate_reverse(int n) {
        auto vec = generate_random(n);
        unsigned long long temp = char_comparisons;
        std::sort(vec.begin(), vec.end());
        std::reverse(vec.begin(), vec.end());
        char_comparisons = temp;
        return vec;
    }

    static std::vector<std::string> generate_almost_sorted(int n, int swaps = 10) {
        auto vec = generate_random(n);
        unsigned long long temp = char_comparisons;
        std::sort(vec.begin(), vec.end());
        char_comparisons = temp;
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(0, n - 1);
        for (int i = 0; i < swaps; ++i) {
            std::swap(vec[dist(gen)], vec[dist(gen)]);
        }
        return vec;
    }
};

// Standard QuickSort
void standard_quicksort(std::vector<std::string>& a, int lo, int hi) {
    if (hi <= lo) return;
    int i = lo, j = hi + 1;
    const std::string& v = a[lo];
    while (true) {
        while (compare_strings(a[++i], v)) if (i == hi) break;
        while (compare_strings(v, a[--j])) if (j == lo) break;
        if (i >= j) break;
        std::swap(a[i], a[j]);
    }
    std::swap(a[lo], a[j]);
    standard_quicksort(a, lo, j - 1);
    standard_quicksort(a, j + 1, hi);
}

// Standard MergeSort
void merge(std::vector<std::string>& a, std::vector<std::string>& aux, int lo, int mid, int hi) {
    for (int k = lo; k <= hi; k++) aux[k] = a[k];
    int i = lo, j = mid + 1;
    for (int k = lo; k <= hi; k++) {
        if (i > mid) a[k] = aux[j++];
        else if (j > hi) a[k] = aux[i++];
        else if (compare_strings(aux[j], aux[i])) a[k] = aux[j++];
        else a[k] = aux[i++];
    }
}
void standard_mergesort(std::vector<std::string>& a, std::vector<std::string>& aux, int lo, int hi) {
    if (hi <= lo) return;
    int mid = lo + (hi - lo) / 2;
    standard_mergesort(a, aux, lo, mid);
    standard_mergesort(a, aux, mid + 1, hi);
    merge(a, aux, lo, mid, hi);
}

// Ternary String QuickSort
int char_at(const std::string& s, int d) {
    if (d < (int)s.length()) return (unsigned char)s[d];
    return -1;
}

void ternary_string_quicksort(std::vector<std::string>& a, int lo, int hi, int d) {
    if (hi <= lo) return;
    int lt = lo, gt = hi;
    int v = char_at(a[lo], d);
    int i = lo + 1;
    while (i <= gt) {
        int t = char_at(a[i], d);
        char_comparisons++;
        if (t < v) std::swap(a[lt++], a[i++]);
        else if (t > v) std::swap(a[i], a[gt--]);
        else i++;
    }
    ternary_string_quicksort(a, lo, lt - 1, d);
    if (v >= 0) ternary_string_quicksort(a, lt, gt, d + 1);
    ternary_string_quicksort(a, gt + 1, hi, d);
}

int get_lcp(const std::string& a, const std::string& b) {
    int len = std::min(a.length(), b.length());
    int lcp = 0;
    while (lcp < len && a[lcp] == b[lcp]) {
        char_comparisons++;
        lcp++;
    }
    if (lcp < len) char_comparisons++;
    return lcp;
}

void string_mergesort(std::vector<std::string>& a, std::vector<std::string>& aux, int lo, int hi) {
    if (hi <= lo) return;
    int mid = lo + (hi - lo) / 2;
    string_mergesort(a, aux, lo, mid);
    string_mergesort(a, aux, mid + 1, hi);
    for (int k = lo; k <= hi; k++) aux[k] = a[k];
    int i = lo, j = mid + 1;
    for (int k = lo; k <= hi; k++) {
        if (i > mid) a[k] = aux[j++];
        else if (j > hi) a[k] = aux[i++];
        else if (compare_strings(aux[j], aux[i])) a[k] = aux[j++];
        else a[k] = aux[i++];
    }
}
// MSD Radix Sort
const int R = 256;
std::vector<std::string> msd_aux;

void msd_radix_sort(std::vector<std::string>& a, int lo, int hi, int d) {
    if (hi <= lo) return;
    std::vector<int> count(R + 2, 0);
    for (int i = lo; i <= hi; i++) {
        count[char_at(a[i], d) + 2]++;
        char_comparisons++;
    }
    for (int r = 0; r < R + 1; r++)
        count[r + 1] += count[r];
    for (int i = lo; i <= hi; i++)
        msd_aux[count[char_at(a[i], d) + 1]++] = a[i];
    for (int i = lo; i <= hi; i++)
        a[i] = msd_aux[i - lo];
    for (int r = 0; r < R; r++)
        msd_radix_sort(a, lo + count[r], lo + count[r + 1] - 1, d + 1);
}

// MSD Radix Sort with QuickSort
void msd_radix_plus_quicksort(std::vector<std::string>& a, int lo, int hi, int d) {
    if (hi <= lo) return;
    if (hi - lo < R) {
        ternary_string_quicksort(a, lo, hi, d);
        return;
    }
    std::vector<int> count(R + 2, 0);
    for (int i = lo; i <= hi; i++) {
        count[char_at(a[i], d) + 2]++;
        char_comparisons++;
    }
    for (int r = 0; r < R + 1; r++)
        count[r + 1] += count[r];
    for (int i = lo; i <= hi; i++)
        msd_aux[count[char_at(a[i], d) + 1]++] = a[i];
    for (int i = lo; i <= hi; i++)
        a[i] = msd_aux[i - lo];
    for (int r = 0; r < R; r++)
        msd_radix_plus_quicksort(a, lo + count[r], lo + count[r + 1] - 1, d + 1);
}

class StringSortTester {
public:
    struct Result {
        double time_ms;
        unsigned long long comparisons;
    };

    static Result test(std::vector<std::string> data, std::string algo) {
        char_comparisons = 0;
        std::vector<std::string> aux(data.size());
        msd_aux.resize(data.size());

        auto start = std::chrono::high_resolution_clock::now();
        
        if (algo == "QUICKSORT") standard_quicksort(data, 0, data.size() - 1);
        else if (algo == "MERGESORT") standard_mergesort(data, aux, 0, data.size() - 1);
        else if (algo == "STRING_QUICKSORT") ternary_string_quicksort(data, 0, data.size() - 1, 0);
        else if (algo == "STRING_MERGESORT") string_mergesort(data, aux, 0, data.size() - 1);
        else if (algo == "MSD_RADIX") msd_radix_sort(data, 0, data.size() - 1, 0);
        else if (algo == "MSD_RADIX_QUICK") msd_radix_plus_quicksort(data, 0, data.size() - 1, 0);

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> diff = end - start;

        return {diff.count(), char_comparisons};
    }
};

int main() {
    std::vector<std::string> algos = {"QUICKSORT", "MERGESORT", "STRING_QUICKSORT", "STRING_MERGESORT", "MSD_RADIX", "MSD_RADIX_QUICK"};
    std::vector<std::string> types = {"random", "reverse", "almost"};
    
    std::ofstream out("results.csv");
    out << "Type,Size,Algorithm,Time_ms,Comparisons\n";

    for (auto& type : types) {
        std::cout << "Testing type: " << type << std::endl;
        std::vector<std::string> full_data;
        if (type == "random") full_data = StringGenerator::generate_random(3000);
        else if (type == "reverse") full_data = StringGenerator::generate_reverse(3000);
        else full_data = StringGenerator::generate_almost_sorted(3000);

        for (int size = 100; size <= 3000; size += 100) {
            std::vector<std::string> sub_data(full_data.begin(), full_data.begin() + size);
            for (auto& algo : algos) {
                double total_time = 0;
                unsigned long long total_comps = 0;
                int runs = 5;
                for(int r=0; r<runs; ++r) {
                    auto res = StringSortTester::test(sub_data, algo);
                    total_time += res.time_ms;
                    total_comps = res.comparisons;
                }
                out << type << "," << size << "," << algo << "," << total_time/runs << "," << total_comps << "\n";
            }
        }
    }
    out.close();
    return 0;
}
