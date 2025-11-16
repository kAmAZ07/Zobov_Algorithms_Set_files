import subprocess
import time
import random
import math
import matplotlib.pyplot as plt
import numpy as np
import os

SIZES = [10000, 25000, 50000, 75000, 100000]
REPEATS = 5
EXECUTABLE_PATH = "./A3_sol/A3/sort_experiment"
RESULTS_FILE = "A3_sol/A3/experiment_results_new.csv"

def generate_random_data(n):
    return [random.randint(-10**9, 10**9) for _ in range(n)]

def generate_sorted_data(n):
    arr = [random.randint(-10**9, 10**9) for _ in range(n)]
    arr.sort()
    return arr

def generate_reverse_sorted_data(n):
    arr = [random.randint(-10**9, 10**9) for _ in range(n)]
    arr.sort(reverse=True)
    return arr

def generate_nearly_sorted_data(n):
    arr = generate_sorted_data(n)
    
    swaps = n // 100
    if swaps == 0 and n > 1:
        swaps = 1
        
    for _ in range(swaps):
        idx1 = random.randint(0, n - 1)
        idx2 = random.randint(0, n - 1)
        arr[idx1], arr[idx2] = arr[idx2], arr[idx1]
    return arr

def get_data_generator(data_type):
    if data_type == "random":
        return generate_random_data
    elif data_type == "reverse_sorted":
        return generate_reverse_sorted_data
    elif data_type == "nearly_sorted":
        return generate_nearly_sorted_data
    else:
        raise ValueError(f"Неизвестный тип данных: {data_type}")

def run_sort_experiment(data, sort_type):
    n = len(data)
    input_data = f"{n}\n" + " ".join(map(str, data)) + "\n"
    command = [EXECUTABLE_PATH, sort_type]
    
    start_time = time.time()
    try:
        result = subprocess.run(
            command,
            input=input_data,
            capture_output=True,
            text=True,
            timeout=600
        )
        end_time = time.time()
        
        if result.returncode != 0:
            print(f"Ошибка выполнения C++ программы ({sort_type}, N={n}):")
            print(result.stderr)
            return -1.0
            
        return end_time - start_time
        
    except subprocess.TimeoutExpired:
        print(f"Таймаут выполнения C++ программы ({sort_type}, N={n})")
        return -3.0
    except Exception as e:
        print(f"Непредвиденная ошибка: {e}")
        return -4.0


def run_experiment():
    if not os.path.exists(EXECUTABLE_PATH):
        print(f"Ошибка: Исполняемый файл {EXECUTABLE_PATH} не найден. Сначала скомпилируйте main.cpp.")
        return
    os.makedirs(os.path.dirname(RESULTS_FILE), exist_ok=True)

    with open(RESULTS_FILE, "w") as f:
        f.write("Size,DataType,SortType,Time\n")
        
    all_results = []
    
    for size in SIZES:
        for data_type in ["random", "reverse_sorted", "nearly_sorted"]:
            generator = get_data_generator(data_type)
            
            data = generator(size)
            
            print(f"Тестирование: N={size}, Тип данных: {data_type}")
            
            for sort_type in ["intro", "quick"]:
                times = []
                for i in range(REPEATS):
                    if data_type == "random" or data_type == "nearly_sorted":
                        current_data = generator(size)
                    else:
                        current_data = data
                        
                    time_taken = run_sort_experiment(current_data, sort_type)
                    
                    if time_taken > 0:
                        times.append(time_taken)
                        print(f"  {sort_type.upper()} - Повторение {i+1}: {time_taken:.4f} сек")
                    else:
                        print(f"  {sort_type.upper()} - Ошибка. Пропуск оставшихся повторений.")
                        break
                
                if times:
                    avg_time = sum(times) / len(times)
                    print(f"  {sort_type.upper()} - Среднее время: {avg_time:.4f} сек\n")
                    with open(RESULTS_FILE, "a") as f:
                        f.write(f"{size},{data_type},{sort_type},{avg_time}\n")
                        
                    all_results.append({
                        "Size": size,
                        "DataType": data_type,
                        "SortType": sort_type,
                        "Time": avg_time
                    })
                else:
                    print(f"  {sort_type.upper()} - Не удалось получить результаты.\n")
                    
    return all_results

def plot_results(results):
    if not results:
        print("Нет данных")
        return
    data_types = ["random", "reverse_sorted", "nearly_sorted"]
    
    for data_type in data_types:
        plt.figure(figsize=(10, 6))
        
        intro_data = [r for r in results if r["DataType"] == data_type and r["SortType"] == "intro"]
        quick_data = [r for r in results if r["DataType"] == data_type and r["SortType"] == "quick"]
        x_intro = [r["Size"] for r in intro_data]
        y_intro = [r["Time"] for r in intro_data]
        
        x_quick = [r["Size"] for r in quick_data]
        y_quick = [r["Time"] for r in quick_data]
        
        plt.plot(x_intro, y_intro, marker='o', label='Introsort')
        plt.plot(x_quick, y_quick, marker='x', label='Quick Sort (Standard)')
        
        title_map = {
            "random": "Случайные данные",
            "reverse_sorted": "Обратно отсортированные данные",
            "nearly_sorted": "Почти отсортированные данные"
        }
        
        plt.title(f"Сравнение Introsort и Quick Sort на {title_map[data_type]}", fontsize=14)
        plt.xlabel("Размер массива (N)", fontsize=12)
        plt.ylabel("Среднее время выполнения (секунды)", fontsize=12)
        plt.legend()
        plt.grid(True, linestyle='--', alpha=0.6)
        
        filename = f"comparison_plot_{data_type}_new.png"
        plt.savefig(filename)
        print(f"График сохранен: {filename}")

if __name__ == "__main__":
    try:
        plt.rcParams['font.family'] = 'sans-serif'
        plt.rcParams['font.sans-serif'] = ['Noto Sans CJK SC', 'DejaVu Sans', 'Arial']
    except Exception:
        print("не получилось установить шрифт")

    results = run_experiment()
    plot_results(results)
