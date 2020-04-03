#! /bin/bash

# Map data
if [ ! -d "data" ]; then
    mkdir data
fi
if [ ! -f "data/map_data.txt" ]; then
    wget -P data/ https://raw.githubusercontent.com/udacity/CarND-Kidnapped-Vehicle-Project/master/data/map_data.txt
fi

if [ ! -d "include" ]; then
    mkdir include
fi

# JSON
if [ ! -f "include/json.hpp" ]; then
    wget -P include/ https://github.com/nlohmann/json/releases/download/v3.7.3/json.hpp
fi
