import os
from dionysus import *
# export PYTHONPATH= ... Dionysus/build/bindings/python

print "Reading in files..."

fdiag = os.listdir("diagrams")

diags = []

for fname in fdiag:
	diags.append(PersistenceDiagram(2))
	with open("diagrams/"+fname, "r") as f:
		for line in f:
			banana = line.split(" ")
			pt = (float(banana[1]), float(banana[2]), int(banana[0]))
			diags[-1].append(pt)

print "Calculating distances..."

dists = []

for i in range(len(diags)):
	dists.append(bottleneck_distance(diags[0], diags[i]))

print dists