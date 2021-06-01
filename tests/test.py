# -*- coding: utf-8 -*-
import pyneva as m

assert m.__version__ == "0.0.1"
assert m.add(1, 2) == 3
assert m.subtract(1, 2) == -1

A=m.EA(iterations=10);
B=m.EA(iterations=100);
pop=m.Population(size=20,n_parents=4)
#go3=m.GOptimizer(["--showAll"])
go3=m.GOptimizer(cli_options=["-c","sc",])
print(go3.optimize(pop,[A,B]))
