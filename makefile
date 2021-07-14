all:
	python setup.py build  
	python setup.py install --user -DPYBUILD=1
clean:
	@rm -rf examples/CMakeFiles examples/GGenericStarter examples/Makefile examples/cmake_install.cmake examples/config/CMakeFiles examples/config/Makefile examples/config/cmake_install.cmake *.so pysrc/pyneva.egg-info _skbuild Makefile
	python setup.py clean 
test:
	python tests/pagmo.py
	python tests/geneva.py
docker:
	docker build --tag "pyneva:latest" .

