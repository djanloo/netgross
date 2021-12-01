'''simple model t do something with the network

Each node has a value V_n, I want to study the propagation of a signal over the
network. Possible ways:

- each node transmit to the child only if V_child < V_node, with probability d_cn/(sum_c d_cn)

the fact is that 'child' is a reflective property, so net will reach an equilibrium value like
the mean of the initial values.
'''

import numpy as np
import matplotlib.pyplot as plt
import network as nw
import netplot

from termcolor import colored
from timeit import default_timer as time
import cnets

class propagateNet(nw.Network):

    def __init__(self):
        self.net = nw.Network.Random(1000,.15)
        self.net.init_positions(dim=2)

        for node in self.net.nodes.values():
            node.value = 0# np.random.randint(20)

        self.net.nodes[0].value = 11

        self.net.max_expansion = 0
        
        cnets.init_network(self.net.targetSM, self.net.values, self.net.repr_dim)
        self.net.cMDE(Nsteps=1000)

        self.updated_times = 0

    def apple_game(self, verbose=False):

        for node in self.net.nodes.values():
            Ztot = np.sum(1./np.array(list(node.childs.values())))
            if verbose: print(f'epoch:{self.updated_times} node {node.n}: v = {node.value} --- Ztot = {Ztot : .3f}')
            for child, dist in node.childs.items():
                p = 1./(dist*Ztot)
                if verbose: print(f'\tchild {child.n : 3}: v = {child.value} --- p = {p : .2f}', end = '\t--> ')
                if  True:
                    if verbose: print(colored('transaction possible', 'blue'), end='\t--> ')
                    if np.random.uniform(0,1) < p:
                        if verbose: print(colored('apple given', 'green'))
                        child.value += 1
                        node.value -= 1
                        node.synapsis[child] = True

                        # synapsis enhancement/ distance reduction
                        node.childs[child] = node.childs[child]*0.9
                    else:
                        node.synapsis[child] = False
                        if verbose: print(colored('apple not given', 'red'))
                else:
                    node.synapsis[child] = False
                    if verbose: print(colored('transaction impossible','red'))

    def update(self, verbose=False):
        if int(self.updated_times) % 5 == 0:
            self.apple_game()
            self.net.expand(.01)
        self.net.cMDE(Nsteps=50)
        self.updated_times += 1



A = propagateNet()
# A.net.print_distanceM()

# animation = netplot.animate_super_network(A, A.update,
#                                            frames=150, interval=60, blit=True)
netplot.plot_net(A.net)
# animation.save('random_2d.gif',progress_callback = lambda i, n: print(f'Saving frame {i} of {n}', end='\r'), dpi=80)
plt.show()