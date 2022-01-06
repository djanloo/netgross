import numpy as np
import unittest
from networkMDE.classiter import cdict, cset, clist
from networkMDE.network import Node, uniLink, uniNetwork

class testNodeLinks(unittest.TestCase):
    def setUp(self):
        self.nodes = cdict({i: Node(i) for i in range(5)})
        self.links = cset()
        self.link_sparse = [
            [1, 2, 0.1],
            [2, 1, 0.1],
            [1, 3, 0.5],
            [3, 4, 1.0],
            [4, 3, 1.0],
        ]
        for i, j, d in self.link_sparse:
            self.links += self.nodes[i].connect(self.nodes[j], d)

    def test_links(self):
        self.assertEqual(
            len(self.links),
            3,
            f"uniLink(i,j) should be equivalent to uniLink(j,i): {self.links}, {self.link_sparse}",
        )

    def test_density(self):
        for N in range(3, 100, 20):
            rn = uniNetwork.Random(N, 1)
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
        rn = uniNetwork.Random(N, 1)
        values = np.random.uniform(0,1,size=N)
        rn.values = values
        self.assertTrue((values == rn.values).all())


if __name__ == "__main__":
    unittest.main()
