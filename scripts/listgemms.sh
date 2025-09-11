#!/bin/bash
loops=$1
ls ../gencode/ | grep Parallel | grep -v RA | grep -v RB | grep _$loops.hpp