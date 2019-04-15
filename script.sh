#!/bin/bash
mkdir Source
mkdir Target
mkdir Source/A
touch Source/A/a.txt
touch Source/b.txt
echo "Szedł Sasza suchą szosą suszył sobie spodnie" >> Source/b.txt
dd if=/dev/zero of=Source/c.txt count=1024 bs=1024
mkdir Target/B
touch Target/B/a.txt
echo "Król Karol kupił królowej Karolinie korale koloru koralowego." >> Target/B/a.txt
touch Target/b.txt
make
./daemon ./Source ./Target -R -s 1024
