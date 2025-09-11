#!/bin/bash

for pa in N W; do
    for pb in N W; do
        for pc in N R; do
            ./gengemm $pa $pb $pc $1 $2 $3 $4 $5 $6
        done
    done
done
