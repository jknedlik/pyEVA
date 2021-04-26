// Standard header files go here
#include <iostream>

// The new geneva optimizer interface #3
#include <geneva-interface/Go3.hpp>

// The lambda to be used as evaluation function
auto lambda = [](std::vector<double>& a) {
  return std::accumulate(a.begin(), a.end(), 0.0);
};

int main(int argc, char** argv)
{
  using namespace GO3;
  /*Set boundaries (start value, left & right)*/
  std::vector<double> start{50, 50, 50}, left{0, 0, 0}, right{100, 100, 100};
  /*create optimizer,give CLI options */
  GenevaOptimizer3 go3{argc, argv};
  /* create ea , for easy handling*/
  Algorithm_EA ea{.Iterations = 30};
  /* change config parameters via [] */
  ea[cfg::Iterations] = 100;  // only accepts cfg's for EA for now...
  auto pop = Population{start, left, right, .func = lambda, .Iterations = {10}};
  /* change population config */
  pop[cfg::Size] = 1000;
  /* use the optimizer, add an Inplace GD */
  auto best_ptr = go3.optimize(pop, {ea, Algorithm_GD{.Iterations = 1005}});
  std::cout << best_ptr << std::endl;
}
