'''
Persistence visualizer. Much visual. Very persistence.
'''

import matplotlib.pyplot as plt
import re
import os
from collections import defaultdict
import random


class PersistenceItem:
    def __init__(self, betti, c):
        self.betti = int(betti)
        self.cycle = c
        self.bnd = []

    def add_variation(self, b, d):
        self.bnd.append((float(b), float(d)))

    def __repr__(self):
        return self.cycle + str(self.bnd)


class Visualiser:
    """
    parses files, produces visualizations and saves them
    """

    IN_FOLDER = "diagram_dataset/"
    IN_FILES = re.compile(r"data2_d4_p1_i(\d).txt")
    OUT_FOLDER = "diagram_imgs/"
    LINES_NUMBER = 200
    LINES_WIDTH = 0.3
    SORT_LINES = False

    def __init__(self):
        self.data = {}
        self._parse()

    def _parse(self):
        ctr = 0
        for fname in sorted(os.listdir(Visualiser.IN_FOLDER)):
            if not Visualiser.IN_FILES.match(fname):
                continue

            ctr += 1

            with open(Visualiser.IN_FOLDER + fname) as file:
                for line in file:
                    betti, start, end, cycle = line.strip().split(' ')

                    if cycle not in self.data:
                        self.data[cycle] = PersistenceItem(betti, cycle)

                    self.data[cycle].add_variation(start, end)

    def visualize(self):
        self._visualize_groups()
        self._visualize_lines()

    def _visualize_groups(self):
        plt.title("Persistence diagram 1")
        plt.hold(True)

        XY = []
        for _, v in self.data.items():
            x, y = [d[0] for d in v.bnd], [d[1] for d in v.bnd]
            clr = ("g" if v.betti == 0 else "b" if v.betti == 1 else "r") + "-"
            plt.plot(x, y, clr, linewidth=0.3)

            XY.extend(x)
            XY.extend(y)

        delta = 0.3
        mx = max(XY)

        plt.plot([0, mx], [0, mx], 'k')
        plt.axis((-delta, mx + delta, -delta, mx + delta))

        fout = ("" + Visualiser.IN_FILES.pattern).replace(".txt", ".svg").replace("_i(\\d)", "groups")
        plt.savefig(fout)

    def _visualize_lines(self):
        fig = plt.figure()
        fig.suptitle('Persistence diagram 2')
        hax0 = fig.add_subplot(311)
        hax1 = fig.add_subplot(312, sharex=hax0)
        hax2 = fig.add_subplot(313, sharex=hax1)
    
        haxs = [hax0, hax1, hax2]
        colors = ['g', 'r', 'b']

        lines = defaultdict(list)
        for _, line in self.data.items():
            b = sum([i[0] for i in line.bnd]) / len(line.bnd)
            d = sum([i[1] for i in line.bnd]) / len(line.bnd)
            lines[line.betti].append((b, d))

        for i, line in lines.items():
            hax = haxs[i]

            if Visualiser.SORT_LINES:
                line = sorted(line, key=lambda x: x[0] - x[1])
            else:
                random.shuffle(line)

            line = line[:Visualiser.LINES_NUMBER] if len(line) > Visualiser.LINES_NUMBER else line

            hax.set_ylabel('H' + str(i))
            hax.hlines(range(len(line)), [i[0] for i in line], [i[1] for i in line], colors=colors[i], lw=Visualiser.LINES_WIDTH)
            hax.yaxis.set_visible(False)
            hax.xaxis.set_visible(False)

        fout = ("" + Visualiser.IN_FILES.pattern).replace(".txt", ".svg").replace("_i(\\d)", "lines")
        plt.savefig(fout)

if __name__ == '__main__':
    vis = Visualiser()
    vis.visualize()

