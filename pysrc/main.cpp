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
  py::class_<GO3::Algorithm_EA>(m, "EA")
      .def(py::init<int>())  // Algorithm ctor
      .def_readwrite("config",
		     &GO3::Algorithm_EA::cfg);	// access to attributes
  // custom wrapper for Population Wrapper
  using ftype = std::function<double(std::vector<double>)>;
  using dvec = std::vector<double>;
  py::class_<GO3::Population<ftype>>(m, "Population")
      .def(py::init([](int size, int numParents) {
	return GO3::Population<ftype>(
	    {5, 5, 5}, {0, 0, 0}, {10, 10, 10},
	    [](std::vector<double> v) {
	      return std::accumulate(v.begin(), v.end(), 0);
	    },
	    size, numParents);
      }));
  py::class_<GO3::GenevaOptimizer3>(m, "GOptimizer")
      //.def(py::init([](py::list argv_list) {
      .def(py::init([](std::vector<std::string> argv_str) {
	// std::vector<std::string> argv_str;  // argv using strings
	// copy into
	// for (py::handle obj : argv_list)
	//  argv_str.push_back(obj.attr("__str__")().cast<std::string>());
	// argv as char* using that underlying memory
	std::vector<char*> argv(argv_str.size());
	std::generate(argv.begin(), argv.end(), [n = 0, &argv_str]() mutable {
	  // evil const cast magic that should be changed in GenevaOptimizer
	  return const_cast<char*>(argv_str[n++].c_str());
	});
	return std::make_unique<GO3::GenevaOptimizer3>(argv.size(),
						       argv.data());
      }));

#ifdef VERSION_INFO
  m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
  m.attr("__version__") = "dev";
#endif
}
