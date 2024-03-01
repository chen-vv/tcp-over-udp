#!/bin/bash

text="I love rice cakes"

for ((i=0; i<1000000; i++)); do
    echo "$text" >> output.txt
done

echo "Text file generated with 100,000 lines."

