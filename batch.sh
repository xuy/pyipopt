#!/bin/bash
# To run the code, compile it with Python include file and ipopt library
gcc -g -I/usr/local/include/python2.5 -I/Project/ThirdParty/ipopt/Ipopt-3.3.3/include/ipopt/ -I/usr/lib/python2.5/site-packages/numpy/core/include/ -fpic -shared -lipopt -lg2c -lm -o pyipopt.so pyipopt.c callback.c 
# copy it to the python dir 
# cp ./ipopt.so /usr/lib/python2.5/ 

python amplipopt.py test.nl
python example.py

