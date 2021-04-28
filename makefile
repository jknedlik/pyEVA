all:
	python setup.py build -DGENEVA_ROOT=~/.geneva_build 
	python setup.py install --user -DGENEVA_ROOT=~/.geneva_build  -DPYBUILD=1
clean:
	@rm -rf examples/CMakeFiles examples/GGenericStarter examples/Makefile examples/cmake_install.cmake examples/config/CMakeFiles examples/config/Makefile examples/config/cmake_install.cmake *.so pysrc/pyneva.egg-info _skbuild Makefile
	python setup.py clean -DGENEVA_ROOT=~/.geneva_build 
test:
	python tests/test.py
