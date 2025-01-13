import unittest
import numpy as np
from lab_9 import longest_sequence_length

class TestLongestSequenceLength(unittest.TestCase):
    def test_empty_array(self): # Проверка на пустой массив
        arr = np.array([])
        self.assertEqual(longest_sequence_length(arr), 0)

    def test_all_elements_same(self): # Проверка массива с одинаковыми числами
        arr = np.array([3, 3, 3, 3, 3])
        self.assertEqual(longest_sequence_length(arr), 5)

    def test_no_repeated_elements(self): # Проверка массива с разными числами
        arr = np.array([1, 2, 3, 4, 5])
        self.assertEqual(longest_sequence_length(arr), 1, )

    def test_varied_sequences(self): # Проверка произвольного массива
        arr = np.array([1, 1, 2, 2, 2, 3, 3, 4])
        self.assertEqual(longest_sequence_length(arr), 3)

if __name__ == "__main__":
    unittest.main()
