import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# Чтение данных
try:
    stats = pd.read_csv('comparison_statistics.csv')
    print("Данные загружены успешно")
except FileNotFoundError:
    print("Нет файла")
    exit(1)

plt.style.use('seaborn-v0_8-darkgrid')
fig = plt.figure(figsize=(18, 12))

x = stats['step'] * 5000
B = 10
theoretical_1 = (1.04 / np.sqrt(2**B)) * 100
theoretical_2 = (1.3 / np.sqrt(2**B)) * 100

ax1 = plt.subplot(2, 3, 1)
ax1.plot(x, stats['true_count'], 'r-', linewidth=2.5, label=r'$F_0^t$ (истинное)', marker='o', markersize=4, zorder=5)
ax1.plot(x, stats['mean_std'], 'b--', linewidth=2, label='Стандартный HLL', marker='s', markersize=3, alpha=0.8)
ax1.plot(x, stats['mean_imp'], 'g--', linewidth=2, label='Улучшенный HLL', marker='^', markersize=3, alpha=0.8)
ax1.plot(x, stats['mean_cmp'], 'm--', linewidth=2, label='Компактный HLL', marker='d', markersize=3, alpha=0.8)

ax1.set_xlabel('Размер обработанной части потока', fontsize=10)
ax1.set_ylabel('Количество уникальных элементов', fontsize=10)
ax1.set_title(r'График №1: Сравнение оценок с истинным значением', fontsize=11, fontweight='bold')
ax1.legend(fontsize=9, loc='upper left')
ax1.grid(True, alpha=0.3)

ax2 = plt.subplot(2, 3, 2)
ax2.plot(x, stats['error_std'], 'b-', linewidth=2, label='Стандартный', marker='s', markersize=3)
ax2.plot(x, stats['error_imp'], 'g-', linewidth=2, label='Улучшенный', marker='^', markersize=3)
ax2.plot(x, stats['error_cmp'], 'm-', linewidth=2, label='Компактный', marker='d', markersize=3)

ax2.axhline(y=theoretical_1, color='orange', linestyle=':', linewidth=1.5, 
            label=f'Теория (1.04/√m) = {theoretical_1:.2f}%', alpha=0.7)
ax2.axhline(y=theoretical_2, color='red', linestyle=':', linewidth=1.5, 
            label=f'Теория (1.3/√m) = {theoretical_2:.2f}%', alpha=0.7)

ax2.set_xlabel('Размер обработанной части потока', fontsize=10)
ax2.set_ylabel('Относительная погрешность (%)', fontsize=10)
ax2.set_title('Относительная погрешность vs Теория', fontsize=11, fontweight='bold')
ax2.legend(fontsize=8, loc='upper left')
ax2.grid(True, alpha=0.3)
ax2.set_ylim(0, max(theoretical_2 * 1.2, stats['error_std'].max() * 1.1))

ax3 = plt.subplot(2, 3, 3)
ax3.plot(x, stats['std_std'], 'b-', linewidth=2, label='Стандартный', marker='s', markersize=3)
ax3.plot(x, stats['std_imp'], 'g-', linewidth=2, label='Улучшенный', marker='^', markersize=3)
ax3.plot(x, stats['std_cmp'], 'm-', linewidth=2, label='Компактный', marker='d', markersize=3)

ax3.set_xlabel('Размер обработанной части потока', fontsize=10)
ax3.set_ylabel(r'Стандартное отклонение ($\sigma$)', fontsize=10)
ax3.set_title('Стабильность оценок', fontsize=11, fontweight='bold')
ax3.legend(fontsize=9)
ax3.grid(True, alpha=0.3)

ax4 = plt.subplot(2, 3, 4)
ax4.plot(x, stats['mean_std'], 'b-', linewidth=2.5, label=r'$\mathbb{E}(N_t)$ Стандартный', zorder=3)
ax4.fill_between(x, stats['mean_std'] - stats['std_std'], 
                  stats['mean_std'] + stats['std_std'], 
                  alpha=0.3, color='blue', label=r'$\mathbb{E}(N_t) \pm \sigma_{N_t}$', zorder=2)
ax4.plot(x, stats['true_count'], 'r--', linewidth=2, label=r'$F_0^t$ (истинное)', alpha=0.8, zorder=4)

ax4.set_xlabel('Размер обработанной части потока', fontsize=10)
ax4.set_ylabel('Количество уникальных элементов', fontsize=10)
ax4.set_title('График №2a: Стандартный HLL', fontsize=11, fontweight='bold')
ax4.legend(fontsize=9)
ax4.grid(True, alpha=0.3)

