# -*- coding: utf-8 -*-
import pyneva as m
import sys

assert m.__version__ == "0.0.1"

ea1=m.Pagmo_SADE(iterations=100)
pop=m.Population(f=lambda x:sum(x,0),size=20,n_parents=1,
        left =[0,0,0],
        #start=[5,5,5],
        right=[9,9,9])
po3=m.POptimizer()
best=po3.optimize(pop,[ea1,m.Pagmo_EA(iterations=1)])
print(f"best:{best.fitness},{best.values}")
assert(best.fitness<0.05)
