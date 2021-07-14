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
# build boost,pagmo and geneva
RUN git clone https://github.com/esa/pagmo2.git --branch v2.17.0 --single-branch --depth=1
RUN git clone https://github.com/jknedlik/geneva.git --branch python_client --single-branch --depth=1
RUN wget https://boostorg.jfrog.io/artifactory/main/release/1.75.0/source/boost_1_75_0.tar.gz
RUN tar -xvf boost_1_75_0.tar.gz
RUN cd boost_1_75_0 && ./bootstrap.sh && ./b2 install --prefix=/usr/local -j8
RUN mkdir pagmo2/build && cd pagmo2/build && cmake .. && make install -j8
RUN mkdir geneva/build && cd geneva/build && cmake -DGENEVA_STATIC=1 -DCMAKE_POSITION_INDEPENDENT_CODE=true .. && make install -j8
RUN mkdir /pyneva
#build pyneva
RUN python -m pip install --upgrade pip
RUN pip install scikit-build numpy pybind11 pymc3 covid19_inference
COPY . /pyneva/
RUN cd /pyneva && LD_LIBRARY_PATH=/usr/local/lib make
RUN cd /pyneva && LD_LIBRARY_PATH=/usr/local/lib make test
RUN chmod -R 777 /home/jovyan
RUN conda install mkl-service 
RUN rm -rf geneva pagmo2 boost* /pyneva 
RUN echo 'export LD_LIBRARY_PATH=/usr/local/lib'|cat /usr/local/bin/start-notebook.sh > /tmp/out && mv /tmp/out /usr/local/bin/start-notebook.sh
RUN chmod a+x /usr/local/bin/start-notebook.sh
CMD ["start-notebook.sh"]
