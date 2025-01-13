import numpy as np

def longest_sequence_length(arr:np.ndarray) -> int:
    # Проверка на пустой массив
    if len(arr) == 0:
        return 0

    delta_value = np.where(np.diff(arr) != 0)[0] + 1 # Находим индексы элементов, с которых начинается следующая подпоследовательность
    # "np.Where" - Находит индексы элементов, подходящих под условие / Прибавляем единицу, чтобы было удобнее работать с индексами
    # "np.diff" - Находит разницу между соседними элементами

    # Добавляем индекс начача и индекс конца массива
    delta_value = np.insert(delta_value, 0,0)
    delta_value = np.append(delta_value, len(arr))

    # Находим максимальную разницу между индексами, что и будет является максимальной длиной
    return np.max(np.diff(delta_value))
