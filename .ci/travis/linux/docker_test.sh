#!/usr/bin/env bash
###########################################################################
#    docker_test.sh
#
#    Run a particular test on docker testing env and return its exit code
#
#    Arguments:
#
#        $1:  test name in dotted notation
#
#    ---------------------
#    Date                 : November 2018
#    Copyright            : (C) 2018 by Alessandro Pasotti
#    Email                : elpaso at itopen dot it
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################

TEST_NAME=$1

docker exec -it qgis-testing-environment sh -c "cd /tests_directory && qgis_testrunner.sh ${TEST_NAME}" &>/dev/null

echo $?
