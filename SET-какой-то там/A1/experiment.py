import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import os
import math
from tqdm import tqdm

S_EXACT = 0.25 * math.pi + 1.25 * math.asin(0.8) - 1

CIRCLES = [
    {'x': 1.0, 'y': 1.0, 'r': 1.0, 'r_sq': 1.0},
    {'x': 1.5, 'y': 2.0, 'r': math.sqrt(5)/2, 'r_sq': 1.25},
    {'x': 2.0, 'y': 1.5, 'r': math.sqrt(5)/2, 'r_sq': 1.25}
]

BOUNDING_BOXES = {
    "wide": {
        "X_min": 0.0,
        "X_max": 1.5 + math.sqrt(5)/2,
        "Y_min": 0.0,
        "Y_max": 2.0 + math.sqrt(5)/2,
    },
    "narrow": {
        "X_min": 2.0 - math.sqrt(5)/2,
        "X_max": 2.0,
        "Y_min": 2.0 - math.sqrt(5)/2,
        "Y_max": 2.0,
    }
}

BOUNDING_BOXES["wide"]["S_rec"] = (BOUNDING_BOXES["wide"]["X_max"] - BOUNDING_BOXES["wide"]["X_min"]) * \
                                  (BOUNDING_BOXES["wide"]["Y_max"] - BOUNDING_BOXES["wide"]["Y_min"])
BOUNDING_BOXES["narrow"]["S_rec"] = (BOUNDING_BOXES["narrow"]["X_max"] - BOUNDING_BOXES["narrow"]["X_min"]) * \
                                    (BOUNDING_BOXES["narrow"]["Y_max"] - BOUNDING_BOXES["narrow"]["Y_min"])

def is_inside(px, py, c):
    dist_sq = (px - c['x'])**2 + (py - c['y'])**2
    return dist_sq <= c['r_sq']

def is_in_intersection(px, py, circles):
    return is_inside(px, py, circles[0]) and \
           is_inside(px, py, circles[1]) and \
           is_inside(px, py, circles[2])

def run_monte_carlo(N, box_params, circles, seed=42):
    X_min, X_max = box_params["X_min"], box_params["X_max"]
    Y_min, Y_max = box_params["Y_min"], box_params["Y_max"]
    S_rec = box_params["S_rec"]

    np.random.seed(seed)

    points_x = np.random.uniform(X_min, X_max, N)
    points_y = np.random.uniform(Y_min, Y_max, N)
    M = 0
    for px, py in zip(points_x, points_y):
        if is_in_intersection(px, py, circles):
            M += 1


    S_estimate = (M / N) * S_rec
    return S_estimate, M

def conduct_experiment(max_N=100000, step=500, num_runs=1):
    N_values = np.arange(100, max_N + step, step)
    results = []
    for N in tqdm(N_values, desc="Running algorithm"):
        for box_name, box_params in BOUNDING_BOXES.items():
            S_estimate, M = run_monte_carlo(N, box_params, CIRCLES)
            relative_error = abs(S_estimate - S_EXACT) / S_EXACT
            results.append({
                "N": N,
                "Box_Type": box_name,
                "S_rec": box_params["S_rec"],
                "M": M,
                "S_estimate": S_estimate,
                "S_exact": S_EXACT,
                "Relative_Error": relative_error
            })
            
    return pd.DataFrame(results)

def plot_results(df):
    plt.figure(figsize=(12, 6))
    df_wide = df[df['Box_Type'] == 'wide']
    plt.plot(df_wide['N'], df_wide['S_estimate'], label='Широкая область ($R_{wide}$)', color='red', alpha=0.7)
    df_narrow = df[df['Box_Type'] == 'narrow']
    plt.plot(df_narrow['N'], df_narrow['S_estimate'], label='Узкая область ($R_{narrow}$)', color='blue', alpha=0.7)
    plt.axhline(y=S_EXACT, color='green', linestyle='--', label=f'Точное значение ($S_{{exact}} \\approx {S_EXACT:.6f}$)')
    
    plt.title('График 1: Зависимость оценки площади от числа точек N', fontsize=14)
    plt.xlabel('Число сгенерированных точек N', fontsize=12)
    plt.ylabel('Оценка площади $\\hat{S}$', fontsize=12)
    plt.legend()
    plt.grid(True, linestyle=':', alpha=0.6)
    plt.tight_layout()
    plt.savefig('A1_Graph1.png')
    plt.close()
    plt.figure(figsize=(12, 6))

    plt.plot(df_wide['N'], df_wide['Relative_Error'] * 100, label='Широкая область ($R_{wide}$)', color='red', alpha=0.7)

    plt.plot(df_narrow['N'], df_narrow['Relative_Error'] * 100, label='Узкая область ($R_{narrow}$)', color='blue', alpha=0.7)
    
    plt.title('График 2: Зависимость относительного отклонения от числа точек N', fontsize=14)
    plt.xlabel('Число сгенерированных точек N', fontsize=12)
    plt.ylabel('Относительное отклонение, %', fontsize=12)
    plt.legend()
    plt.grid(True, which="both", linestyle=':', alpha=0.6)
    plt.tight_layout()
    plt.savefig('A1_Graph2.png')
    plt.close()

if __name__ == "__main__":
    df_results = conduct_experiment()
    data_file_path = 'A1_raw_data.csv'
    df_results.to_csv(data_file_path, index=False)
    print(f"данные сохранены в {data_file_path}")
    plot_results(df_results)
    print("графики построены, картинки сохранены")