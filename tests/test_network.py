import numpy as np
import unittest
from netgross.classiter import cdict, cset, clist
from netgross.network import Node, undLink, undNetwork, dirLink, dirNetwork

class testUndLinks(unittest.TestCase):
    def setUp(self):
        self.nodes = cdict({i: Node(i) for i in range(5)})
        self.links = cset()
        self.link_sparse = [
            [1, 2, 0.1],
            [2, 1, 0.1],
            [1, 3, 0.5],
            [3, 4, 1.0],
            [4, 3, 1.5],
        ]
        for i, j, d in self.link_sparse:
            self.links += self.nodes[i].connect(self.nodes[j], d)

    def test_links(self):
        self.assertEqual(
            len(self.links),
            3,
            f"uniLink(i,j) should be equivalent to undLink(j,i): {self.links}, {self.link_sparse}",
        )

    def test_density(self):
        for N in range(3, 100, 20):
            rn = undNetwork.Random(N, 1)
            density = len(rn.links) / (N * (N - 1) / 2)
            self.assertEqual(
                density,
                1.0,
                f"Dense random network has density {density:.2f}"
                + f" instead of 1 (number_of_nodes = {N})\n"
                + f"links = {rn.links}\n"
                + f"links hash = {[hash(link) for link in rn.links]}",
            )

    def test_value_assignment(self):
        N = 100
        rn = undNetwork.Random(N, 1)
        values = np.random.uniform(0,1,size=N)
        rn.values = values
        self.assertTrue((values == rn.values).all())
    
    def test_print(self):
        print(self)


class testDirLinks(unittest.TestCase):
    def setUp(self):
        self.nodes = cdict({i: Node(i) for i in range(5)})
        self.links = cset()
        self.link_sparse = [
            [1, 2, 0.1],
            [2, 1, 0.1],
            [1, 3, 0.5],
            [3, 4, 1.0],
            [4, 3, 1.5],
        ]
        for i, j, d in self.link_sparse:
            self.links += self.nodes[i].connect(self.nodes[j], d, directed=True)

    def test_links(self):
        self.assertEqual(
            len(self.links),
            5,
            f"wrong number of effective links in dirNetwork: {self.links}, {self.link_sparse}",
        )

    def test_density(self):
        for N in range(3, 100, 20):
            rn = dirNetwork.Random(N, 1)
            density = len(rn.links) / (N * (N - 1))
            self.assertEqual(
                density,
                1.0,
                f"Dense random network has density {density:.2f}"
                + f" instead of 1 (number_of_nodes = {N})\n"
                + f"links = {rn.links}\n"
                + f"links hash = {[hash(link) for link in rn.links]}",
            )

    def test_value_assignment(self):
        N = 100
        rn = dirNetwork.Random(N, 1)
        values = np.random.uniform(0,1,size=N)
        rn.values = values
        self.assertTrue((values == rn.values).all())
    
    def test_print(self):
        print(self)


if __name__ == "__main__":
    unittest.main()
