#!/bin/bash

#
# A simple script to get rid of .orig and .rej files left in 
# your repository by svn. It will request confirmation before 
# deleting each file.
#
# Tim Sutton, May 2008
find . -name "*.orig" -exec rm -rf {} \;
