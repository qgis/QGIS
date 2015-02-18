#!/usr/bin/env python

# Generates (or updates) a unit test image mask, which is used to specify whether
# a pixel in the control image should be checked (black pixel in mask) or not (white
# pixel in mask). For non black or white pixels, the pixels lightness is used to
# specify a maximum delta for each color component 

import os
import sys
import argparse
from PyQt4.QtCore import *
from PyQt4.QtGui import *
import struct

def error ( msg ):
  print msg
  sys.exit( 1 )
  
def colorDiff( c1, c2 ):
  redDiff = abs( qRed( c1 ) - qRed( c2 ) )
  greenDiff = abs( qGreen( c1 ) - qGreen( c2 ) )
  blueDiff = abs( qBlue( c1 ) - qBlue( c2 ) )
  alphaDiff = abs( qAlpha( c1 ) - qAlpha( c2 ) )
  return max( redDiff, greenDiff, blueDiff, alphaDiff )

def updateMask(control_image_path, rendered_image_path, mask_image_path):
  control_image = QImage( control_image_path )
  if not control_image:
    error('Could not read control image {}'.format(control_image_path))

  rendered_image = QImage( rendered_image_path )
  if not rendered_image:
    error('Could not read rendered image {}'.format(rendered_image_path))
  if not rendered_image.width() == control_image.width() or not rendered_image.height() == control_image.height():
    error('Size mismatch - control image is {}x{}, rendered image is {}x{}'.format(control_image.width(),
                                                                                   control_image.height(),
                                                                                   rendered_image.width(),
                                                                                   rendered_image.height()))
    
  #read current mask, if it exist
  mask_image = QImage( mask_image_path )
  if mask_image.isNull():
    print 'Mask image does not exist, creating'
    mask_image = QImage( control_image.width(), control_image.height(), QImage.Format_ARGB32 )
    mask_image.fill( QColor(0,0,0) )

  #loop through pixels in rendered image and compare
  mismatch_count = 0
  width = control_image.width()
  height = control_image.height()
  linebytes = width * 4
  for y in xrange( height ):
    control_scanline = control_image.constScanLine( y ).asstring(linebytes)
    rendered_scanline = rendered_image.constScanLine( y ).asstring(linebytes)
    mask_scanline = mask_image.scanLine( y ).asstring(linebytes)

    for x in xrange( width ):
      currentTolerance = qRed( struct.unpack('I', mask_scanline[ x*4:x*4+4 ] )[0] )

      if currentTolerance == 255:
        #ignore pixel
        continue

      expected_rgb = struct.unpack('I', control_scanline[ x*4:x*4+4 ] )[0]
      rendered_rgb = struct.unpack('I', rendered_scanline[ x*4:x*4+4 ] )[0]
      difference = colorDiff( expected_rgb, rendered_rgb )

      if difference > currentTolerance:
        #update mask image
        mask_image.setPixel( x, y, qRgb( difference, difference, difference ) )
        mismatch_count += 1
      
  if mismatch_count:
    #update mask
    mask_image.save( mask_image_path, "png" );
    print 'Updated {} pixels'.format( mismatch_count )

parser = argparse.ArgumentParser() #OptionParser("usage: %prog control_image rendered_image mask_image")
parser.add_argument('control_image')
parser.add_argument('rendered_image')
parser.add_argument('mask_image')
args = parser.parse_args()


updateMask(args.control_image, args.rendered_image, args.mask_image)

