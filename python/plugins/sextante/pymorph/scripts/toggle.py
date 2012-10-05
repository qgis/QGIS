# -*- coding: utf-8 -*-

"""
***************************************************************************
    toggle.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

##Additional_image=raster
##Additional_image_2=raster
from sextante.pymorph.mmorph import toggle
img2 = gdal.Open(Additional_image)
input_array2 = img2.ReadAsArray()
img3 = gdal.Open(Additional_image_2)
input_array3 = img2.ReadAsArray()

output_array = toggle(input_array, input_array2, input_array3)

