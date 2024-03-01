#!/bin/bash

text="I love rice cakes"

for ((i=0; i<10000; i++)); do
    echo "$text" >> output.txt
done

echo "Done"

