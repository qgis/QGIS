#!/bin/bash
txt2tags -o ../INSTALL -t txt INSTALL.t2t 
txt2tags -o index.html -t html INSTALL.t2t
txt2tags -o ../CODING -t txt CODING.t2t
txt2tags -o CODING.html -t html CODING.t2t
