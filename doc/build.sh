#!/bin/bash
txt2tags -o ../INSTALL -t txt INSTALL.t2t 
txt2tags -o INSTALL.html -t html INSTALL.t2t
txt2tags -o INSTALL.tex -t tex INSTALL.t2t
pdflatex INSTALL.tex
mv INSTALL.pdf ..
txt2tags -o ../CODING -t txt CODING.t2t
txt2tags -o CODING.html -t html CODING.t2t
txt2tags -o CODING.tex -t tex CODING.t2t
pdflatex CODING.tex
mv CODING.pdf ..
