# -*- docker-image-name: "xrootd_base" -*-
# xrootd base image. Provides the base image for each xrootd service

ARG DEB_VER=debian:8.8
FROM jupyter/scipy-notebook
MAINTAINER jknedlik <j.knedlik@gsi.de>
USER root
RUN uname -a
RUN apt-get update --yes &&  apt-get install gpg  software-properties-common --yes
RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null
RUN apt-add-repository 'deb https://apt.kitware.com/ubuntu/ focal main'
RUN apt install kitware-archive-keyring && sudo rm /etc/apt/trusted.gpg.d/kitware.gpg
RUN apt-get update --yes && \
    apt-get install --yes --no-install-recommends gcc g++ cmake build-essential libtbb-dev 
RUN g++ --version
RUN git clone https://github.com/esa/pagmo2.git --branch v2.17.0 --single-branch --depth=1
RUN git clone https://github.com/jknedlik/geneva.git --branch python_client --single-branch --depth=1
RUN wget https://boostorg.jfrog.io/artifactory/main/release/1.75.0/source/boost_1_75_0.tar.gz
RUN tar -xvf boost_1_75_0.tar.gz
RUN cd boost_1_75_0 && ./bootstrap.sh && ./b2 install --prefix=/usr/local -j8
#RUN mkdir pagmo2/build && cd pagmo2/build && cmake -DPAGMO_BUILD_STATIC_LIBRARY=ON .. && make install -j8
RUN mkdir pagmo2/build && cd pagmo2/build && cmake .. && make install -j8
RUN mkdir geneva/build && cd geneva/build && cmake -DGENEVA_STATIC=1 -DCMAKE_POSITION_INDEPENDENT_CODE=true .. && make install -j8
RUN mkdir /pyneva
#COPY CMakeModules config/ examples/ pysrc/ tests/ CMakeLists.txt makefile pyproject.toml setup.py LICENSE /pyneva/ 
RUN python -m pip install --upgrade pip
RUN pip install scikit-build numpy pybind11
COPY . /pyneva/
RUN ls -la /pyneva
RUN cd /pyneva && LD_LIBRARY_PATH=/usr/local/lib make
RUN cd /pyneva && LD_LIBRARY_PATH=/usr/local/lib make test
RUN chmod -R 777 /home/jovyan
