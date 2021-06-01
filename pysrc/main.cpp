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

PYBIND11_MAKE_OPAQUE(std::vector<double>);
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
  py::class_<GO3::Algorithm_EA>(m, "EA")
      .def(py::init([](int iters) { return GO3::Algorithm_EA(iters); }),
	   py::kw_only(), py::arg_v("iterations", 10))	// Algorithm ctor
      .def_readwrite("config",
		     &GO3::Algorithm_EA::cfg);	// access to cfg attributes
  using dvec = std::vector<double>;
  using ftype = std::function<double(std::vector<double>)>;
  // custom wrapper for the Go3 Population
  py::class_<GO3::Population<ftype>>(m, "Population")
      .def(py::init(  // Population ctor
	       [](py::object functor, int size, int numParents) {
		 pybind11::gil_scoped_release nogil;
		 return GO3::Population<ftype>(
		     {5, 5, 5}, {0, 0, 0}, {10, 10, 10},
		     [functor](std::vector<double> v) -> double {
		       // return std::accumulate(v.begin(), v.end(), 0.0);
		       // std::string res = functor.attr("fitness")(v)
		       auto a = py::array(py::cast(v));
		       py::float_ r = functor.attr("fitness")(a);
		       // double r = 15.0;
		       return r.cast<double>();
		     },
		     size, numParents);
	       }),
	   py::arg("f"), py::kw_only(), py::arg_v("size", 42),
	   py::arg_v("n_parents", 2));
  // custom wrapper for the Go3 Optimizer
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
	  [](GO3::GenevaOptimizer3& self, GO3::Population<ftype>& pop,
	     GO3::algorithmsT algos) {
	    pybind11::gil_scoped_acquire acquire;
	    return (*self.optimize(pop, algos)).raw_fitness();
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
