from setuptools import setup, Extension
import pybind11
import sys
import os

ext_modules = [
    Extension(
        name="ReplayMemory",
        sources=["ReplayMemory.cpp"],
        include_dirs=[
            pybind11.get_include(),
        ],
        language="c++",
        extra_compile_args=["-O3", "-std=c++17","-Wall","-fPIC","-shared"],
    ),
]

ext_modules2 = [
    Extension(
        name="Rocket",
        sources=["rocket.cpp"],
        include_dirs=[
            pybind11.get_include(),
        ],
        language="c++",
        extra_compile_args=["-O3", "-std=c++17","-Wall","-fPIC","-shared"],
    ),
]

setup(
    name="ReplayMemory",
    version="0.0.1",
    ext_modules=ext_modules,
)


setup(
    name="Rocket",
    version="0.0.0",
    ext_modules=ext_modules2,
)



os.system("rm -r build")
