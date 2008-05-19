#!/bin/bash

#
# A simple script to get rid of .orig and .rej files left in 
# your repository by svn. It will request confirmation before 
# deleting each file.
#
# Tim Sutton, May 2008

for FILE in `find . -name *.orig`; do echo "Deleting reject:  $FILE"; rm -i $FILE; done
for FILE in `find . -name *.rej`; do echo "Deleting reject:  $FILE"; rm -i $FILE; done
for FILE in `find . -name *.tmp | grep -v "\.svn"`; do echo "Deleting reject:  $FILE"; rm -i $FILE; done
