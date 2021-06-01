# -*- coding: utf-8 -*-
import pyneva as m

assert m.__version__ == "0.0.1"
assert m.add(1, 2) == 3
assert m.subtract(1, 2) == -1
A=m.EA(10);
print(dir(A.config))
print(A.config[1])
x=A.config
pop=m.Population(10,5)
go3=m.GOptimizer(["--client"])
