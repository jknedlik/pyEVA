#define BOOST_BIND_GLOBAL_PLACEHOLDERS 1
#include <chrono>
#include <iostream>
#include <pagmo/algorithm.hpp>
#include <pagmo/algorithms/de.hpp>
#include <pagmo/problem.hpp>
#include <pagmo/problems/rosenbrock.hpp>
#include <pagmo/types.hpp>
#include <thread>

using namespace std::chrono;
using std::cout;
using std::endl;
template <typename timeT = nanoseconds>
struct timed {
  static std::vector<float> ms;
  static auto print_avg()
  {
    return std::accumulate(timed::ms.begin(), timed::ms.end(), 0) /
	   timed::ms.size();
  };
  time_point<high_resolution_clock> start{(high_resolution_clock::now())};
  virtual ~timed()
  {
    ms.push_back(
	duration_cast<timeT>(high_resolution_clock::now() - start).count());
  }
};
template <typename timeT>
std::vector<float> timed<timeT>::ms{};

std::mt19937 gen32(123);
std::vector<double> Randomvector(int vsize = 2)
{
  // std::vector<double> res;
  // std::generate_n(std::back_inserter(res), vsize,
  //		  std::bind(&std::mt19937::operator(), &gen32));

  return std::vector<double>(vsize, 2);
}
template <typename T, typename timeT = std::chrono::microseconds>
auto test_and_time_v(T&& t, size_t vsize, size_t tsize = 100,
		     std::string fn = "", size_t batchfactor = 1)
{
  timed<timeT>::ms.clear();
  for (int i = 0; i < tsize; i++) {
    timed<timeT> _;
    auto v = Randomvector(vsize);
    volatile auto a = t(v);
  }
  return timed<timeT>::print_avg() / static_cast<float>(batchfactor);
}
template <typename T, typename timeT = std::chrono::microseconds>
auto test_and_time(T&& t, size_t tsize = 100, std::string fn = "",
		   size_t batchfactor = 1)
{
  timed<timeT>::ms.clear();
  for (int i = 0; i < tsize; i++) {
    timed<timeT> _;
    volatile auto a = t();
  }
  return timed<timeT>::print_avg() / static_cast<float>(batchfactor);
}
using pagmo::thread_safety;
using std::chrono::microseconds;
using std::chrono::milliseconds;
template <typename Duration>
struct rbrock {
  size_t dim{3};
  pagmo::rosenbrock rb{dim};
  Duration dur{};
  thread_safety get_thread_safety() const { return thread_safety::none; }
  auto fitness(const std::vector<double>& v) const
  {
    std::this_thread::sleep_for(dur);
    return rb.fitness(v);
  };
  auto get_bounds() const { return rb.get_bounds(); }
};
rbrock<nanoseconds> r{.dim = 3, .dur = 0ns};
auto rbrock_fitness(std::vector<double> v) { return r.fitness(v).at(0); }
// using rbrock = pagmo::rosenbrock;
//
auto test_random(size_t dim, nanoseconds waittime, const int popsize)
{
  using std::placeholders::_1, std::placeholders::_2;
  auto nsec = std::chrono::duration_cast<nanoseconds>(waittime);
  r = rbrock<nanoseconds>{.dim = dim, .dur = nsec};
  auto bind = std::bind(&rbrock<nanoseconds>::fitness, r, _1);
  return test_and_time_v(bind, dim, popsize, "Random");
}

auto test_random_binding(size_t dim, nanoseconds waittime, const int popsize)
{
  using std::placeholders::_1, std::placeholders::_2;
  using namespace pagmo;
  auto nsec = std::chrono::duration_cast<nanoseconds>(waittime);
  r = rbrock<nanoseconds>{.dim = dim, .dur = nsec};
  return test_and_time_v(rbrock_fitness, dim, popsize,
			 "Random, no std::bind()");
}

auto test_pagmo(size_t dim, nanoseconds waittime, const int popsize)
{
  using namespace pagmo;
  auto nsec = std::chrono::duration_cast<nanoseconds>(waittime);
  problem prob(rbrock<nanoseconds>{.dim = dim, .dur = nsec});
  de d{};
  auto rx = std::bind(&de::evolve, d, population(prob, popsize));
  // assert(d.get_log().rbegin()->Fevals == popsize);
  return test_and_time(rx, 1, "pagmo::de", popsize) / 1.0;
}

auto test_geneva(size_t dim, nanoseconds waittime, const int popsize,
		 GO3::ParMode p, char** argv, int argc)
{
  auto nsec = std::chrono::duration_cast<nanoseconds>(waittime);
  r = rbrock<nanoseconds>{.dim = dim, .dur = nsec};
  using namespace GO3;
  using Algorithms::Algorithm;
  using Algorithms::Evolutionary;
  Population pop{.start = std::vector<double>(dim, 2),
		 .left = r.get_bounds().first,
		 .right = r.get_bounds().second,
		 .Size = popsize,
		 .func = rbrock_fitness};
  bool isClient{argc > 1 && GO3::ParMode::networked == p};
  auto tm = std::make_shared<timemonitor>();
  GenevaOptimizer3 go3(argc, argv, p, argc == 0, {tm});
  go3.optimize(pop, {Algorithm{Evolutionary{.Iterations = 1}}}, isClient);
  return tm->dur.count() / (float)popsize / 1000;
}

