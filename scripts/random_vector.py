#!/usr/bin/env python

# Generates random shapefile which may be used for benchmarks

import os
import sys
import random
import string
import math
from osgeo import ogr
from optparse import OptionParser

def error ( msg ):
  print msg
  sys.exit( 1 )

parser = OptionParser("usage: %prog [options] output")
parser.add_option("-t", "--type", dest="type", type="choice", choices=("point", "line", "polygon"), default="point", help="Geometry type")
parser.add_option("-f", "--features", dest="features", type="int", default=1000, help="Number of features")
parser.add_option("-c", "--coordinates", dest="coordinates", type="int", default=10, help="Number of coordinates per feature (lines and polygons)")
parser.add_option("-a", "--attributes", dest="attributes", type="int", default=10, help="Number of attributes")
parser.add_option("-e", "--extent", dest="extent", type="string", default="-180,-90,180,90", help="Extent")

(options, args) = parser.parse_args()
if len(args) != 1:
  error( "Output file path missing" )

(minx, miny, maxx, maxy) = map ( float, options.extent.split(",") )

driverName = "ESRI Shapefile"
drv = ogr.GetDriverByName( driverName )
if drv is None:
    error ( "%s driver not available.\n" % driverName )

# delete if exists
try:
  if os.path.exists( args[0] ):
    drv.DeleteDataSource( args[0] )
except:
  pass

ds = drv.CreateDataSource( args[0] )
if ds is None:
    error( "Creation of output file failed.\n" )

types = { "point": ogr.wkbPoint, "line": ogr.wkbLineString, "polygon": ogr.wkbPolygon }

lyr = ds.CreateLayer( "out", None, types[options.type] )
if lyr is None:
    error ( "Layer creation failed.\n" )

attrTypes = ( ogr.OFTString, ogr.OFTInteger, ogr.OFTReal )
stringWidth = 100
for a in range(0,options.attributes):
  attrName = "attr%s" % a
  field_defn = ogr.FieldDefn( attrName, random.choice( attrTypes ) )
  if field_defn.type == ogr.OFTString:
    field_defn.SetWidth( stringWidth )

  if lyr.CreateField ( field_defn ) != 0:
      error ( "Creating Name field failed.\n" )

feat_defn = lyr.GetLayerDefn()
for f in range(options.features):
  feat = ogr.Feature( feat_defn )

  buffer = (maxx-minx)/100
  if options.type == "point":
    geo = ogr.Geometry( ogr.wkbPoint )
    x = random.uniform( minx, maxx )
    y = random.uniform( miny, maxy )
    geo.SetPoint_2D(0, x, y)

  elif options.type == "line":
    geo = ogr.Geometry(ogr.wkbLineString)
    xc = random.uniform( minx+buffer, maxx-buffer )
    yc = random.uniform( miny+buffer, maxy-buffer )
    for c in range(options.coordinates):
      a = c * 2 * math.pi / options.coordinates
      r = random.uniform( buffer/10, 9*buffer/10 )
      x = xc + r * math.sin(a)
      y = yc + r * math.cos(a)
      geo.SetPoint_2D(c, x, y)

  elif options.type == "polygon":
    ring = ogr.Geometry(ogr.wkbLinearRing)
    xc = random.uniform( minx+buffer, maxx-buffer )
    yc = random.uniform( miny+buffer, maxy-buffer )
    for c in range(options.coordinates):
      a = c * 2 * math.pi / options.coordinates
      r = random.uniform( buffer/10, 9*buffer/10 )
      x = xc + r * math.sin(a)
      y = yc + r * math.cos(a)
      ring.SetPoint_2D(c, x, y)
    geo = ogr.Geometry(ogr.wkbPolygon)
    geo.AddGeometry ( ring )

  feat.SetGeometry(geo)

  for i in range(feat_defn.GetFieldCount()):
    field_defn = feat_defn.GetFieldDefn(i)
    val = None
    limit = 10000000
    if field_defn.GetType() == ogr.OFTString:
      nChars = random.randint(0,stringWidth)
      val = ''.join(random.choice(string.ascii_letters+ string.digits) for x in range(nChars) )
    elif field_defn.GetType() == ogr.OFTInteger:
      val = random.randint( -limit, limit )
    elif field_defn.GetType() == ogr.OFTReal:
      val = random.uniform ( -limit, limit )
    feat.SetField( field_defn.name, val )

  if lyr.CreateFeature(feat) != 0:
      error ( "Failed to create feature in shapefile.\n" )
