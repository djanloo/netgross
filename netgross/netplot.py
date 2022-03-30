"""Graphic module for plotting and animating networks.
"""
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib.colors import hsv_to_rgb

from networkMDE import network

plt.rc("font", family="serif")

# Default setttings for plots
fig_kwargs = {"figsize": (5, 5)}
scat_kwargs = {"cmap": "viridis", "s": 30, "alpha": 1}
line_kwargs = {"color": "k", "alpha": 1, "lw": 0.4}
plot_lines = True
plot_points = True

# This must be filled with the return
# mappable from set_array for colorbars
mappable = None


def get_graphics(net):
    """Generates the figure, axis and empty scatterplot/lines for the net."""

    # creates figure and axis
    fig = plt.figure(**fig_kwargs)
    ax = fig.add_subplot(projection="3d") if net.repr_dim == 3 else fig.add_subplot()

    # length scale is useless (?)
    ax.axis("off")

    empty = [[], [], []] if ax.name == "3d" else [[], []]

    # In 2d plots points over lines if not differently specified
    if ax.name != "3d":
        line_kwargs.setdefault("zorder", 0)
        scat_kwargs.setdefault("zorder", 1)

    # plots points
    net.scatplot = ax.scatter(*empty, **scat_kwargs)
    artists = (net.scatplot,)

    if plot_lines:
        for link in net.links:
            # line_data = np.vstack((link.node1.position, link.node2.position)).transpose()
            # line, = ax.plot(*line_data, color='k', alpha=0.3)
            (line,) = ax.plot(*empty, **line_kwargs)
            link.line = line
            artists += (line,)

    return fig, ax


def update_scatter(ax, net, colors, normalize_colors=True):
    """Updates the scatter plot position and colors"""

    position = net.to_scatter()
    scat = net.scatplot

    if ax.name == "3d":
        x_coord, y_coord, z_coord = position
    else:
        x_coord, y_coord = position

    # NOTE ABOUT Path3Dcollection (matplotib v3.5.0)
    # set_offsets takes ([x1, y1], [x2, y2], ...) and
    # not ([x1, x2, ..], [y1, y2, ...]) as a good boy would do
    # also, z coordinates must be set AFTER the x-y coordinates

    position_on_plane = tuple(np.vstack((x_coord, y_coord)).transpose())
    scat.set_offsets(position_on_plane)
    if ax.name == "3d":
        scat.set_3d_properties(z_coord, "z")
        ax.set_zlim((np.min(z_coord), np.max(z_coord)))

    # Set padding
    min_, max_ = np.min(x_coord), np.max(x_coord)
    min_, max_ = min_ - 0.1 * (max_ - min_), max_ + 0.1 * (max_ - min_)
    ax.set_xlim((min_, max_))

    min_, max_ = np.min(y_coord), np.max(y_coord)
    min_, max_ = min_ - 0.1 * (max_ - min_), max_ + 0.1 * (max_ - min_)
    ax.set_ylim((min_, max_))

    # colors
    mappable = scat.set_array(np.array(list(colors)))
    if normalize_colors:
        vmin, vmax = min(colors), max(colors)
        scat.set_clim(vmin, vmax)


def update_lines(ax, net, colors, alphas):
    """updates the lines of the network"""
    for link, color, alpha in zip(net.links, colors, alphas):

        line_data = np.vstack((link.node1.position, link.node2.position)).transpose()

        if ax.name == "3d":
            x_coord, y_coord, z_coord = line_data
        else:
            x_coord, y_coord = line_data

        link.line.set_data(x_coord, y_coord)
        link.line.set_color(color)
        link.line.set_alpha(alpha)

        if ax.name == "3d":
            link.line.set_3d_properties(z_coord)


def animate_super_network(super_net, super_net_function, **anim_kwargs):
    """Animates the super_network values evolution function

    Note
    ----
        The update function can include an MDE update on position
    """
    fig, ax = get_graphics(super_net.net)

    def _update_graphics(_):

        super_net_function()
        super_net.net.distortion_activation()
        activations = super_net.net.links.activation

        point_colors = super_net.net.nodes.value
        line_colors = np.array([])
        for a in activations:
            if a > 0:
                line_colors = np.append(line_colors, hsv_to_rgb((0.0, 1.0, a)), axis=0)
            else:
                line_colors = np.append(line_colors, hsv_to_rgb((0.5, 1.0, -a)), axis=0)
        line_colors = line_colors.reshape(-1, 3)
        line_alpha = [1.0 for a in activations]

        update_scatter(ax, super_net.net, point_colors)
        if plot_lines:
            update_lines(ax, super_net.net, line_colors, line_alpha)
        return (super_net.net.scatplot,) + tuple(super_net.net.links.line)

    super_net.net.animation = animation.FuncAnimation(
        fig, _update_graphics, **anim_kwargs
    )
    return super_net.net.animation


def animate_MDE(net, updates_per_frame, MDEkwargs, anim_kwargs):
    class dummy:
        def __init__(self):
            self.net = net

        def update(self):
            for _ in range(updates_per_frame):
                self.net.cMDE(**MDEkwargs)

    supernet = dummy()
    animate_super_network(supernet, supernet.update, **anim_kwargs)
    return supernet.net.animation


def plot_net(net, labels=None, colorbar=False):
    """Plots a statical image for the network embedding"""
    print("Plot started:")
    print("Getting graphics..", end="", flush=True)
    _, ax = get_graphics(net)
    print("\tDone.")

    activations = net.links.activation

    point_colors = net.nodes.value
    line_colors = np.array([])
    for a in activations:
        if a > 0:
            line_colors = np.append(line_colors, hsv_to_rgb((0.0, 1.0, a)), axis=0)
        else:
            line_colors = np.append(line_colors, hsv_to_rgb((0.5, 1.0, -a)), axis=0)

    line_colors = line_colors.reshape(-1, 3)
    line_alpha = [
        0.2 + 0.8 * abs(a) for a in activations
    ]  # [0.2 + 0.8 * a for a in activations]

    if plot_points:
        print("Updating scatter..", end="", flush=True)
        update_scatter(ax, net, point_colors)
        print("\tDone.")

    if plot_lines:
        print("Updating lines..", end="", flush=True)
        update_lines(ax, net, line_colors, line_alpha)
        print("\tDone.")

    if labels is not None:
        if ax.name == "3d":
            for node in net:
                # ax.text(x, y, z, label, zdir)
                ax.text(*node.position, labels[node.n], None, size=11)
        else:
            for node in net:
                ax.annotate(labels[node.n], tuple(node.position), size=11)
    if colorbar:
        plt.colorbar(mappable=net.scatplot, ax=ax)


def plot_links(net):
    _, ax = plt.subplots()
    ax.imshow(net.linkM.astype(float), cmap="gray")
