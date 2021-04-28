pyEVA
==============




A tool for optimization using distribtued EVolutionary Algorithms based on the geneva framework.
Python unterface usingexample project built with [pybind11](https://github.com/pybind/pybind11) and scikit-build.



Installation
------------

**On Unix (Linux, macOS)**

 - clone this repository
 - build using `python setup.py install -DGENEVA_ROOT=~/.geneva_build`
 - install using `python setup.py build -DGENEVA_ROOT=~/.geneva_build`
 - or using the simple makefile `make`


License
-------

geneva is provided under a Apache 2-style license that can be found in the LICENSE
file. By using, distributing, or contributing to this project, you agree to the
terms and conditions of this license.

Test call
---------

```python
import pyneva
pyneva.add(1, 2)
```

[`cibuildwheel`]:          https://cibuildwheel.readthedocs.io