ax5 = plt.subplot(2, 3, 5)
ax5.plot(x, stats['mean_imp'], 'g-', linewidth=2.5, label=r'$\mathbb{E}(N_t)$ Улучшенный', zorder=3)
ax5.fill_between(x, stats['mean_imp'] - stats['std_imp'], 
                  stats['mean_imp'] + stats['std_imp'], 
                  alpha=0.3, color='green', label=r'$\mathbb{E}(N_t) \pm \sigma_{N_t}$', zorder=2)
ax5.plot(x, stats['true_count'], 'r--', linewidth=2, label=r'$F_0^t$ (истинное)', alpha=0.8, zorder=4)

ax5.set_xlabel('Размер обработанной части потока', fontsize=10)
ax5.set_ylabel('Количество уникальных элементов', fontsize=10)
ax5.set_title('График №2b: Улучшенный HLL', fontsize=11, fontweight='bold')
ax5.legend(fontsize=9)
ax5.grid(True, alpha=0.3)

ax6 = plt.subplot(2, 3, 6)
ax6.plot(x, stats['mean_cmp'], 'm-', linewidth=2.5, label=r'$\mathbb{E}(N_t)$ Компактный', zorder=3)
ax6.fill_between(x, stats['mean_cmp'] - stats['std_cmp'], 
                  stats['mean_cmp'] + stats['std_cmp'], 
                  alpha=0.3, color='magenta', label=r'$\mathbb{E}(N_t) \pm \sigma_{N_t}$', zorder=2)
ax6.plot(x, stats['true_count'], 'r--', linewidth=2, label=r'$F_0^t$ (истинное)', alpha=0.8, zorder=4)

ax6.set_xlabel('Размер обработанной части потока', fontsize=10)
ax6.set_ylabel('Количество уникальных элементов', fontsize=10)
ax6.set_title('График №2c: Компактный HLL', fontsize=11, fontweight='bold')
ax6.legend(fontsize=9)
ax6.grid(True, alpha=0.3)

plt.tight_layout()
plt.savefig('hyperloglog_comparison.png', dpi=300, bbox_inches='tight')
print("✓ График сохранен: hyperloglog_comparison.png")

print("\n" + "="*70)
print("  СВОДНАЯ ТАБЛИЦА РЕЗУЛЬТАТОВ")
print("="*70)
print(f"{'Версия':<20} {'Средняя ошибка':<18} {'Макс. ошибка':<15} {'Средняя σ':<12}")
print("-" * 70)

versions = [
    ('Стандартный', 'error_std', 'std_std'),
    ('Улучшенный', 'error_imp', 'std_imp'),
    ('Компактный', 'error_cmp', 'std_cmp')
]

for name, error_col, std_col in versions:
    mean_error = stats[error_col].mean()
    max_error = stats[error_col].max()
    mean_std = stats[std_col].mean()
    rel_std = (mean_std / stats['true_count'].mean()) * 100
    print(f"{name:<20} {mean_error:>6.2f}%{'':<10} {max_error:>6.2f}%{'':<6} {rel_std:>6.2f}%")

print("\nТеоретические пределы:")
print(f"  1.04/√(2^B) = {theoretical_1:.2f}%")
print(f"  1.3/√(2^B) = {theoretical_2:.2f}%")

print("\n" + "="*70)
print("  АНАЛИЗ УЛУЧШЕНИЙ")
print("="*70)

improvement_accuracy = ((stats['error_std'].mean() - stats['error_imp'].mean()) / 
                        stats['error_std'].mean() * 100)
improvement_stability = ((stats['std_std'].mean() - stats['std_imp'].mean()) / 
                        stats['std_std'].mean() * 100)

print(f"\nУлучшенный vs Стандартный:")
print(f"  Точность:      {improvement_accuracy:+.1f}%")
print(f"  Стабильность:  {improvement_stability:+.1f}%")

print(f"\nКомпактный vs Стандартный:")
compact_accuracy = ((stats['error_std'].mean() - stats['error_cmp'].mean()) / 
                   stats['error_std'].mean() * 100)
compact_stability = ((stats['std_std'].mean() - stats['std_cmp'].mean()) / 
                    stats['std_std'].mean() * 100)
print(f"  Точность:      {compact_accuracy:+.1f}%")
print(f"  Стабильность:  {compact_stability:+.1f}%")
print(f"  Экономия памяти: 25%")

print("\n" + "="*70)
