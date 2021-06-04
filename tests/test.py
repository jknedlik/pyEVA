# -*- coding: utf-8 -*-
import pyneva as m
import sys

assert m.__version__ == "0.0.1"

ea1=m.EA(iterations=10);
pop=m.Population(f=lambda x:sum(x,0),size=20,n_parents=1,
        left =[0,0,0],
        #start=[5,5,5],
        right=[9,9,9])
go3=m.GOptimizer(cli_options=sys.argv)
fitness=go3.optimize(pop,[ea1,m.EA(iterations=100)])
assert(fitness <0.05)
