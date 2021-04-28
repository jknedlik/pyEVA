all:
	python setup.py install -DGENEVA_ROOT=~/.geneva_build 
	python setup.py build -DGENEVA_ROOT=~/.geneva_build 
clean:
	@rm -rf examples/CMakeFiles examples/GGenericStarter examples/Makefile examples/cmake_install.cmake examples/config/CMakeFiles examples/config/Makefile examples/config/cmake_install.cmake *.so pysrc/pyneva.egg-info _skbuild Makefile
