import subprocess
import time
import random
import math
import matplotlib.pyplot as plt
import numpy as np
import os

# --- Конфигурация эксперимента ---
# Размеры массивов для тестирования (скорректировано по данным A2, где MAX_SIZE = 100000)
SIZES = [10000, 25000, 50000, 75000, 100000]
# Количество повторений для усреднения
REPEATS = 5
# Путь к исполняемому файлу
EXECUTABLE_PATH = "./A3_sol/A3/sort_experiment"
# Файл для сохранения результатов
RESULTS_FILE = "A3_sol/A3/experiment_results_new.csv"

# --- Функции генерации данных ---

def generate_random_data(n):
    """Генерирует массив из n случайных целых чисел."""
    # Используем диапазон, как в задаче A3i: по модулю не превосходит 10^9
    return [random.randint(-10**9, 10**9) for _ in range(n)]

def generate_sorted_data(n):
    """Генерирует отсортированный массив из больших случайных чисел."""
    # Генерируем случайные числа и сортируем их, чтобы они были в том же диапазоне, что и случайные.
    arr = [random.randint(-10**9, 10**9) for _ in range(n)]
    arr.sort()
    return arr

def generate_reverse_sorted_data(n):
    """Генерирует обратно отсортированный массив из больших случайных чисел."""
    # Генерируем случайные числа и сортируем их в обратном порядке.
    arr = [random.randint(-10**9, 10**9) for _ in range(n)]
    arr.sort(reverse=True)
    return arr

def generate_nearly_sorted_data(n):
    """Генерирует почти отсортированный массив (сортированный с небольшим количеством перестановок)."""
    # Начинаем с отсортированного массива из больших случайных чисел
    arr = generate_sorted_data(n)
    
    # Количество перестановок: N / 100 (как указано в отчете)
    swaps = n // 100
    # Гарантируем хотя бы одну перестановку для N < 100
    if swaps == 0 and n > 1:
        swaps = 1
        
    for _ in range(swaps):
        # Выбираем два случайных индекса и меняем элементы местами
        idx1 = random.randint(0, n - 1)
        idx2 = random.randint(0, n - 1)
        arr[idx1], arr[idx2] = arr[idx2], arr[idx1]
    return arr

def get_data_generator(data_type):
    """Возвращает функцию-генератор по типу данных."""
    if data_type == "random":
        return generate_random_data
    elif data_type == "reverse_sorted":
        return generate_reverse_sorted_data
    elif data_type == "nearly_sorted":
        return generate_nearly_sorted_data
    else:
        raise ValueError(f"Неизвестный тип данных: {data_type}")

# --- Функция для запуска C++ программы и замера времени ---

def run_sort_experiment(data, sort_type):
    """
    Запускает C++ программу с заданным массивом и типом сортировки.
    Возвращает время выполнения в секундах.
    """
    n = len(data)
    
    # Формируем входные данные для C++ программы
    # В первой строке N, во второй - элементы массива
    input_data = f"{n}\n" + " ".join(map(str, data)) + "\n"
    
    # sort_type: "intro" или "quick"
    
    command = [EXECUTABLE_PATH, sort_type]
    
    start_time = time.time()
    try:
        # Запускаем процесс, передавая данные через stdin
        result = subprocess.run(
            command,
            input=input_data, # Передаем строку, так как text=True
            capture_output=True,
            text=True,
            timeout=600 # Увеличиваем таймаут на всякий случай
        )
        end_time = time.time()
        
        if result.returncode != 0:
            print(f"Ошибка выполнения C++ программы ({sort_type}, N={n}):")
            print(result.stderr)
            return -1.0
            
        # Проверка, что массив отсортирован (опционально, но полезно)
        # output_array = list(map(int, result.stdout.strip().split()))
        # if output_array != sorted(data):
        #     print(f"Ошибка сортировки ({sort_type}, N={n})")
        #     return -2.0
            
        return end_time - start_time
        
    except subprocess.TimeoutExpired:
        print(f"Таймаут выполнения C++ программы ({sort_type}, N={n})")
        return -3.0
    except Exception as e:
        print(f"Непредвиденная ошибка: {e}")
        return -4.0

