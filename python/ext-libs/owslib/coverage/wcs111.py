# -*- coding: ISO-8859-15 -*-
# =============================================================================
# Copyright (c) 2015 Luís de Sousa
#
# Authors : 
#          Luís de Sousa <luis.a.de.sousa@gmail.com>
#
# Contact email: luis.a.de.sousa@gmail.com
# =============================================================================

from owslib.coverage import wcs110

class Namespaces_1_1_1():

    def WCS(self, tag):
        return '{http://www.opengis.net/wcs/1.1.1}'+tag

    def WCS_OWS(self, tag):
        return '{http://www.opengis.net/wcs/1.1.1/ows}'+tag

    def OWS(self, tag):
        return '{http://www.opengis.net/ows/1.1}'+tag


class WebCoverageService_1_1_1(wcs110.WebCoverageService_1_1_0):
    """Abstraction for OGC Web Coverage Service (WCS), version 1.1.1
    Implements IWebCoverageService.
    """
    version='1.1.1'
    ns = Namespaces_1_1_1()

