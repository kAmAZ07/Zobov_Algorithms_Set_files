import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

results = pd.read_csv('results.csv')
statistics = pd.read_csv('statistics.csv')

plt.style.use('seaborn-v0_8-darkgrid')
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(15, 6))

for exp in results['experiment'].unique():
    exp_data = results[results['experiment'] == exp]
    ax1.plot(exp_data['step'], exp_data['hll_estimate'], 
             alpha=0.3, color='blue', linewidth=0.8)

ax1.plot(statistics['step'] * 5000, statistics['true_count'], 
         color='red', linewidth=2, label=r'$F_0^t$ (истинное)', marker='o', markersize=4)
ax1.plot(statistics['step'] * 5000, statistics['mean_estimate'], 
         color='blue', linewidth=2, label=r'$N_t$ (HyperLogLog)', marker='s', markersize=4)

ax1.set_xlabel('Размер обработанной части потока')
ax1.set_ylabel('Количество уникальных элементов')
ax1.set_title(r'График №1: Сравнение оценки $N_t$ и $F_0^t$')
ax1.legend()
ax1.grid(True, alpha=0.3)

x = statistics['step'] * 5000
mean = statistics['mean_estimate']
std = statistics['std_estimate']

ax2.plot(x, mean, color='blue', linewidth=2, label=r'$\mathbb{E}(N_t)$', marker='o', markersize=4)
ax2.fill_between(x, mean - std, mean + std, alpha=0.3, color='blue', 
                  label=r'$\mathbb{E}(N_t) \pm \sigma_{N_t}$')
ax2.plot(x, statistics['true_count'], color='red', linewidth=2, 
         label=r'$F_0^t$ (истинное)', linestyle='--', alpha=0.7)

ax2.set_xlabel('Размер обработанной части потока')
ax2.set_ylabel('Количество уникальных элементов')
ax2.set_title('График №2: Статистики оценки')
ax2.legend()
ax2.grid(True, alpha=0.3)

plt.tight_layout()
plt.savefig('hyperloglog_analysis.png', dpi=300, bbox_inches='tight')
print("График сохранен как hyperloglog_analysis.png")

print("\n=== Анализ точности ===")
print(f"Средняя относительная погрешность: {statistics['relative_error'].mean():.2f}%")
print(f"Максимальная относительная погрешность: {statistics['relative_error'].max():.2f}%")

B = 10
theoretical_error_1 = (1.04 / np.sqrt(2**B)) * 100
theoretical_error_2 = (1.3 / np.sqrt(2**B)) * 100
print(f"\nТеоретическая погрешность (1.04/√(2^B)): {theoretical_error_1:.2f}%")
print(f"Теоретическая погрешность (1.3/√(2^B)): {theoretical_error_2:.2f}%")
print(f"Практическая погрешность укладывается в теоретические рамки: {statistics['relative_error'].max() < theoretical_error_2}")