# --- Основная функция эксперимента ---

def run_experiment():
    """Проводит полный эксперимент и сохраняет результаты."""
    
    # Проверяем наличие исполняемого файла
    if not os.path.exists(EXECUTABLE_PATH):
        print(f"Ошибка: Исполняемый файл {EXECUTABLE_PATH} не найден. Сначала скомпилируйте main.cpp.")
        return
    
    # Создаем директорию для результатов, если ее нет
    os.makedirs(os.path.dirname(RESULTS_FILE), exist_ok=True)

    # Заголовки CSV файла
    with open(RESULTS_FILE, "w") as f:
        f.write("Size,DataType,SortType,Time\n")
        
    all_results = []
    
    for size in SIZES:
        for data_type in ["random", "reverse_sorted", "nearly_sorted"]:
            generator = get_data_generator(data_type)
            
            # Генерируем данные один раз для всех повторений
            data = generator(size)
            
            print(f"Тестирование: N={size}, Тип данных: {data_type}")
            
            for sort_type in ["intro", "quick"]:
                times = []
                for i in range(REPEATS):
                    # Создаем копию данных для сортировки
                    # Копирование не нужно, так как данные передаются через stdin
                    # Но нужно генерировать новые данные для каждого повторения,
                    # если генератор случайный (random, nearly_sorted)
                    
                    # Для random и nearly_sorted генерируем новые данные для каждого повторения
                    if data_type == "random" or data_type == "nearly_sorted":
                        current_data = generator(size)
                    else:
                        # Для reverse_sorted данные остаются теми же
                        current_data = data
                        
                    # Запускаем эксперимент
                    time_taken = run_sort_experiment(current_data, sort_type)
                    
                    if time_taken > 0:
                        times.append(time_taken)
                        print(f"  {sort_type.upper()} - Повторение {i+1}: {time_taken:.4f} сек")
                    else:
                        # Если ошибка, прерываем повторения для этого случая
                        print(f"  {sort_type.upper()} - Ошибка. Пропуск оставшихся повторений.")
                        break
                
                if times:
                    avg_time = sum(times) / len(times)
                    print(f"  {sort_type.upper()} - Среднее время: {avg_time:.4f} сек\n")
                    
                    # Сохраняем результат
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

# --- Функция для построения графиков ---

def plot_results(results):
    """Строит графики по собранным результатам."""
    
    if not results:
        print("Нет данных для построения графиков.")
        return
        
    # Группируем данные по типу
    data_types = ["random", "reverse_sorted", "nearly_sorted"]
    
    for data_type in data_types:
        plt.figure(figsize=(10, 6))
        
        # Фильтруем данные для Introsort и Quick Sort
        intro_data = [r for r in results if r["DataType"] == data_type and r["SortType"] == "intro"]
        quick_data = [r for r in results if r["DataType"] == data_type and r["SortType"] == "quick"]
        
        # Извлекаем оси X (размер) и Y (время)
        x_intro = [r["Size"] for r in intro_data]
        y_intro = [r["Time"] for r in intro_data]
        
        x_quick = [r["Size"] for r in quick_data]
        y_quick = [r["Time"] for r in quick_data]
        
        # Построение графиков
        plt.plot(x_intro, y_intro, marker='o', label='Introsort')
        plt.plot(x_quick, y_quick, marker='x', label='Quick Sort (Standard)')
        
        # Настройка графика
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
        
        # Сохранение графика
        filename = f"comparison_plot_{data_type}_new.png"
        plt.savefig(filename)
        print(f"График сохранен: {filename}")

# --- Запуск ---
if __name__ == "__main__":
    # Для корректного отображения кириллицы в Matplotlib
    # Установка шрифта Noto Sans CJK SC (если доступен)
    try:
        plt.rcParams['font.family'] = 'sans-serif'
        plt.rcParams['font.sans-serif'] = ['Noto Sans CJK SC', 'DejaVu Sans', 'Arial']
    except Exception:
        print("Шрифт Noto Sans CJK SC не найден, используем стандартный.")

    # Запускаем эксперимент
    results = run_experiment()
    plot_results(results)
