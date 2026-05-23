import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
from matplotlib import rcParams

rcParams['font.family'] = 'DejaVu Sans'

df = pd.read_csv('results.csv')

sns.set_theme(style="whitegrid")

type_map = {
    'random': 'Случайные данные',
    'reverse': 'Обратно отсортированные данные',
    'almost': 'Почти отсортированные данные'
}

for t in df['Type'].unique():
    subset = df[df['Type'] == t]
    ru_type = type_map.get(t, t)
    
    plt.figure(figsize=(12, 8))
    sns.lineplot(data=subset, x='Size', y='Time_ms', hue='Algorithm', marker='o')
    plt.title(f'Время выполнения - {ru_type}')
    plt.ylabel('Среднее время (мс)')
    plt.xlabel('Размер массива (n)')
    plt.legend(bbox_to_anchor=(1.05, 1), loc='upper left')
    plt.tight_layout()
    plt.savefig(f'time_{t}.png')
    plt.close()
    
    plt.figure(figsize=(12, 8))
    sns.lineplot(data=subset, x='Size', y='Comparisons', hue='Algorithm', marker='o')
    plt.title(f'Посимвольные сравнения - {ru_type}')
    plt.ylabel('Количество сравнений')
    plt.xlabel('Размер массива (n)')
    plt.legend(bbox_to_anchor=(1.05, 1), loc='upper left')
    plt.tight_layout()
    plt.savefig(f'comps_{t}.png')
    plt.close()

print("Charts was saved")
