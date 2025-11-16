import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import os

plt.rcParams['font.family'] = 'Noto Sans CJK SC'
plt.rcParams['font.size'] = 12

DATA_FILE = "experiment_results.csv"
PLOTS_DIR = "plots"

if not os.path.exists(PLOTS_DIR):
    os.makedirs(PLOTS_DIR)

def load_data():
    try:
        df = pd.read_csv(DATA_FILE)
        df['Time_ms'] = df['Time_us'] / 1000
        return df
    except FileNotFoundError:
        print(f"Файл {DATA_FILE} не найден")
        return None

def find_optimal_k(df):
    optimal_k_df = df[df['Algorithm'] == 'HybridMergeInsertionSort'].copy()
    optimal_k_df = optimal_k_df.loc[optimal_k_df.groupby(['Size', 'ArrayType'])['Time_ms'].idxmin()]
    most_frequent_k = optimal_k_df['K'].mode()[0]
    print(f"Оптимальное K: {most_frequent_k}")
    return most_frequent_k

def plot_standard_merge_sort(df):
    print("Графики для Merge Sort")
    valid_array_types = ['Random', 'Reversed', 'NearlySorted']
    for array_type in valid_array_types:
        subset = df[(df['Algorithm'] == 'StandardMergeSort') & (df['ArrayType'] == array_type)]
        plt.figure(figsize=(10, 6))
        plt.plot(subset['Size'], subset['Time_ms'], label='Standard Merge Sort', color='blue')
        plt.title(f'Standard Merge Sort: Зависимость времени от размера массива ({array_type})')
        plt.xlabel('Размер массива (N)')
        plt.ylabel('Время выполнения (мс)')
        plt.grid(True, linestyle='--', alpha=0.7)
        plt.legend()
        plt.tight_layout()
        plt.savefig(os.path.join(PLOTS_DIR, f'Standard_Merge_Sort_{array_type}.png'))
        plt.close()

def plot_hybrid_k_comparison(df):
    print("Графики сравнения Hybrid Merge Sort (разные K)")
    valid_array_types = ['Random', 'Reversed', 'NearlySorted']
    for array_type in valid_array_types:
        plt.figure(figsize=(12, 7))
        hybrid_df = df[df['Algorithm'] == 'HybridMergeInsertionSort']
        for k in sorted(hybrid_df['K'].unique()):
            subset = hybrid_df[(hybrid_df['ArrayType'] == array_type) & (hybrid_df['K'] == k)]
            plt.plot(subset['Size'], subset['Time_ms'], label=f'K={k}', alpha=0.8)
        plt.title(f'Hybrid Merge+Insertion Sort: Сравнение K ({array_type})')
        plt.xlabel('Размер массива (N)')
        plt.ylabel('Время выполнения (мс)')
        plt.grid(True, linestyle='--', alpha=0.7)
        plt.legend(title='Порог K')
        plt.tight_layout()
        plt.savefig(os.path.join(PLOTS_DIR, f'Hybrid_K_Comparison_{array_type}.png'))
        plt.close()

def plot_final_comparison(df, optimal_k):
    print("Графики финального сравнения")
    
    valid_array_types = ['Random', 'Reversed', 'NearlySorted']
    for array_type in valid_array_types:
        plt.figure(figsize=(10, 6))
        std_subset = df[(df['Algorithm'] == 'StandardMergeSort') & (df['ArrayType'] == array_type)]
        plt.plot(std_subset['Size'], std_subset['Time_ms'], label='Standard Merge Sort', color='blue', linewidth=2)
        hybrid_subset = df[(df['Algorithm'] == 'HybridMergeInsertionSort') & 
                           (df['ArrayType'] == array_type) & 
                           (df['K'] == optimal_k)]
        plt.plot(hybrid_subset['Size'], hybrid_subset['Time_ms'], label=f'Hybrid Merge+Insertion Sort (K={optimal_k})', color='red', linestyle='--', linewidth=2)
        
        plt.title(f'Сравнительный анализ: Standard vs Hybrid (K={optimal_k}) ({array_type})')
        plt.xlabel('Размер массива (N)')
        plt.ylabel('Время выполнения (мс)')
        plt.grid(True, linestyle='--', alpha=0.7)
        plt.legend()
        plt.tight_layout()
        plt.savefig(os.path.join(PLOTS_DIR, f'Final_Comparison_{array_type}.png'))
        plt.close()

def analyze_and_plot():
    df = load_data()
    if df is None:
        return
    plot_standard_merge_sort(df)
    plot_hybrid_k_comparison(df)
    optimal_k = find_optimal_k(df)
    plot_final_comparison(df, optimal_k)

    print(f"Успешно")

if __name__ == "__main__":
    analyze_and_plot()
