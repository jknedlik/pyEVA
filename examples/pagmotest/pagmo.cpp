
#include <iostream>
#include <pagmo/algorithm.hpp>
#include <pagmo/algorithms/nsga2.hpp>
#include <pagmo/algorithms/sea.hpp>
#include <pagmo/population.hpp>
#include <pagmo/problem.hpp>
#include <pagmo/problems/dtlz.hpp>
#include <pagmo/types.hpp>

using namespace pagmo;

struct accumulate_problem {
  vector_double fitness(const vector_double &dv) const
  {
    return {std::accumulate(dv.begin(), dv.end(), 0.0)};
  }

  std::pair<vector_double, vector_double> get_bounds() const
  {
    return {{0., 0., 0.}, {9., 9., 9.}};
  }
};
int main()
{
  // 1 - Instantiate a pagmo problem constructing it from a UDP
  // (user defined problem).
  problem prob{accumulate_problem{}};

  // 2 - Instantiate a pagmo algorithm
  auto ea = sea(100);
  ea.set_verbosity(10);
  algorithm algo{ea};

  // 3 - Instantiate a population
  population pop{prob, 48};

  // 4 - Evolve the population

  for (int i = 0; i < 10; i++) pop = algo.evolve(pop);

  // 5 - Output the population
  std::cout << "The population: \n" << pop;
}
