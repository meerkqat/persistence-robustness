'''
Persistence visualizer. Much visual. Very persistence.
'''

import matplotlib.pyplot as plt
import re
import os
from collections import defaultdict
import random
import math
# from dionysus import *


class PersistenceItem:
    def __init__(self, betti, c):
        self.betti = int(betti)
        self.cycle = c
        self.bnd = []

    def add_variation(self, b, d):
        self.bnd.append((float(b), float(d)))

    def __repr__(self):
        return self.cycle + str(self.bnd)

    def radius(self):
        xs = [b[0] for b in self.bnd]
        ys = [b[1] for b in self.bnd]

        x_c = sum(xs)/len(xs)
        y_c = sum(ys)/len(ys)

        rs = [math.sqrt((b[0] - x_c) ** 2 + (b[1] - y_c) ** 2) for b in self.bnd]
        return max(rs)

class Visualiser:
    """
    parses files, produces visualizations and saves them
    """

    IN_FOLDER = "diagram_dataset/"
    OUT_FOLDER = "diagram_imgs/"
    # IN_FILES = re.compile(r"data1_d18_p1_i(\d).txt")
    IN_FILES = re.compile(r"data2_d7\.5_p1_i(\d).txt")
    LINES_NUMBER = 1000
    LINES_WIDTH = 0.3
    GROUPS_WIDTH = 0.3
    GROUP_RADIUS = 0.4
    EXISTS_IN_PERCENTAGE_ITERATIONS = 0.3
    SORT_LINES = False

    def __init__(self):
        # self.pdiags = []

        self.data = {}
        self._parse()

    def _parse(self):
        ctr = 0
        for fname in sorted(os.listdir(Visualiser.IN_FOLDER)):
            if not Visualiser.IN_FILES.match(fname):
                continue

            ctr += 1

            # self.pdiags.append(PersistenceDiagram(2))

            with open(Visualiser.IN_FOLDER + fname) as file:
                for line in file:
                    betti, start, end, cycle = line.strip().split(' ')

                    # self.pdiags[-1].append((float(start), float(end), int(betti)))

                    if cycle not in self.data:
                        self.data[cycle] = PersistenceItem(betti, cycle)

                    self.data[cycle].add_variation(start, end)
        
        # remove short lived
        for k in list(self.data.keys()):
            if len(self.data[k].bnd) < ctr * Visualiser.EXISTS_IN_PERCENTAGE_ITERATIONS:
                del self.data[k]

        # filter to max_radius
        self.data = {c: v for c, v in self.data.items() if v.radius() < Visualiser.GROUP_RADIUS}


        # filter to LINES_NUMBER
        new_keys = list(self.data.keys())
        random.shuffle(new_keys)
        new_data = []
        for i in range(3):
            new_data.extend([k for k in new_keys if self.data[k].betti == i][:Visualiser.LINES_NUMBER])


        self.data = {k: self.data[k] for k in new_data}
        

    def visualize(self):
        self._visualize_groups()
        self._visualize_lines()

    def _visualize_groups(self):
        plt.title("Persistence diagram 1")
        plt.hold(True)

        XY = []
        for _, v in self.data.items():
            x, y = [d[0] for d in v.bnd], [d[1] for d in v.bnd]
            clr = self._color(v.betti)
            plt.plot(x, y, clr, linewidth=Visualiser.GROUPS_WIDTH)

            XY.extend(x)
            XY.extend(y)

        delta = 0.3
        mx = max(XY)

        plt.plot([0, mx], [0, mx], 'k')
        plt.axis((-delta, mx + delta, -delta, mx + delta))

        fout = ("r{}_n{}_".format(Visualiser.GROUP_RADIUS, Visualiser.LINES_NUMBER) + Visualiser.IN_FILES.pattern).replace(".txt", ".svg").replace("_i(\\d)", "groups")
        plt.savefig(fout)
    
    def _color(self, dim, delta=70):        
        clr = [random.randint(0, delta), random.randint(0, delta), random.randint(0, delta)]
        clr[dim] = 0
        clr[dim] = 255 - sum(clr)
        
        return "#{:02x}{:02x}{:02x}".format(*clr)

    def _visualize_lines(self):
        fig = plt.figure()
        fig.suptitle('Persistence diagram 2')
        hax0 = fig.add_subplot(311)
        haxs = [hax0, fig.add_subplot(312, sharex=hax0), fig.add_subplot(313, sharex=hax0)]

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

            print(len(line),i)
            hax.set_ylabel('H' + str(i))
            draw = [(0, 0)] + line + [(0, 0)]
            hax.hlines(range(len(draw)), [i[0] for i in draw], [i[1] for i in draw], colors=self._color(i, 0), lw=Visualiser.LINES_WIDTH)
            hax.yaxis.set_visible(False)
            hax.xaxis.set_visible(False)

        fout = ("r{}_n{}_".format(Visualiser.GROUP_RADIUS, Visualiser.LINES_NUMBER) + Visualiser.IN_FILES.pattern).replace(".txt", ".svg").replace("_i(\\d)", "lines")
        plt.savefig(fout)

if __name__ == '__main__':
    vis = Visualiser()
    vis.visualize()

