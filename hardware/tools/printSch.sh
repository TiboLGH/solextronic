#!/bin/bash
convert -rotate 90 -flatten $1.ps $1.png
ps2pdf $1.ps $1.pdf
