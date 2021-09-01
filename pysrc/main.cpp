#define BOOST_BIND_GLOBAL_PLACEHOLDERS 1
#include <pybind11/chrono.h>
#include <pybind11/iostream.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

//#include <geneva-interface/Go3.hpp>

#include "pagmo-interface.hpp"
#include "timedtests.hpp"

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

int add(int i, int j) { return i + j; }

namespace py = pybind11;
// PYBIND11_MAKE_OPAQUE(std::vector<double>);
PYBIND11_MODULE(pyneva, m)
{
  m.doc() = R"pbdoc(
        Pybind11 pyneva plugin
        -----------------------

        .. currentmodule:: pyneva plugin

        .. autosummary::
           :toctree: _generate

           add
           subtract
    )pbdoc";

  m.def("add", &add, R"pbdoc(
        Add two numbers

        Some other explanation about the add function.
    )pbdoc");

  m.def(
      "subtract", [](int i, int j) { return i - j; }, R"pbdoc(
        Subtract two numbers

        Some other explanation about the subtract function.
    )pbdoc");
  // custom wrapper for the Go3 EA Algorithm
  py::class_<GO3::Algorithms::Evolutionary>(m, "EA").def(
      py::init([](int iters) {
	return GO3::Algorithms::Evolutionary{.Iterations = iters, .algonum = 2};
      }),
      py::kw_only(), py::arg_v("iterations", 10));  // Algorithm ctor
						    //.def_readwrite("config",
  //		     &GO3::Algorithm_EA::cfg);	// access to cfg
  // attributes
  using dvec = std::vector<double>;
  // custom wrapper for the Go3 Population
  py::class_<GO3::Population>(m, "Population")
      .def(py::init(  // Population ctor
	       [](py::object functor, int size, int numParents,
		  std::vector<double> start, std::vector<double> left,
		  std::vector<double> right, int dim) {
		 assert(!left.empty() || !right.empty() || !start.empty() ||
			dim > 0);  // xor
		 if (start.empty()) {
		   if (!left.empty()) {
		     dim = left.size();
		     std::cout << "boundaries set, but no start values given\n "
				  "setting first individual between bounds"
			       << std::endl;
		     assert(left.size() == right.size());
		     std::generate_n(std::back_inserter(start), dim,
				     [&left, &right, n = 0]() mutable {
				       return (right[n] - left[n++]) / 2;
				     });
		   }
		   else
		     start.resize(dim);
		 }
		 else
		   dim = start.size();
		 // if unset set bounds to max/min

		 if (left.empty()) {
		   assert(dim > 0);
		   std::generate_n(
		       std::back_inserter(left), dim,
		       Gem::Geneva::GConstrainedValueLimitT<double>::lowest);
		   std::generate_n(
		       std::back_inserter(right), dim,
		       Gem::Geneva::GConstrainedValueLimitT<double>::highest);
		 }

		 auto ptr = std::make_shared<py::object>(functor);
		 ptr->inc_ref();  // inc_ref, as our shared_ptr shall also count
		 return GO3::Population{
		     start,
		     left,
		     right,
		     .Size = size,
		     .numParents = numParents,
		     [ptr](std::vector<double>& v) -> double {
		       assert(!v.empty());
		       // acquire GIL so the fitness threads can use the
		       // interpreter
		       py::gil_scoped_acquire acquire;
		       py::float_ r = (*ptr)(v);
		       return r.cast<double>();
		     },
		 };
	       }),
	   py::arg("f"), py::kw_only(), py::arg_v("size", 42),
	   py::arg_v("n_parents", 2), py::arg_v("start", std::vector<double>()),
	   py::arg_v("left", std::vector<double>()),
	   py::arg_v("right", std::vector<double>()), py::arg_v("dim", -1));
  //.def_readwrite("config",
  //		     &GO3::Population::cfg);  // access to cfg
  // attributes

  // custom wrapper for the Go3 Optimizer
  // custom return struct
  struct PynevaResult {
    double fitness;
    std::vector<double> values;
  };
  py::class_<GO3::bestMonitor, std::shared_ptr<GO3::bestMonitor>>(m, "Monitor")
      .def_readwrite("best", &GO3::bestMonitor::bests)
      .def(py::init([]() {
	return std::make_shared<GO3::bestMonitor>();
      }));  // Optimizer ctor
  py::enum_<GO3::ParMode>(m, "parmode")
      .value("networked", GO3::ParMode::networked)
      .value("serial", GO3::ParMode::serial)
      .value("threaded", GO3::ParMode::threaded);
  py::class_<PynevaResult>(m, "Result")
      .def_readwrite("values", &PynevaResult::values)
      .def_readwrite("fitness", &PynevaResult::fitness);
  py::class_<GO3::GenevaOptimizer3>(m, "GOptimizer")
      .def(py::init(  // Optimizer ctor
	       [](std::vector<std::string> argv_str, GO3::ParMode p,
		  size_t evalthreads,
		  std::vector<std::shared_ptr<GO3::bestMonitor>> monitors) {
		 std::vector<char*> argv(argv_str.size() + 1);
		 // dummy element since the first one is stripped in c++
		 argv[0] = (char*)"python";
		 std::generate(
		     argv.begin() + 1, argv.end(),
		     [n = 0, &argv_str]() mutable {
		       // evil const cast magic that should be changed
		       // in GenevaOptimizer
		       return const_cast<char*>(argv_str[n++].c_str());
		     });
		 // use unique_ptr as the go2 attribute of go3 is not moveable
		 std::cout << "argv:";
		 for (auto a : argv) std::cout << a;
		 std::vector<std::shared_ptr<GBasePluggableOM>> plugs(
		     monitors.size());
		 std::transform(monitors.begin(), monitors.end(), plugs.begin(),
				[](auto& ptr) { return ptr; });
		 return std::make_unique<GO3::GenevaOptimizer3>(
		     argv.size(), argv.data(), p, false, plugs, evalthreads);
	       }),
	   py::kw_only(), py::arg_v("cli_options", std::vector<std::string>()),
	   py::arg_v("parMode", GO3::ParMode::serial),
	   py::arg_v("evalthreads", std::thread::hardware_concurrency()),
	   py::arg_v("monitors",
		     std::vector<std::shared_ptr<GO3::bestMonitor>>{}))
      .def(
	  "optimize",
	  [](GO3::GenevaOptimizer3& self, GO3::Population& pop,
	     std::vector<GO3::Algorithms::Algorithm> algos,
	     bool isclient = false) {
	    // release GIL so other threads can use python Interpreter
	    pybind11::gil_scoped_release release;
	    auto best = self.optimize(pop, algos, isclient);
	    PynevaResult pr;
	    if (best) {
	      pr.fitness = best->raw_fitness();
	      best->streamline(pr.values);
	    }
	    return pr;
	  },
	  py::call_guard<py::scoped_ostream_redirect,
			 py::scoped_estream_redirect>());
  py::class_<Algo<pagmo::sea>>(m, "Pagmo_EA")
      .def(py::init([](int iters) {
	     return Algo<pagmo::sea>{.Iterations = iters, .algonum = 2};
	   }),
	   py::kw_only(),
	   py::arg_v("iterations", 10));  // Algorithm ctor
					  //.def_readwrite("config",
  py::class_<Algo<pagmo::sade>>(m, "Pagmo_SADE")
      .def(py::init([](int iters) {
	     return Algo<pagmo::sade>{.Iterations = iters, .algonum = 2};
	   }),
	   py::kw_only(),
	   py::arg_v("iterations", 10));  // Algorithm ctor
					  //.def_readwrite("config",
  py::class_<Algo<pagmo::nsga2>>(m, "Pagmo_nsga2")
      .def(py::init([](int iters) {
	     return Algo<pagmo::nsga2>{.Iterations = iters, .algonum = 2};
	   }),
	   py::kw_only(),
	   py::arg_v("iterations", 10));  // Algorithm ctor
					  //.def_readwrite("config",
  py::class_<pagmo::gaco>(m, "Pagmo_gaco")
      .def(py::init([](int iters, int ker) { return pagmo::gaco(iters, ker); }),
	   py::kw_only(), py::arg_v("iterations", 10),
	   py::arg_v("ker", 10));  // Algorithm ctor
				   //.def_readwrite("config",
  using pagmo_algorithmT =
      std::vector<std::variant<Algo<::pagmo::sea>, Algo<::pagmo::sade>,
			       Algo<::pagmo::nsga2>, ::pagmo::gaco>>;
  py::class_<PagmoOptimizer>(m, "POptimizer")
      .def(py::init([](GO3::ParMode p = GO3::ParMode::serial) {
	return PagmoOptimizer{.ts = (GO3::ParMode::threaded == p)
					? thread_safety::basic
					: thread_safety::none};
      }))
      .def(
	  "optimize",
	  [](PagmoOptimizer& self, GO3::Population& pop,
	     pagmo_algorithmT algos) {
	    // release GIL so other threads can use python Interpreter
	    pybind11::gil_scoped_release release;
	    auto pagmo_pop = self.optimize(pop, algos);
	    return PynevaResult{.fitness = pagmo_pop.champion_f()[0],
				.values = pagmo_pop.champion_x()};
	  },
	  py::call_guard<py::scoped_ostream_redirect,
			 py::scoped_estream_redirect>());
  m.def("rosenbrock", &rbrock_fitness);
  m.def("test_random_binding", &test_random_binding);
  m.def("test_random", &test_random);
  m.def("test_pagmo", &test_pagmo);

  m.def("test_geneva", [](size_t dim, const int popsize, nanoseconds waittime,
			  std::vector<std::string> argv_str, GO3::ParMode p) {
    std::vector<char*> argv(argv_str.size() + 1);
    // dummy element since the first one is stripped in c++
    argv[0] = (char*)"python";
    std::generate_n(argv.begin() + 1, argv_str.size(),
		    [n = 0, &argv_str]() mutable {
		      // evil const cast magic that should be changed
		      // in GenevaOptimizer
		      return const_cast<char*>(argv_str[n++].c_str());
		    });
    return test_geneva(dim, waittime, popsize, p, argv.data(), argv_str.size());
  });
  // use unique_ptr as the go2 attribute of go3 is not moveable
#ifdef VERSION_INFO
  m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
  m.attr("__version__") = "dev";
#endif
}
//#BOOST_CLASS_EXPORT(Gem::Geneva::GenericIndividual<ftype>);
