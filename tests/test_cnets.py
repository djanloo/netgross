import unittest
import numpy as np

import cnets

# import matplotlib.pyplot as plt


class cnetsTest(unittest.TestCase):
    """This test case should not use network.py classes
    
    Note:
        the graph is a square with a 0.8 long edge
        this tests also the floating point target bug.
    """

    def setUp(self):
        self.squareSM = [[0, 1, 0.8], [1, 2, 0.8], [2, 3, 0.8], [3, 0, 0.8]]
        diag = np.sqrt(2)
        self.correct_dist_M = np.array(
            [
                [0.0, 1.0, diag, 1.0],
                [1.0, 0.0, 1.0, diag],
                [diag, 1.0, 0.0, 1.0],
                [1.0, diag, 1.0, 0.0],
            ]
        )
        self.correct_dist_M *= 0.8
        self.values = [0.0, 0.0, 0.0, 0.0]
        cnets.init_network(self.squareSM, self.values, 2)

    def test_MDE(self):
        cnets.MDE(0.1, 0.01, 100)
        distM = np.array(cnets.get_distanceM())
        # print(distM)
        distortion = np.mean((distM - self.correct_dist_M) ** 2)
        # print(distortion)
        self.assertLessEqual(distortion, 0.5)
    
if __name__ == "__main__":
    unittest.main()
