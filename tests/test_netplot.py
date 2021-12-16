import numpy as np
import unittest
from networkMDE import network, netplot
import cnets


class dummySuperNet(network.uniNetwork):
    def __init__(self, embedding_dimension):

        self.net = network.uniNetwork.Random(20, 0.7)
        for node in self.net:
            for link in node.synapses:
                link.length = np.float32(1.0)  # must be done using numpy types
        self.net.update_target_matrix()
        self.net.initialize_embedding(dim=embedding_dimension)
        self.net.cMDE(step=0.2, neg_step=0.1, Nsteps=1000)

        self.updated_times = 0

    def process_on_network(self):

        for node in self.net:
            Ztot = np.sum(
                1.0 / np.array(list(node.synapses.length))
            )  # classiter at work
            p_sum = 0
            for link in node.synapses:
                child = link.get_child(node)
                p = 1.0 / np.array(link.length) / Ztot
                if np.random.uniform(0, 1) < p:
                    child.value += 1
                    node.value -= 1
                    link.activation = p
                    link.length *= 1.0 / link.length + p * (
                        1.0 - 1.0 / link.length
                    )  # synapsis enhancement/ distance reduction
                else:
                    link.activation = np.float32(0.0)

        self.net.update_target_matrix()
        cnets.set_target(self.net.targetSM)
        self.updated_times += 1

    def update(self):
        if self.updated_times % 5 == 0 and self.updated_times < 25:
            self.apple_game()
        self.net.cMDE(step=0.1, neg_step=0.01, Nsteps=10)
        self.updated_times += 1


class netPlotStaticTest(unittest.TestCase):
    def setUp(self):
        self.net = network.uniNetwork.Random(10, 1.0)

    def test_2D_static_plot(self):
        self.net.initialize_embedding(dim=2)
        self.net.cMDE(step=0.1, neg_step=0.1, Nsteps=1000)
        netplot.plot_net(self.net)

    def test_3D_static_plot(self):
        self.net.initialize_embedding(dim=3)
        self.net.cMDE(step=0.1, neg_step=0.1, Nsteps=1000)
        netplot.plot_net(self.net)


class netPlotDynamicTest(unittest.TestCase):
    def test_2D_animation(self):
        self.super_net = dummySuperNet(2)
        animation = netplot.animate_super_network(
            self.super_net, self.super_net.update, frames=50, interval=60, blit=False
        )

    def test_3D_animation(self):
        self.super_net = dummySuperNet(3)
        animation = netplot.animate_super_network(
            self.super_net, self.super_net.update, frames=70, interval=60, blit=False
        )


if __name__ == "__main__":
    unittest.main()
