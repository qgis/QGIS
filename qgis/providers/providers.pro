######################################################################
# Qmake project file for QGIS providers directory
# This file is used by qmake to generate the Makefile for building
# QGIS on Windows
#
# $Id $ 
######################################################################
TEMPLATE = subdirs
SUBDIRS =  delimitedtext \
           ogr \
           postgres
