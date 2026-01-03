#!/bin/bash



version=-std=c++17

name=ReplayMemory.so

pybind11=$(python3 -m pybind11 --includes)


#g++ $flags -O3 -Wall -fPIC -shared $pybind11 ReplayMemory.cpp -o $name

name=Rocket.so

g++ $flags -O3 -Wall -fPIC -shared $pybind11 rocket.cpp -o $name

# python3 main.py


