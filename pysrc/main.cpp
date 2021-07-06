#define BOOST_BIND_GLOBAL_PLACEHOLDERS 1
#include <pybind11/iostream.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

#include <geneva-interface/Go3.hpp>

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

int add(int i, int j) { return i + j; }

namespace py = pybind11;

using ftype = std::function<double(std::vector<double>)>;
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
  py::class_<GO3::Algorithm_EA>(m, "EA").def(
      py::init([](int iters) {
	return GO3::Algorithm_EA{.Iterations = iters, .algonum = 2};
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
		 assert(left.empty() != right.empty());	 // xor
		 assert(!start.empty() || dim > 0);
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

		 return GO3::Population{
		     start,
		     left,
		     right,
		     .Size = size,
		     .numParents = numParents,
		     [functor](std::vector<double>& v) -> double {
		       assert(!v.empty());
		       // acquire GIL so the fitness threads can use the
		       // interpreter
		       py::gil_scoped_acquire acquire;
		       py::float_ r = functor.attr("__call__")(v);
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
  py::class_<PynevaResult>(m, "Result")
      .def_readwrite("values", &PynevaResult::values)
      .def_readwrite("fitness", &PynevaResult::fitness);
  py::class_<GO3::GenevaOptimizer3>(m, "GOptimizer")
      .def(py::init(  // Optimizer ctor
	       [](std::vector<std::string> argv_str) {
		 std::vector<char*> argv(argv_str.size() + 1);
		 // dummy element since the first one is stripped in c++
		 argv[0] = (char*)"python";
		 std::generate(
		     argv.begin() + 1, argv.end(),
		     [n = 0, &argv_str]() mutable {
		       // evil const cast magic that should be changed in
		       // GenevaOptimizer
		       return const_cast<char*>(argv_str[n++].c_str());
		     });
		 // use unique_ptr as the go2 attribute of go3 is not moveable
		 std::cout << "argv:";
		 for (auto a : argv) std::cout << a;
		 return std::make_unique<GO3::GenevaOptimizer3>(argv.size(),
								argv.data());
	       }),
	   py::kw_only(), py::arg_v("cli_options", std::vector<std::string>()))
      .def(
	  "optimize",
	  [](GO3::GenevaOptimizer3& self, GO3::Population& pop,
	     GO3::algorithmsT algos) {
	    // release GIL so other threads can use python Interpreter
	    pybind11::gil_scoped_release release;
	    auto best = self.optimize(pop, algos);
	    PynevaResult pr;
	    if (best) {
	      pr.fitness = best->raw_fitness();
	      best->streamline(pr.values);
	    }
	    return pr;
	  },
	  py::call_guard<py::scoped_ostream_redirect,
			 py::scoped_estream_redirect>())

      ;
#ifdef VERSION_INFO
  m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
  m.attr("__version__") = "dev";
#endif
}
//#BOOST_CLASS_EXPORT(Gem::Geneva::GenericIndividual<ftype>);
