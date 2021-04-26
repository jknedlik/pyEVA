# -*- coding: utf-8 -*-

from __future__ import print_function

import sys

try:
    from skbuild import setup
except ImportError:
    print(
        "Please update pip, you need pip 10 or greater,\n"
        " or you need to install the PEP 518 requirements in pyproject.toml yourself",
        file=sys.stderr,
    )
    raise

from setuptools import find_packages


setup(
    name="teneva",
    version="0.0.1",
    description="Tool enabled evolutionary algorithms",
    author="Jan Knedlik",
    license="MIT",
    packages=find_packages(where = 'pysrc'),
    package_dir={"": "pysrc"},
    cmake_install_dir="pysrc/scikit_build_example",
    include_package_data = True,
)
