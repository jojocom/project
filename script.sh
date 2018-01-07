#!/bin/bash

# time
echo "dynamic small dates" > dates
date >> dates
./ngrams -i small/small_dynamic/small_dynamic.init -q small/small_dynamic/small_dynamic.work > dynamic_small
date >> dates

echo >> dates

echo "static small dates" >> dates
date >> dates
./ngrams -i small/small_static/small_static.init -q small/small_static/small_static.work > static_small
date >> dates

echo >> dates

echo "dynamic medium dates" >> dates
date >> dates
./ngrams -i medium/medium_dynamic/medium_dynamic.init -q medium/medium_dynamic/medium_dynamic.work > dynamic_medium
date >> dates

echo >> dates

echo "static medium dates" >> dates
date >> dates
./ngrams -i medium/medium_static/medium_static.init -q medium/medium_static/medium_static.work > static_medium
date >> dates

# valgrind
valgrind --log-file="dynamic_small_valgrind" ./ngrams -i small/small_dynamic/small_dynamic.init -q small/small_dynamic/small_dynamic.work > dynamic_small

valgrind --log-file="static_small_valgrind" ./ngrams -i small/small_static/small_static.init -q small/small_static/small_static.work > static_small

valgrind --log-file="dynamic_medium_valgrind" ./ngrams -i medium/medium_dynamic/medium_dynamic.init -q medium/medium_dynamic/medium_dynamic.work > dynamic_medium

valgrind --log-file="static_medium_valgrind" ./ngrams -i medium/medium_static/medium_static.init -q medium/medium_static/medium_static.work > static_medium
