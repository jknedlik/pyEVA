# -*- coding: utf-8 -*-
import pyneva as m

assert m.__version__ == "0.0.1"
assert m.add(1, 2) == 3
assert m.subtract(1, 2) == -1

A=m.EA(10);
B=m.EA(100);
pop=m.Population(20,2)
go3=m.GOptimizer(["-c","sc"])
print(go3.optimize(pop,[A,B]))
