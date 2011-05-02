#!/bin/bash

#
# A simple script to get rid of files that are not 
# managed by svn. It will request confirmation before 
# deleting each file.
#
# Tim Sutton, May 2008

for FILE in `svn status |grep ^? | awk '{print $2}'`;do rm -i -r $FILE; done
