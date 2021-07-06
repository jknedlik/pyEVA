
#include <geneva-interface/Go3.hpp>
#include <iostream>
#include <pagmo/algorithm.hpp>
#include <pagmo/algorithms/nsga2.hpp>
#include <pagmo/algorithms/sea.hpp>
#include <pagmo/population.hpp>
#include <pagmo/problem.hpp>
#include <pagmo/problems/dtlz.hpp>
#include <pagmo/types.hpp>

using namespace pagmo;
struct pyneva_python_problem {
  std::function<double(vector_double &)> func{[](vector_double &dv) -> double {
    throw std::runtime_error("no function defined");
    return 0.0;
  }};
  vector_double left{};
  vector_double right{};
  vector_double fitness(const vector_double &dv) const
  {
    vector_double r{dv};
    return vector_double{func(r)};
  }
  using pair_v_d = std::pair<vector_double, vector_double>;
  pair_v_d get_bounds() const { return {left, right}; }
};
template <typename pagmo_algo>
struct Algo : pagmo_algo {
  int Iterations{10};
  int algonum{0};
  template <typename Integral>
  constexpr auto &operator[](Integral)	// adress via 0,1 and integrals
  {
    return std::get<Integral::value>(
	std::forward_as_tuple(Iterations, algonum));
  }
};
using AlgorithmT = std::vector<std::variant<Algo<::pagmo::sea>>>;
struct PagmoOptimizer {
  auto optimize(GO3::Population &gpop, AlgorithmT algos)
  {
    // define problem
    problem prob{pyneva_python_problem{
	.func = gpop.func, .left = gpop.left, .right = gpop.right}};
    // create pop from gpop
    population pop{prob, static_cast<size_t>(gpop.Size)};

    for (auto algo : algos)
      std::visit(
	  [&](auto &al) {
	    for (int i = 0; i < al[GO3::cfg::Iterations]; i++)
	      pop = al.evolve(pop);
	  },
	  algo);
    return pop;
  }
};
