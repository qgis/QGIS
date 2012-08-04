# --------------------------------------------------------
#    mmqgisx_library - mmqgisx operation functions
#
#    begin                : 10 May 2010
#    copyright            : (c) 2010 by Michael Minn
#    email                : See michaelminn.com
#
#   MMQGIS is free software and is offered without guarantee
#   or warranty. You can redistribute it and/or modify it 
#   under the terms of version 2 of the GNU General Public 
#   License (GPL v2) as published by the Free Software 
#   Foundation (www.gnu.org).
# --------------------------------------------------------

import csv
import sys
import time
import urllib
import os.path
import operator
import tempfile

from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from math import *

# --------------------------------------------------------
#    MMQGIS Utility Functions
# --------------------------------------------------------

def mmqgisx_find_layer(layer_name):
	# print "find_layer(" + str(layer_name) + ")"

	for name, search_layer in QgsMapLayerRegistry.instance().mapLayers().iteritems():
		if search_layer.name() == layer_name:
			return search_layer

	return None

def mmqgisx_is_float(s):
	try:
		float(s)
		return True
	except:
		return False

# Cumbersome function to give backward compatibility before python 2.7

def format_float(value, separator, decimals):
	formatstring = ("%0." + unicode(int(decimals)) + "f")
	# print str(value) + ": " + formatstring
	string = formatstring % value
	intend = string.find('.')
	if intend < 0:
		intend = len(string)

	if separator and (intend > 3):
		start = intend % 3
		if start == 0:
			start = 3
		intstring = string[0:start]

		for x in range(start, intend, 3):
			intstring = intstring + separator + string[x:x+3]

		string = intstring + string[intend:]

	return string

def mmqgisx_gridify_points(hspacing, vspacing, points):
	# Align points to grid
	point_count = 0
	deleted_points = 0
	newpoints = []
	for point in points:
		point_count += 1
		newpoints.append(QgsPoint(round(point.x() / hspacing, 0) * hspacing, \
				    round(point.y() / vspacing, 0) * vspacing))

	# Delete overlapping points
	z = 0
	while z < (len(newpoints) - 2):
		if newpoints[z] == newpoints[z + 1]:
			newpoints.pop(z + 1)
			deleted_points += 1
		else:
			z += 1

	# Delete line points that go out and return to the same place
	z = 0
	while z < (len(newpoints) - 3):
		if newpoints[z] == newpoints[z + 2]:
			newpoints.pop(z + 1)
			newpoints.pop(z + 1)
			deleted_points += 2
			# Step back to catch arcs
			if (z > 0):
				z -= 1
		else:
			z += 1

	# Delete overlapping start/end points
	while (len(newpoints) > 1) and (newpoints[0] == newpoints[len(newpoints) - 1]):
		newpoints.pop(len(newpoints) - 1)
		deleted_points += 2
				
	return newpoints, point_count, deleted_points

# mmqgisx_layer_attribute_bounds() is needed because the 
# QgsVectorDataProvider::minimumValue() and maximumValue() 
# do not work as of QGIS v1.5.0

def mmqgisx_layer_attribute_bounds(layer, attribute_name):
	#attribute_index = -1
	#for index, field in layer.dataProvider().fields().iteritems():	
	#	if str(field.name()) == attribute_name:
	#		attribute_index = index

	attribute_index = layer.dataProvider().fieldNameIndex(attribute_name)
	if attribute_index == -1:
		return 0, 0, 0

	# print attribute_index

	feature = QgsFeature()
	layer.dataProvider().select(layer.dataProvider().attributeIndexes())
	layer.dataProvider().rewind()

	count = 0
	minimum = 0
	maximum = 0
	while layer.dataProvider().nextFeature(feature):
		# print str(feature.attributeMap())
		# value = float(feature.attributeMap()[attribute_index])
		value, valid = feature.attributeMap()[attribute_index].toDouble()
		if (count == 0) or (minimum > value):
			minimum = value
		if (count == 0) or (maximum < value):
			maximum = value
		# print str(value) + " : " + str(valid) + " : " + str(minimum) + " : " + str(maximum)
		count += 1

	return minimum, maximum, 1 

def mmqgisx_wkbtype_to_text(wkbtype):
	if wkbtype == QGis.WKBUnknown: return "Unknown"
	if wkbtype == QGis.WKBPoint: return "point"
	if wkbtype == QGis.WKBLineString: return "linestring"
	if wkbtype == QGis.WKBPolygon: return "polygon"
	if wkbtype == QGis.WKBMultiPoint: return "multipoint"
	if wkbtype == QGis.WKBMultiLineString: return "multilinestring"
	if wkbtype == QGis.WKBMultiPolygon: return "multipolygon"
	# if wkbtype == QGis.WKBNoGeometry: return "no geometry"
	if wkbtype == QGis.WKBPoint25D: return "point 2.5d"
	if wkbtype == QGis.WKBLineString25D: return "linestring 2.5D"
	if wkbtype == QGis.WKBPolygon25D: return "multipolygon 2.5D"
	if wkbtype == QGis.WKBMultiPoint25D: return "multipoint 2.5D"
	if wkbtype == QGis.WKBMultiLineString25D: return "multilinestring 2.5D"
	if wkbtype == QGis.WKBMultiPolygon25D: return "multipolygon 2.5D"
	return "Unknown WKB " + unicode(wkbtype)

# --------------------------------------------------------
#    mmqgisx_animate_columns - Create animations by
#		interpolating offsets from attributes
# --------------------------------------------------------

def mmqgisx_animate_columns(qgis, layer_name, long_col, lat_col, outdir, frame_count):

	# Error Checks
	layer = mmqgisx_find_layer(layer_name)
	if layer == None:
		return "Invalid map layer ID: " + unicode(map_layer_id)

	long_col_index = layer.dataProvider().fieldNameIndex(long_col)
	if (long_col_index < 0):
		return "Invalid longitude column index: " + unicode(long_col)

	lat_col_index = layer.dataProvider().fieldNameIndex(lat_col)
	if (lat_col_index < 0):
		return "Invalid latitude column: " + unicode(lat_col)

	if not os.path.isdir(outdir):
		return "Invalid output directory: " + unicode(outdir)

	if frame_count <= 0:
		return "Invalid number of frames specified: " + unicode(frame_count)


	# Initialize temporary shapefile

	tempdir = tempfile.mkdtemp()
	tempfilename = tempdir + "/mmqgisx_animate.shp"
	tempcrs = layer.dataProvider().crs()
	if not tempcrs.isValid():
		tempcrs.createFromSrid(4326)
		print "Defaulting layer " + unicode(layer.id()) + " to CRS " + unicode(tempcrs.epsg())

	tempwriter = QgsVectorFileWriter(QString(tempfilename), QString("System"), \
		layer.dataProvider().fields(), layer.dataProvider().geometryType(), tempcrs)
	del tempwriter

	templayer = qgis.addVectorLayer(tempfilename, "animate", "ogr")
	templayer.setRenderer(layer.renderer().clone())
	templayer.enableLabels(layer.hasLabelsEnabled())

	qgis.legendInterface().setLayerVisible(layer, 0)


	# Iterate Frames

	for frame in range(frame_count + 1):
		qgis.mainWindow().statusBar().showMessage("Rendering frame " + unicode(frame))

		# Read, move and rewrite features

		feature = QgsFeature()
		feature_ids = []
		layer.dataProvider().select(layer.dataProvider().attributeIndexes())
		layer.dataProvider().rewind()
		while layer.dataProvider().nextFeature(feature):
			attributes = feature.attributeMap()
			xoffset, valid = attributes[long_col_index].toDouble()
			yoffset, valid = attributes[lat_col_index].toDouble()
			xoffset = xoffset * frame / frame_count;
			yoffset = yoffset * frame / frame_count;

			newfeature = QgsFeature()
			newgeometry = feature.geometry()
			newgeometry.translate(xoffset, yoffset)
			newfeature.setGeometry(newgeometry)
			newfeature.setAttributeMap(attributes)

			if templayer.dataProvider().addFeatures([newfeature]):
				feature_ids.append(newfeature.id())

		# Write Frame

		# templayer.commitChanges()
		qgis.mapCanvas().refresh()
		framefile = outdir + "/frame" + format(frame, "06d") + ".png"
		qgis.mapCanvas().saveAsImage(QString(framefile))

		# Delete features from temporary shapefile

		for feature_id in feature_ids:
			templayer.dataProvider().deleteFeatures([feature_id])

	# Clean up

	QgsMapLayerRegistry.instance().removeMapLayer(templayer.id())
	QgsVectorFileWriter.deleteShapeFile(QString(tempfilename))
	qgis.legendInterface().setLayerVisible(layer, 1)

	return None

# --------------------------------------------------------
#    mmqgisx_animate_rows - Create animations by
#		displaying successive rows
# --------------------------------------------------------

def mmqgisx_animate_rows(qgis, layer_names, cumulative, outdir):

	#print "mmqgisx_animate_rows()"
	#for id in animate_layer_ids:
	#	print str(id)
	#print "cumulative = " + str(cumulative)
	#print outdir

	# Error Checks
	if not os.path.isdir(outdir):
		return "Invalid output directory: " + unicode(outdir)

	layers = []
	for layer_name in layer_names:
		layer = mmqgisx_find_layer(layer_name)
		if layer == None:
			return "Invalid layer name: " + unicode(layer_name)
		layers.append(layer)

	frame_count = 0
	for layer in layers:
		if frame_count < layer.dataProvider().featureCount():
			frame_count = layer.dataProvider().featureCount()

	if frame_count <= 0:
		return "Invalid number of frames specified"


	# Feature ID arrays

	feature_ids = [None] * len(layers)
	for index in range(len(layers)):
		layer = layers[index]
		feature = QgsFeature()
		feature_ids[index] = []
		layer.dataProvider().select()
		layer.dataProvider().rewind()
		while layer.dataProvider().nextFeature(feature):
			feature_ids[index].append(feature.id());
			# print feature.id()

	# Create temporary layers

	tempdir = tempfile.mkdtemp()
	tempnames = [None] * len(layers)
	templayers = [None] * len(layers)
	for layer_index in range(len(layers)):
		tempnames[layer_index] = tempdir + "/mmqgisx_animate" + unicode(layer_index) + ".shp"
		tempcrs = layers[layer_index].dataProvider().crs()
		if not tempcrs.isValid():
			tempcrs.createFromSrid(4326)
			print "Defaulting layer " + unicode(animate_layer_ids[layer_index]) + \
				" to CRS " + unicode(tempcrs.epsg())

		tempwriter = QgsVectorFileWriter(QString(tempnames[layer_index]), QString("System"), \
			layers[layer_index].dataProvider().fields(), \
			layers[layer_index].dataProvider().geometryType(), tempcrs)
		del tempwriter

		templayers[layer_index] = qgis.addVectorLayer(tempnames[layer_index], "animate" + unicode(layer_index), "ogr")
		templayers[layer_index].setRenderer(layers[layer_index].renderer().clone())

		# There doesn't seem to be a way to directly copy labeling fields and attributes
		if layers[layer_index].hasLabelsEnabled():
			templayers[layer_index].enableLabels(1)
			label = layers[layer_index].label()
			templabel = templayers[layer_index].label()
			templabel.setFields(label.fields())

			for field_index, field in templabel.fields().iteritems():
				if field.name() == label.labelField(0):
					templabel.setLabelField(0, field_index)

			attributes = label.labelAttributes()
			tempattributes = templabel.labelAttributes()
			tempattributes.setFamily(attributes.family())
			tempattributes.setBold(attributes.bold())
			tempattributes.setItalic(attributes.italic())
			tempattributes.setUnderline(attributes.underline())
			tempattributes.setSize(attributes.size(), attributes.sizeType())
			tempattributes.setColor(attributes.color())

		qgis.legendInterface().setLayerVisible(layers[layer_index], 0)

	# return "Testing"

	# Iterate frames

	for frame in range(int(frame_count + 1)):
		qgis.mainWindow().statusBar().showMessage("Rendering frame " + unicode(frame))

		for layer_index in range(len(layers)):
			if frame < layers[layer_index].featureCount():
				feature = QgsFeature()
				featureid = feature_ids[layer_index][frame]
				layers[layer_index].dataProvider().featureAtId(featureid, feature, 1, \
					layers[layer_index].dataProvider().attributeIndexes())

				newfeature = QgsFeature()
				newfeature.setGeometry(feature.geometry())
				newfeature.setAttributeMap(feature.attributeMap())
				if templayers[layer_index].dataProvider().addFeatures([newfeature]):
					feature_ids[layer_index][frame] = newfeature.id()
			#templayers[layer_index].commitChanges()

		qgis.mapCanvas().refresh()
		framefile = outdir + "/frame" + format(frame, "06d") + ".png"
		qgis.mapCanvas().saveAsImage(QString(framefile))

		if not cumulative:
			for layer_index in range(len(layers)):
				if frame < layers[layer_index].featureCount():
					feature = QgsFeature()
					featureid = feature_ids[layer_index][frame]
					templayers[layer_index].dataProvider().deleteFeatures([featureid])

	for layer_index in range(len(layers)):
		QgsMapLayerRegistry.instance().removeMapLayer(templayers[layer_index].id())
		QgsVectorFileWriter.deleteShapeFile(QString(tempnames[layer_index]))
		qgis.legendInterface().setLayerVisible(layers[layer_index], 1)

	return None

# ----------------------------------------------------------
#    mmqgisx_attribute_export - Export attributes to CSV file
# ----------------------------------------------------------

def mmqgisx_attribute_export(qgis, outfilename, layername, attribute_names, field_delimiter, line_terminator):
	# Error checks

	if (not outfilename) or (len(outfilename) <= 0):
		return "No output CSV file given"
	
	layer = mmqgisx_find_layer(layername)
	if layer == None:
		return "Layer " + layername + " not found"

	# Find attribute indices
	attribute_indices = []
	if (not attribute_names) or (len(attribute_names) <= 0):
		attribute_names = []
		# print "fields: " + str(layer.dataProvider().fields())
		for index, field in layer.dataProvider().fields().iteritems():
			attribute_indices.append(index)
			attribute_names.append(field.name())

	else:
		for x in range(0, len(attribute_names)):
			index = layer.dataProvider().fieldNameIndex(attribute_names[x])
			if index < 0:
				return "Layer " + layername + " has no attribute " + attribute_names[x]
			attribute_indices.append(index)

	# Create the CSV file
	try:
		outfile = open(outfilename, 'w')
    	except:
		return "Failure opening " + outfilename

	writer = csv.writer(outfile, delimiter = field_delimiter, lineterminator = line_terminator)
	writer.writerow(attribute_names) # header


	# Iterate through each feature in the source layer
	feature_count = layer.dataProvider().featureCount()

	feature = QgsFeature()
	layer.dataProvider().select(layer.dataProvider().attributeIndexes())
	layer.dataProvider().rewind()
        while layer.dataProvider().nextFeature(feature):
		qgis.mainWindow().statusBar().showMessage \
			("Exporting feature " + unicode(feature.id()) + " of " + unicode(feature_count))
		attributes = feature.attributeMap()

		row = []
		for column in attribute_indices:
			row.append(unicode(attributes[column].toString()).encode("iso-8859-1"))

		# print unicode(feature.id()).encode("iso-8859-1") + " = " + unicode(row[0]).encode("iso-8859-1")

		writer.writerow(row)

	del writer

	qgis.mainWindow().statusBar().showMessage(unicode(feature_count) + " records exported")

	return None

# --------------------------------------------------------
#    mmqgisx_attribute_join - Join attributes from a CSV
#                            file to a shapefile
# --------------------------------------------------------

def mmqgisx_attribute_join(qgis, layername, infilename, joinfield, joinattribute, outfilename, notfoundname, addlayer):
	layer = mmqgisx_find_layer(layername)
	if layer == None:
		return "Layer " + unicode(layername) + " not found"

	joinattribute_index = layer.fieldNameIndex(joinattribute)
	if joinattribute_index < 0:
		return "Invalid join attribute " + unicode(joinattribute)

	if len(infilename) <= 0:
		return "No input CSV file given"

	if len(outfilename) <= 0:
		return "No output shapefile name given"
		
	# Create a combined field list from the source layer and the CSV file header
	try:
		infile = open(infilename, 'r')
	except:
		return "Failure opening input file: " + unicode(infilename)
			
	try:
		dialect = csv.Sniffer().sniff(infile.read(2048))
	except:
		return "Bad CSV file (verify that your delimiters are consistent)" + unicode(infilename)

	infile.seek(0)
	reader = csv.reader(infile, dialect)

	# Build composite list of fields
	joinfield_index = -1
	joinfield = joinfield[0:9].strip().lower()
	newfields = layer.dataProvider().fields()
	header = reader.next()
	for index in range(0, len(header)):
		fieldname = header[index]
		fieldname = fieldname[0:9].strip().lower()
		if fieldname == joinfield:
			joinfield_index = index
		newfields[len(newfields)] = QgsField(fieldname, QVariant.String)
		#print str(field) + ": " + str(newfields[len(newfields) - 1])

	# Attempt to prevent duplicate column names
	#print "new = " + unicode(len(newfields))

	for x in range(0, len(newfields)):
		#print "x = " + unicode(newfields[x].name())
		for y in range(x + 1, len(newfields)):
			#print "y = " + unicode(newfields[y].name())
			if unicode(newfields[x].name()).lower() == unicode(newfields[y].name()).lower():
				duplication = unicode(newfields[x].name())
				if duplication[0] != '_':
					newfields[y].setName('_' + duplication[1:len(duplication)])
				elif duplication[0] != '-':
					newfields[y].setName('-' + duplication[1:len(duplication)])
				else:
					newfields[y].setName('.' + duplication[1:len(duplication)])

	if joinfield_index < 0:
		return "Join field " + unicode(joinfield) + " not found in " + unicode(infilename)

	# Create the output shapefile
	if QFile(outfilename).exists():
		if not QgsVectorFileWriter.deleteShapeFile(QString(outfilename)):
			return "Failure deleting existing shapefile: " + unicode(outfilename)

	#print newfields

	outfile = QgsVectorFileWriter(QString(outfilename), QString("System"), \
			newfields, layer.dataProvider().geometryType(), layer.dataProvider().crs())

	if (outfile.hasError() != QgsVectorFileWriter.NoError):
		return "Failure creating output shapefile: " + unicode(outfile.errorMessage())

	# Read the CSV file data into memory
	csv_data = []
	csv_found = []
	for row in reader:
		csv_data.append(row)
		csv_found.append(0)

	for x in range(0, len(csv_data)):
		# print str(x) + ":" + str(joinfield_index) + ":" + str(len(csv_data[x]))
		csv_data[x][joinfield_index] = csv_data[x][joinfield_index].strip().lower()

	del reader


	# Iterate through each feature in the source layer
	matched_count = 0
	feature_count = layer.dataProvider().featureCount()

	#feature_id = 1
	#while layer.featureAtId(feature_id, feature):
	#	feature_id += 1
	feature = QgsFeature()
	layer.dataProvider().select(layer.dataProvider().attributeIndexes())
	layer.dataProvider().rewind()
	while layer.dataProvider().nextFeature(feature):
		qgis.mainWindow().statusBar().showMessage("Joining feature " + unicode(feature.id()) + \
				" of " + unicode(feature_count) + " (" + unicode(matched_count) + " matched)")
		attributes = feature.attributeMap()
		key = unicode(attributes[joinattribute_index].toString()).encode("iso-8859-1").lower()

		row_index = 0
		for row in csv_data:
			# print "'" + str(row[joinfield_index]) + "' == '" + key + "'"
			if row[joinfield_index] == key:
				newattributes = {}
				for name, value in attributes.iteritems():
					newattributes[len(newattributes)] = value
					#print str(value)
					
				for combine in row:
					newattributes[len(newattributes)] = QVariant(combine)
					#print str(combine)
					#print newattributes[len(newattributes) - 1].toString()

				# print key + " --------------"
				# for attribute in newattributes.values():
				# 	print unicode(attribute.toString())

				newfeature = QgsFeature()
				newfeature.setAttributeMap(newattributes)
				newfeature.setGeometry(feature.geometry())
				outfile.addFeature(newfeature)
				matched_count += 1
				csv_found[row_index] += 1
			row_index += 1

	del outfile
	del infile

	# Write records that were not joined to the notfound file
	try:
		outfile = open(notfoundname, 'w')
	except:
		return "Failure opening not found file: " + unicode(notfoundname)

	else:
		writer = csv.writer(outfile, dialect)
		writer.writerow(header)
		for x in range(0, len(csv_data)):
			if not csv_found[x]:
				writer.writerow(csv_data[x])
		del writer
		del outfile
	
	if addlayer:	
		qgis.addVectorLayer(outfilename, os.path.basename(outfilename), "ogr")

	qgis.mainWindow().statusBar().showMessage(unicode(matched_count) + " records joined from " + \
		unicode(feature_count) + " shape records and " + unicode(len(csv_data)) + " CSV file records")

	return None


# --------------------------------------------------------
#    mmqgisx_color_map - Robust layer coloring
# --------------------------------------------------------

def mmqgisx_set_color_map(qgis, layername, bandname, lowvalue, midvalue, highvalue, steps, lowcolor, midcolor, highcolor):
	layer = mmqgisx_find_layer(layername)
	if layer == None:
		return "Invalid layer name: " + layername

	# temp_filename = "/tmp/mmqgisx.qml"
	# outfile = open(temp_filename, 'w')
	# if outfile == None:
	try:
		outfile, temp_filename = tempfile.mkstemp()
	except:
		return "Failure creating temporary style file"

	lowred = (lowcolor >> 16) & 0xff
	lowgreen = (lowcolor >> 8) & 0xff
	lowblue = lowcolor & 0xff
	midred = (midcolor >> 16) & 0xff
	midgreen = (midcolor >> 8) & 0xff
	midblue = midcolor & 0xff
	highred = (highcolor >> 16) & 0xff
	highgreen = (highcolor >> 8) & 0xff
	highblue = highcolor & 0xff

	os.write(outfile, "<!DOCTYPE qgis PUBLIC 'http://mrcc.com/qgis.dtd' 'SYSTEM'>")
	os.write(outfile, "<qgis version=\"1.4.0-Enceladus\" minimumScale=\"1\" ")
	os.write(outfile, "  maximumScale=\"1e+08\" hasScaleBasedVisibilityFlag=\"0\" >")
	os.write(outfile, "<transparencyLevelInt>255</transparencyLevelInt>")

	if layer.type() == QgsMapLayer.RasterLayer:
		# http://www.osgeo.org/pipermail/qgis-developer/2009-January/005742.html
		os.write(outfile, "<rasterproperties>")
		os.write(outfile, "<mDrawingStyle>SingleBandPseudoColor</mDrawingStyle>")
		os.write(outfile, "<mColorShadingAlgorithm>ColorRampShader</mColorShadingAlgorithm>")
		os.write(outfile, "<mInvertColor boolean=\"false\"/>")
		os.write(outfile, "<mRedBandName>Not Set</mRedBandName>")
		os.write(outfile, "<mGreenBandName>Not Set</mGreenBandName>")
		os.write(outfile, "<mBlueBandName>Not Set</mBlueBandName>")
		os.write(outfile, "<mGrayBandName>" + bandname + "</mGrayBandName>")
		os.write(outfile, "<mStandardDeviations>0</mStandardDeviations>")
		os.write(outfile, "<mUserDefinedRGBMinimumMaximum boolean=\"false\"/>")
    		os.write(outfile, "<mRGBMinimumMaximumEstimated boolean=\"true\"/>")
		os.write(outfile, "<mUserDefinedGrayMinimumMaximum boolean=\"false\"/>")
		os.write(outfile, "<mGrayMinimumMaximumEstimated boolean=\"true\" />")
		os.write(outfile, "<mContrastEnhancementAlgorithm>StretchToMinimumMaximum</mContrastEnhancementAlgorithm>")
		os.write(outfile, "<contrastEnhancementMinMaxValues>")
		os.write(outfile, "<minMaxEntry>")
		os.write(outfile, "<min>" + unicode(lowvalue) + "</min>")
		os.write(outfile, "<max>" + unicode(highvalue) + "</max>")
		os.write(outfile, "</minMaxEntry>")
		os.write(outfile, "</contrastEnhancementMinMaxValues>")
		os.write(outfile, "<mNoDataValue mValidNoDataValue=\"true\" >-1.000000</mNoDataValue>")
		os.write(outfile, "<singleValuePixelList>")
		os.write(outfile, "<pixelListEntry pixelValue=\"-1.000000\" percentTransparent=\"100\" />")
		os.write(outfile, "</singleValuePixelList>")
		os.write(outfile, "<threeValuePixelList>")
		os.write(outfile, "<pixelListEntry red=\"-1.000000\" blue=\"-1.000000\" ")
		os.write(outfile, "  green=\"-1.000000\" percentTransparent=\"100\" />")
		os.write(outfile, "</threeValuePixelList>\n")
		os.write(outfile, "<customColorRamp>\n")
		os.write(outfile, "<colorRampType>DISCRETE</colorRampType>\n")

		for x in range(0, steps):
			interpolate = x / float(steps)
			if (interpolate < 0.5):
				interpolate = interpolate * 2.0;
				value = lowvalue + ((midvalue - lowvalue) * interpolate)
				red = unicode(int(round(lowred + ((midred - lowred) * interpolate))))
				green = unicode(int(round(lowgreen + ((midgreen - lowgreen) * interpolate))))
				blue = unicode(int(round(lowblue + ((midblue - lowblue) * interpolate))))
				os.write(outfile, "<colorRampEntry red=\"" + red + "\" blue=\"" + blue + 
					"\" green=\"" + green + "\" value=\"" + unicode(value) + "\" label=\"\"/>\n")
			else:
				interpolate = (interpolate - 0.5) * 2.0
				value = midvalue + ((highvalue - midvalue) * interpolate)
				red = unicode(int(round(midred + ((highred - midred) * interpolate))))
				green = unicode(int(round(midgreen + ((highgreen - midgreen) * interpolate))))
				blue = unicode(int(round(midblue + ((highblue - midblue) * interpolate))))
				os.write(outfile, "<colorRampEntry red=\"" + red + "\" blue=\"" + blue + 
					"\" green=\"" + green + "\" value=\"" + unicode(value) + "\" label=\"\"/>\n")

			#print str(x) + ", " + str(interpolate) + ", " + str(value)

   		os.write(outfile, "</customColorRamp>\n")
		os.write(outfile, "</rasterproperties>\n")

	else: # vector
		os.write(outfile, "<classificationattribute>" + bandname + "</classificationattribute>\n")
		os.write(outfile, "<graduatedsymbol>\n")
		os.write(outfile, "<mode>Quantile</mode>\n")
		os.write(outfile, "<classificationfield>" + bandname + "</classificationfield>\n")

		if (steps < 3):
			steps = 3

		values = []
		for x in range(0, steps + 1):
			interpolate = x / float(steps)
			if (interpolate < 0.5):
				interpolate = interpolate * 2.0
				value = lowvalue + ((midvalue - lowvalue) * interpolate)
			else:
				interpolate = (interpolate - 0.5) * 2.0
				value = midvalue + ((highvalue - midvalue) * interpolate)
			values.append(value)
			# print str(x) + ", " + str(value)
		
		for x in range(0, steps):
			interpolate = x / float(steps)
			if (interpolate < 0.5):
				interpolate = interpolate * 2.0
				red = unicode(int(round(lowred + ((midred - lowred) * interpolate))))
				green = unicode(int(round(lowgreen + ((midgreen - lowgreen) * interpolate))))
				blue = unicode(int(round(lowblue + ((midblue - lowblue) * interpolate))))
			else:
				interpolate = (interpolate - 0.5) * 2.0
				red = unicode(int(round(midred + ((highred - midred) * interpolate))))
				green = unicode(int(round(midgreen + ((highgreen - midgreen) * interpolate))))
				blue = unicode(int(round(midblue + ((highblue - midblue) * interpolate))))

			os.write(outfile, "<symbol>\n")
			os.write(outfile, "<lowervalue>" + unicode(values[x]) + "</lowervalue>\n")
			os.write(outfile, "<uppervalue>" + unicode(values[x + 1]) + "</uppervalue>\n")
			os.write(outfile, "<label></label>\n")
			os.write(outfile, "<pointsymbol>hard:circle</pointsymbol>\n")
			os.write(outfile, "<pointsize>2</pointsize>\n")
			os.write(outfile, "<pointsizeunits>pixels</pointsizeunits>\n")
			os.write(outfile, "<rotationclassificationfieldname></rotationclassificationfieldname>\n")
			os.write(outfile, "<scaleclassificationfieldname></scaleclassificationfieldname>\n")
			os.write(outfile, "<symbolfieldname></symbolfieldname>\n")
			os.write(outfile, "<outlinecolor red=\"128\" blue=\"128\" green=\"128\"/>\n")
			os.write(outfile, "<outlinestyle>SolidLine</outlinestyle>\n")
			os.write(outfile, "<outlinewidth>0.26</outlinewidth>\n")
			os.write(outfile, "<fillcolor red=\"" + unicode(red) + "\" blue=\"" + 
					unicode(blue) + "\" green=\"" + unicode(green) + "\"/>")
			os.write(outfile, "<fillpattern>SolidPattern</fillpattern>\n")
			os.write(outfile, "<texturepath></texturepath>\n")
			os.write(outfile, "</symbol>\n")

		os.write(outfile, "</graduatedsymbol>\n")

	os.write(outfile, "</qgis>\n")
	os.close(outfile)

	load_message = layer.loadNamedStyle(temp_filename)
	# print load_message

	layer.triggerRepaint()
	# qgis.refreshLegend(layer)
	qgis.legendInterface().refreshLayerSymbology(layer)

	return None

# ---------------------------------------------------------
#    mmqgisx_delete_columns - Change text fields to numbers
# ---------------------------------------------------------

def mmqgisx_delete_columns(qgis, layername, columns, savename, addlayer):
	layer = mmqgisx_find_layer(layername)
	if layer == None:
		return "No layer specified to modify: " + layername

	if len(savename) <= 0:
		return "No output filename given"

	if len(columns) <= 0:
		return "No columns specified for deletion"

	# Build dictionary of fields excluding fields flagged for deletion
	srcfields = {}
	destfields = {}
	for index, field in layer.dataProvider().fields().iteritems():
		keep = 1
		for column in columns:
			if field.name() == column:
				keep = 0

		if keep:
			srcfields[index] = field
			destfields[len(destfields)] = field
			#destfields[index] = QgsField (field.name(), field.type(), field.typeName(), \
			#	field.length(), field.precision(), field.comment())

	if len(destfields) <= 0:
		return "All columns being deleted"
 
	# Create the output file
	if QFile(savename).exists():
		if not QgsVectorFileWriter.deleteShapeFile(QString(savename)):
			return "Failure deleting existing shapefile: " + savename

	outfile = QgsVectorFileWriter(QString(savename), QString("System"), destfields,
			layer.dataProvider().geometryType(), layer.dataProvider().crs())

	if (outfile.hasError() != QgsVectorFileWriter.NoError):
		return "Failure creating output shapefile: " + unicode(outfile.errorMessage())


	# Write the features with modified attributes
	feature = QgsFeature()
	featurecount = layer.dataProvider().featureCount();
	layer.dataProvider().select(layer.dataProvider().attributeIndexes())
	layer.dataProvider().rewind()

	while layer.dataProvider().nextFeature(feature):
		qgis.mainWindow().statusBar().showMessage("Writing feature " + \
			unicode(feature.id()) + " of " + unicode(featurecount))

		attributes = {}
		for index, field in srcfields.iteritems():
			attributes[len(attributes)] = feature.attributeMap()[index]

		feature.setAttributeMap(attributes)
		outfile.addFeature(feature)

	del outfile

	if addlayer:
		vlayer = qgis.addVectorLayer(savename, os.path.basename(savename), "ogr")
		
	qgis.mainWindow().statusBar().showMessage(unicode(len(columns)) + " columns deleted and written to " + savename)

	return None

# --------------------------------------------------------
#    mmqgisx_delete_duplicate_geometries - Save to shaperile
#			while removing duplicate shapes
# --------------------------------------------------------

def mmqgisx_delete_duplicate_geometries(qgis, layername, savename, addlayer):

	# Initialization and error checking
	layer = mmqgisx_find_layer(layername)
	if layer == None:
		return "Invalid layer name: " + savename

	if len(savename) <= 0:
		return "No output filename given"

	if QFile(savename).exists():
		if not QgsVectorFileWriter.deleteShapeFile(QString(savename)):
			return "Failure deleting existing shapefile: " + savename

	outfile = QgsVectorFileWriter(QString(savename), QString("System"), layer.dataProvider().fields(),
			layer.dataProvider().geometryType(), layer.dataProvider().crs())

	if (outfile.hasError() != QgsVectorFileWriter.NoError):
		return "Failure creating output shapefile: " + unicode(outfile.errorMessage())

	# Read geometries into an array
	# Have to save as WKT because saving geometries causes segfault 
	# when they are used with equal() later
	geometries = []
	layer.dataProvider().select(layer.dataProvider().attributeIndexes())
	layer.dataProvider().rewind()
	feature = QgsFeature()
	while layer.dataProvider().nextFeature(feature):
		#print "Read geometry " + str(feature.id())
		geometries.append(feature.geometry().exportToWkt())


	# NULL duplicate geometries
	for x in range(0, len(geometries) - 1):
		if geometries[x] != None:
			qgis.mainWindow().statusBar().showMessage("Checking feature " + unicode(x))
			for y in range(x + 1, len(geometries)):
				#print "Comparing " + str(x) + ", " + str(y)
				if geometries[x] == geometries[y]:
					#print "None " + str(x)
					geometries[y] = None

	# Write unique features to output
	writecount = 0
	layer.dataProvider().select(layer.dataProvider().attributeIndexes())
	layer.dataProvider().rewind()
	for x in range(0, len(geometries)):
		# print "Writing " + str(x)
		if layer.dataProvider().nextFeature(feature):
			if geometries[x] != None:
				writecount += 1
				outfile.addFeature(feature)
				
	del outfile

	if addlayer:
		qgis.addVectorLayer(savename, os.path.basename(savename), "ogr")
		
	qgis.mainWindow().statusBar().showMessage(unicode(writecount) + " of " + \
		unicode(layer.dataProvider().featureCount()) + \
		" unique features written to " + savename)

	return None

# ---------------------------------------------------------
#    mmqgisx_float_to_text - String format numeric fields
# ---------------------------------------------------------

def mmqgisx_float_to_text(qgis, layername, attributes, separator, 
			decimals, prefix, suffix, savename, addlayer):

	layer = mmqgisx_find_layer(layername)
	if layer == None:
		return "Project has no active vector layer to convert: " + layername

	if decimals < 0:
		return "Invalid number of decimals: " + unicode(decimals)

	if len(savename) <= 0:
		return "No output filename given"

	# Build dictionary of fields with selected fields for conversion to floating point
	destfields = {};
	changecount = 0
	for index, field in layer.dataProvider().fields().iteritems():
		destfields[index] = QgsField (field.name(), field.type(), field.typeName(), \
			field.length(), field.precision(), field.comment())
 
		if field.name() in attributes:
			if (field.type() != QVariant.Int) and (field.type() != QVariant.Double):
				return "Cannot convert non-numeric field " + unicode(field.name())

			destfields[index].setType(QVariant.String)
			destfields[index].setLength(20)
			changecount += 1

	if (changecount <= 0):
		return "No numeric fields selected for conversion"


	# Create the output file
	if QFile(savename).exists():
		if not QgsVectorFileWriter.deleteShapeFile(QString(savename)):
			return "Failure deleting existing shapefile: " + savename

	outfile = QgsVectorFileWriter(QString(savename), QString("System"), destfields,
			layer.dataProvider().geometryType(), layer.dataProvider().crs())

	if (outfile.hasError() != QgsVectorFileWriter.NoError):
		return "Failure creating output shapefile: " + unicode(outfile.errorMessage())


	# Write the features with modified attributes
	feature = QgsFeature()
	featurecount = layer.dataProvider().featureCount();
	layer.dataProvider().select(layer.dataProvider().attributeIndexes())
	layer.dataProvider().rewind()
	while layer.dataProvider().nextFeature(feature):
		qgis.mainWindow().statusBar().showMessage("Writing feature " + \
			unicode(feature.id()) + " of " + unicode(featurecount))

		attributes = feature.attributeMap()
		for index, field in layer.dataProvider().fields().iteritems():
			if (field.type() != destfields[index].type()):
				floatvalue, test = attributes[index].toDouble()
				if not test:
					floatvalue = 0
				value = prefix + format_float(floatvalue, separator, decimals) + suffix
				attributes[index] = QVariant(value)

		feature.setAttributeMap(attributes)
		outfile.addFeature(feature)

	del outfile

	if addlayer:
		vlayer = qgis.addVectorLayer(savename, os.path.basename(savename), "ogr")
		
	qgis.mainWindow().statusBar().showMessage(unicode(changecount) + " numeric converted to text")

	return None

# --------------------------------------------------------------
#    mmqgisx_geocode_google - Geocode CSV points from Google Maps
# --------------------------------------------------------------

def mmqgisx_geocode_google(qgis, csvname, shapefilename, notfoundfile, keys, addlayer):
	# Read the CSV file header
	try:
		infile = open(csvname, 'r')
	except:
		return "Failure opening " + csvname

	try:
		dialect = csv.Sniffer().sniff(infile.read(2048))
	except:
		return "Failure reading " + unicode(csvname) + ": " + unicode(sys.exc_info()[1])


	fields = {}
	indices = []
	try:
		infile.seek(0)
		reader = csv.reader(infile, dialect)
		header = reader.next()
	except:
		return "Failure reading " + unicode(csvname) + ": " + unicode(sys.exc_info()[1])

	for x in range(0, len(header)):
		for y in range(0, len(keys)):
			if header[x] == keys[y]:
				indices.append(x)

		fieldname = header[x].strip()
		fields[len(fields)] = QgsField(fieldname[0:9], QVariant.String)

	if (len(fields) <= 0) or (len(indices) <= 0):
		return "No valid location fields in " + csvname


	# Create the CSV file for ungeocoded records
	try:
		notfound = open(notfoundfile, 'w')
	except:
		return "Failure opening " + notfoundfile

	notwriter = csv.writer(notfound, dialect)
	notwriter.writerow(header)


	# Create the output shapefile
	if QFile(shapefilename).exists():
		if not QgsVectorFileWriter.deleteShapeFile(QString(shapefilename)):
			return "Failure deleting existing shapefile: " + unicode(shapefilename)

	crs = QgsCoordinateReferenceSystem()
	crs.createFromSrid(4326)
	outfile = QgsVectorFileWriter(QString(shapefilename), QString("System"), fields, QGis.WKBPoint, crs)

	if (outfile.hasError() != QgsVectorFileWriter.NoError):
		return "Failure creating output shapefile: " + unicode(outfile.errorMessage())

	# Geocode and import

	recordcount = 0
	notfoundcount = 0
	for row in reader:
		time.sleep(0.5) # to avoid Google rate quota limits

		recordcount += 1	
		qgis.mainWindow().statusBar().showMessage("Geocoding " + unicode(recordcount) + 
			" (" + unicode(notfoundcount) + " not found)")

		address = ""
		for x in indices:
			if x < len(row):
				value = row[x].strip().replace(" ","+")
				if len(value) > 0:
					if x != indices[0]:
						address += "+"
					address += value

		if len(address) <= 0:
			notfoundcount += 1
			notwriter.writerow(row)
	
		else:
			url = "http://maps.googleapis.com/maps/api/geocode/xml?sensor=false&address=" + address
			xml = urllib.urlopen(url).read()

			latstart = xml.find("<lat>")
			latend = xml.find("</lat>")
			longstart = xml.find("<lng>")
			longend = xml.find("</lng>")

			if (latstart > 0) and (latend > (latstart + 5)) and \
			   (longstart > 0) and (longend > (longstart + 5)):
				x = float(xml[longstart + 5:longend])
				y = float(xml[latstart + 5:latend])
				# print address + ": " + str(x) + ", " + str(y)

				attributes = {}
				for z in range(0, len(header)):
					if z < len(row):
						attributes[z] = QVariant(row[z].strip())

				#y = 40.714353
				#x = -74.005973 
				newfeature = QgsFeature()
				newfeature.setAttributeMap(attributes)
				geometry = QgsGeometry.fromPoint(QgsPoint(x, y))
				newfeature.setGeometry(geometry)
				outfile.addFeature(newfeature)

			else:
				notfoundcount += 1
				notwriter.writerow(row)
				# print xml

	del outfile
	del notfound

	if addlayer and (recordcount > notfoundcount) and (recordcount > 0):
		vlayer = qgis.addVectorLayer(shapefilename, os.path.basename(shapefilename), "ogr")
		
	qgis.mainWindow().statusBar().showMessage(unicode(recordcount - notfoundcount) + " of " + unicode(recordcount)
		+ " addresses geocoded with Google")

	return None

# --------------------------------------------------------
#    mmqgisx_geometry_convert - Convert geometries to
#		simpler types
# --------------------------------------------------------

def mmqgisx_geometry_convert(qgis, layername, newtype, splitnodes, savename, addlayer):
	layer = mmqgisx_find_layer(layername)

	if (layer == None) and (layer.type() != QgsMapLayer.VectorLayer):
		return "Invalid Vector Layer " + layername

	# Create output file
	if len(savename) <= 0:
		return "Invalid output filename given"

	if QFile(savename).exists():
		if not QgsVectorFileWriter.deleteShapeFile(QString(savename)):
			return "Failure deleting existing shapefile: " + savename

	outfile = QgsVectorFileWriter(QString(savename), QString("System"), 
		layer.dataProvider().fields(), newtype, layer.dataProvider().crs())

	if (outfile.hasError() != QgsVectorFileWriter.NoError):
		return "Failure creating output shapefile: " + unicode(outfile.errorMessage())

	# Iterate through each feature in the source layer
	feature_count = layer.dataProvider().featureCount()

	feature = QgsFeature()
	layer.dataProvider().select(layer.dataProvider().attributeIndexes())
	layer.dataProvider().rewind()
        while layer.dataProvider().nextFeature(feature):
		shapeid = unicode(feature.id()).strip()

		qgis.mainWindow().statusBar().showMessage("Converting feature " + shapeid + " of " + unicode(feature_count))

		if (feature.geometry().wkbType() == QGis.WKBPoint) or \
		   (feature.geometry().wkbType() == QGis.WKBPoint25D):

			if (newtype == QGis.WKBPoint):
				newfeature = QgsFeature()
				newfeature.setAttributeMap(feature.attributeMap())
				newfeature.setGeometry(feature.asPoint())
				outfile.addFeature(newfeature)

			else:
				return "Invalid Conversion: " + mmqgisx_wkbtype_to_text(feature.geometry().wkbType()) + \
					" to " + mmqgisx_wkbtype_to_text(newtype)

		elif (feature.geometry().wkbType() == QGis.WKBLineString) or \
		     (feature.geometry().wkbType() == QGis.WKBLineString25D):

			if (newtype == QGis.WKBPoint) and splitnodes:
				polyline = feature.geometry().asPolyline()
				for point in polyline:
					newfeature = QgsFeature()
					newfeature.setAttributeMap(feature.attributeMap())
					newfeature.setGeometry(QgsGeometry.fromPoint(point))
					outfile.addFeature(newfeature)

			elif (newtype == QGis.WKBPoint):
				newfeature = QgsFeature()
				newfeature.setAttributeMap(feature.attributeMap())
				newfeature.setGeometry(feature.geometry().centroid())
				outfile.addFeature(newfeature)

			elif (newtype == QGis.WKBLineString):
				newfeature = QgsFeature()
				newfeature.setAttributeMap(feature.attributeMap())
				newfeature.setGeometry(feature.geometry())
				outfile.addFeature(newfeature)
				
			else:
				return "Invalid Conversion: " + mmqgisx_wkbtype_to_text(feature.geometry().wkbType()) + \
					" to " + mmqgisx_wkbtype_to_text(newtype)

		elif (feature.geometry().wkbType() == QGis.WKBPolygon) or \
		     (feature.geometry().wkbType() == QGis.WKBPolygon25D):

			if (newtype == QGis.WKBPoint) and splitnodes:
				polygon = feature.geometry().asPolygon()
				for polyline in polygon:
					for point in polyline:
						newfeature = QgsFeature()
						newfeature.setAttributeMap(feature.attributeMap())
						newfeature.setGeometry(QgsGeometry.fromPoint(point))
						outfile.addFeature(newfeature)

			elif (newtype == QGis.WKBPoint):
				newfeature = QgsFeature()
				newfeature.setAttributeMap(feature.attributeMap())
				newfeature.setGeometry(feature.geometry().centroid())
				outfile.addFeature(newfeature)

			elif (newtype == QGis.WKBMultiLineString):
				linestrings = []
				polygon = feature.geometry().asPolygon()
				for polyline in polygon:
					linestrings.append(polyline)

				newfeature = QgsFeature()
				newfeature.setAttributeMap(feature.attributeMap())
				newfeature.setGeometry(QgsGeometry.fromMultiPolyline(linestrings))
				outfile.addFeature(newfeature)

			elif (newtype == QGis.WKBPolygon):
				newfeature = QgsFeature()
				newfeature.setAttributeMap(feature.attributeMap())
				newfeature.setGeometry(feature.geometry())
				outfile.addFeature(newfeature)
				
			else:
				return "Invalid Conversion: " + mmqgisx_wkbtype_to_text(feature.geometry().wkbType()) + \
					" to " + mmqgisx_wkbtype_to_text(newtype)

		elif (feature.geometry().wkbType() == QGis.WKBMultiPoint) or \
		     (feature.geometry().wkbType() == QGis.WKBMultiPoint25D):

			if (newtype == QGis.WKBPoint) and splitnodes:
				points = feature.geometry().asMultiPoint()
				for point in points:
					newfeature = QgsFeature()
					newfeature.setAttributeMap(feature.attributeMap())
					newfeature.setGeometry(QgsGeometry.fromPoint(point))
					outfile.addFeature(newfeature)

			elif (newtype == QGis.WKBPoint):
				newfeature = QgsFeature()
				newfeature.setAttributeMap(feature.attributeMap())
				newfeature.setGeometry(feature.geometry().centroid())
				outfile.addFeature(newfeature)

			else:
				return "Invalid Conversion: " + mmqgisx_wkbtype_to_text(feature.geometry().wkbType()) + \
					" to " + mmqgisx_wkbtype_to_text(newtype)


		elif (feature.geometry().wkbType() == QGis.WKBMultiLineString) or \
		     (feature.geometry().wkbType() == QGis.WKBMultiLineString25D):

			if (newtype == QGis.WKBPoint) and splitnodes:
				polylines = feature.geometry().asMultiPolyline()
				for polyline in polylines:
					for point in polyline:
						newfeature = QgsFeature()
						newfeature.setAttributeMap(feature.attributeMap())
						newfeature.setGeometry(QgsGeometry.fromPoint(point))
						outfile.addFeature(newfeature)

			elif (newtype == QGis.WKBPoint):
				newfeature = QgsFeature()
				newfeature.setAttributeMap(feature.attributeMap())
				newfeature.setGeometry(feature.geometry().centroid())
				outfile.addFeature(newfeature)

			elif (newtype == QGis.WKBLineString):
				linestrings = feature.geometry().asMultiPolyline()
				for linestring in linestrings:
					newfeature = QgsFeature()
					newfeature.setAttributeMap(feature.attributeMap())
					newfeature.setGeometry(QgsGeometry.fromPolyline(linestring))
					outfile.addFeature(newfeature)

			elif (newtype == QGis.WKBMultiLineString):
				linestrings = feature.geometry().asMultiPolyline()
				newfeature = QgsFeature()
				newfeature.setAttributeMap(feature.attributeMap())
				newfeature.setGeometry(QgsGeometry.fromMultiPolyline(linestrings))
				outfile.addFeature(newfeature)

			else:
				return "Invalid Conversion: " + mmqgisx_wkbtype_to_text(feature.geometry().wkbType()) + \
					" to " + mmqgisx_wkbtype_to_text(newtype)

		elif (feature.geometry().wkbType() == QGis.WKBMultiPolygon) or \
		     (feature.geometry().wkbType() == QGis.WKBMultiPolygon25D):

			if (newtype == QGis.WKBPoint) and splitnodes:
				polygons = feature.geometry().asMultiPolygon()
				for polygon in polygons:
					for polyline in polygon:
						for point in polyline:
							newfeature = QgsFeature()
							newfeature.setAttributeMap(feature.attributeMap())
							newfeature.setGeometry(QgsGeometry.fromPoint(point))
							outfile.addFeature(newfeature)
	
			elif (newtype == QGis.WKBPoint):
				newfeature = QgsFeature()
				newfeature.setAttributeMap(feature.attributeMap())
				newfeature.setGeometry(feature.geometry().centroid())
				outfile.addFeature(newfeature)

			elif (newtype == QGis.WKBLineString):
				polygons = feature.geometry().asMultiPolygon()
				for polygon in polygons:
					newfeature = QgsFeature()
					newfeature.setAttributeMap(feature.attributeMap())
					newfeature.setGeometry(QgsGeometry.fromPolyline(polygon))
					outfile.addFeature(newfeature)

			elif (newtype == QGis.WKBPolygon):
				polygons = feature.geometry().asMultiPolygon()
				for polygon in polygons:
					newfeature = QgsFeature()
					newfeature.setAttributeMap(feature.attributeMap())
					newfeature.setGeometry(QgsGeometry.fromPolygon(polygon))
					outfile.addFeature(newfeature)

			elif (newtype == QGis.WKBMultiLineString) or \
			     (newtype == QGis.WKBMultiPolygon):
				polygons = feature.geometry().asMultiPolygon()
				newfeature = QgsFeature()
				newfeature.setAttributeMap(feature.attributeMap())
				newfeature.setGeometry(QgsGeometry.fromMultiPolygon(polygons))
				outfile.addFeature(newfeature)

			else:
				return "Invalid Conversion: " + mmqgisx_wkbtype_to_text(feature.geometry().wkbType()) + \
					" to " + mmqgisx_wkbtype_to_text(newtype)
			
	del outfile

	if addlayer:
		qgis.addVectorLayer(savename, os.path.basename(savename), "ogr")

	qgis.mainWindow().statusBar().showMessage(unicode(feature_count) + " features converted")

	return None

# --------------------------------------------------------
#    mmqgisx_geometry_export_to_csv - Shape node dump to CSV
# --------------------------------------------------------

def mmqgisx_geometry_export_to_csv(qgis, layername, node_filename, attribute_filename, field_delimiter, line_terminator):
	layer = mmqgisx_find_layer(layername)

	if (layer == None) or (layer.type() != QgsMapLayer.VectorLayer):
		return "Invalid Vector Layer " + layername

	node_header = ["shapeid", "x", "y"]
	attribute_header = ["shapeid"]
	for index, field in layer.dataProvider().fields().iteritems():
		if (layer.geometryType() == QGis.Point):
			node_header.append(field.name())
		else:
			attribute_header.append(field.name())

	try:
		nodefile = open(node_filename, 'w')
    	except:
		return "Failure opening " + node_filename

	node_writer = csv.writer(nodefile, delimiter = field_delimiter, lineterminator = line_terminator)
	node_writer.writerow(node_header)

	if (layer.geometryType() != QGis.Point):
		try:
			attributefile = open(attribute_filename, 'w')
 	   	except:
			return "Failure opening " + attribute_filename

		attribute_writer = csv.writer(attributefile, delimiter = field_delimiter, lineterminator = line_terminator)
		attribute_writer.writerow(attribute_header)


	# Iterate through each feature in the source layer
	feature_count = layer.dataProvider().featureCount()

	feature = QgsFeature()
	layer.dataProvider().select(layer.dataProvider().attributeIndexes())
	layer.dataProvider().rewind()
        while layer.dataProvider().nextFeature(feature):
		shapeid = unicode(feature.id()).strip()

		qgis.mainWindow().statusBar().showMessage("Exporting feature " + shapeid + " of " + unicode(feature_count))

		if (feature.geometry() == None):
			return "Cannot export layer with no shape data"

		elif (feature.geometry().type() == QGis.Point):
			point = feature.geometry().asPoint()
			row = [ shapeid, unicode(point.x()), unicode(point.y()) ]
			for attindex, attribute in feature.attributeMap().iteritems():
				row.append(unicode(attribute.toString()).encode("iso-8859-1"))
			node_writer.writerow(row)

		# elif (feature.geometry().wkbType() == QGis.WKBLineString):
		elif (feature.geometry().type() == QGis.Line):
			polyline = feature.geometry().asPolyline()
			#for point in polyline.iteritems():
			for point in polyline:
				row = [ shapeid, unicode(point.x()), unicode(point.y()) ]
				node_writer.writerow(row)

			row = []
			for attindex, attribute in feature.attributeMap().iteritems():
				row.append(attribute.toString())
			attribute_writer.writerow(row)

		# elif (feature.geometry().wkbType() == QGis.WKBPolygon):
		elif (feature.geometry().type() == QGis.Polygon):
			polygon = feature.geometry().asPolygon()
			for polyline in polygon:
				for point in polyline:
					row = [ shapeid, unicode(point.x()), unicode(point.y()) ]
					node_writer.writerow(row)

			row = [shapeid]
			for attindex, attribute in feature.attributeMap().iteritems():
				row.append(unicode(attribute.toString()).encode("iso-8859-1"))
			attribute_writer.writerow(row)
			
	del nodefile
	if (layer.geometryType() != QGis.Point):
		del attributefile

	qgis.mainWindow().statusBar().showMessage(unicode(feature_count) + " records exported")

	return None


# --------------------------------------------------------
#    mmqgisx_geometry_import_from_csv - Shape node import from CSV
# --------------------------------------------------------

def mmqgisx_geometry_import_from_csv(qgis, node_filename, long_colname, lat_colname, 
	shapeid_colname, geometry_type, shapefile_name, addlayer):
	try:
		infile = open(node_filename, 'r')
	except:
		return "Failure opening " + node_filename
			
	try:
		dialect = csv.Sniffer().sniff(infile.read(2048))
	except:
		return "Bad CSV file (verify that your delimiters are consistent): " + node_filename

	infile.seek(0)
	reader = csv.reader(infile, dialect)
	header = reader.next()

	lat_col = -1
	long_col = -1
	shapeid_col = -1
	for x in range(len(header)):
		# print header[x]
		if (header[x] == lat_colname):
			lat_col = x
		elif (header[x] == long_colname):
			long_col = x
		elif (header[x] == shapeid_colname):
			shapeid_col = x

	if (lat_col < 0):
		return "Invalid latitude column name: " + lat_colname

	if (long_col < 0):
		return "Invalid longitude column name: " + long_colname

	if (shapeid_col < 0):
		return "Invalid shape ID column name: " + shapeid_colname

	if (geometry_type == "Point"):
		wkb_type = QGis.WKBPoint

	elif (geometry_type == "Polyline"):
		wkb_type = QGis.WKBLineString

	elif (geometry_type == "Polygon"):
		wkb_type = QGis.WKBPolygon
	else:
		return "Invalid geometry type: " + geometry_type

	# Create the output shapefile
	if QFile(shapefile_name).exists():
		if not QgsVectorFileWriter.deleteShapeFile(QString(shapefile_name)):
			return "Failure deleting existing shapefile: " + shapefile_name

	if qgis.activeLayer() and qgis.activeLayer().dataProvider():
		crs = qgis.activeLayer().dataProvider().crs()
	else:
		crs = QgsCoordinateReferenceSystem()
		crs.createFromSrid(4326) # WGS 84

	fields = { 0 : QgsField(shapeid_colname, QVariant.String) }
	if (geometry_type == "Point"):
		for x in range(len(header)):
			if ((x != lat_col) and (x != long_col) and (x != shapeid_col)):
				fields[len(fields)] = QgsField(header[x], QVariant.String)

	outfile = QgsVectorFileWriter(QString(shapefile_name), QString("System"), fields, wkb_type, crs)

	if (outfile.hasError() != QgsVectorFileWriter.NoError):
		return "Failure creating output shapefile: " + unicode(outfile.errorMessage())

	polyline = []
	node_count = 0
	shape_count = 0
	current_shape_id = False
	reading = True
	while reading:
		try:
			row = reader.next()
		except:
			reading = False

		if reading and (len(row) > long_col) and (len(row) > lat_col) and (len(row) > shapeid_col) \
				and mmqgisx_is_float(row[long_col]) and mmqgisx_is_float(row[lat_col]):
			node_count += 1
			qgis.mainWindow().statusBar().showMessage("Importing node " + unicode(node_count))
			point = QgsPoint(float(row[long_col]), float(row[lat_col]))
		else:
			point = False

		if reading and (wkb_type != QGis.WKBPoint) and (row[shapeid_col] == current_shape_id):
			polyline.append(point)

		else:
			#print str(wkb_type) + ": " + str(current_shape_id)
			#print polyline

			bad_feature = False
			if wkb_type == QGis.WKBPoint:
				if point:
					geometry = QgsGeometry.fromPoint(point)
					current_shape_id = row[shapeid_col]
				else:
					bad_feature = True

			elif wkb_type == QGis.WKBLineString:
				if len(polyline) < 2:
					bad_feature = True
				else:
					geometry = QgsGeometry.fromPolyline(polyline)

			elif wkb_type == QGis.WKBPolygon:
				if len(polyline) < 3:
					bad_feature = True
				else:
					# polyline[len(polyline) - 1] = polyline[0] # must close polygons
					polygon = [ polyline ]
					geometry = QgsGeometry.fromPolygon(polygon)

			if not bad_feature:
				attributes = { 0:QVariant(str(current_shape_id)) }
				if (geometry_type == "Point"):
					for x in range(len(header)):
						if x >= len(row):
							attributes[len(attributes)] = QVariant("")
						elif ((x != lat_col) and (x != long_col) and (x != shapeid_col)):
							attributes[len(attributes)] = QVariant(str(row[x]))

				#print attributes
				newfeature = QgsFeature()
				newfeature.setAttributeMap(attributes)
				newfeature.setGeometry(geometry)
				outfile.addFeature(newfeature)
				shape_count += 1
	
			polyline = []
			if reading and point:
				current_shape_id = row[shapeid_col]
				polyline.append(point)

	del infile
	del outfile

	if addlayer:
		qgis.addVectorLayer(shapefile_name, os.path.basename(shapefile_name), "ogr")
		
	qgis.mainWindow().statusBar().showMessage("Loaded " + unicode(shape_count) + " shapes (" + unicode(node_count) + " nodes")

	return None

# --------------------------------------------------------
#    mmqgisx_grid - Grid shapefile creation
# --------------------------------------------------------

def mmqgisx_grid(qgis, savename, hspacing, vspacing, width, height, originx, originy, gridtype, addlayer):
	if len(savename) <= 0:
		return "No output filename given"

	if (hspacing <= 0) or (vspacing <= 0):
		return "Invalid grid spacing: " + unicode(hspacing) + " / " + unicode(vspacing)
	
	if (width <= hspacing) or (width < vspacing):
		return "Invalid width / height: " + unicode(width) + " / " + unicode(height)
		
	fields = {
		0 : QgsField("longitude", QVariant.Double, "real", 24, 16, "Longitude"),
		1 : QgsField("latitude", QVariant.Double, "real", 24, 16, "Latitude") }

	if QFile(savename).exists():
		if not QgsVectorFileWriter.deleteShapeFile(QString(savename)):
			return "Failure deleting existing shapefile: " + savename


	if gridtype.find("polygon") >= 0:
		shapetype = QGis.WKBPolygon
	else:
		shapetype = QGis.WKBLineString

	# print gridtype + "," + str(shapetype)
		
	outfile = QgsVectorFileWriter(QString(savename), QString("System"), fields, 
		shapetype, QgsCoordinateReferenceSystem());

	if (outfile.hasError() != QgsVectorFileWriter.NoError):
		return "Failure creating output shapefile: " + unicode(outfile.errorMessage())

	linecount = 0
	if gridtype == "Rectangle (line)":
		x = originx
		while x <= (originx + width):
			polyline = []
			geometry = QgsGeometry()
			feature = QgsFeature()
			
			y = originy
			while y <= (originy + height):
				polyline.append(QgsPoint(x, y))
				y = y + vspacing;

			feature.setGeometry(geometry.fromPolyline(polyline))
			feature.addAttribute(0, QVariant(x))
			feature.addAttribute(1, QVariant(0))
			outfile.addFeature(feature)
			linecount = linecount + 1
			#self.iface.mainWindow().statusBar().showMessage("Line " + str(linecount) + " " + str(x))
			x = x + hspacing;

		y = originy
		while y <= (originy + height):
			polyline = []
			geometry = QgsGeometry()
			feature = QgsFeature()
			
			x = originx
			while x <= (originx + width):
				polyline.append(QgsPoint(x, y))
				x = x + hspacing;

			feature.setGeometry(geometry.fromPolyline(polyline))
			feature.addAttribute(0, QVariant(0))
			feature.addAttribute(1, QVariant(y))
			outfile.addFeature(feature)
			linecount = linecount + 1
			#self.iface.mainWindow().statusBar().showMessage("Line " + str(linecount) + " " + str(y))
			y = y + hspacing;

	elif gridtype == "Rectangle (polygon)":
		x = originx
		while x < (originx + width):
			y = originy
			while y < (originy + height):
				polyline = []
				polyline.append(QgsPoint(x, y))
				polyline.append(QgsPoint(x + hspacing, y))
				polyline.append(QgsPoint(x + hspacing, y + vspacing))
				polyline.append(QgsPoint(x, y + vspacing))

				geometry = QgsGeometry()
				feature = QgsFeature()
				feature.setGeometry(geometry.fromPolygon([polyline]))
				feature.addAttribute(0, QVariant(x + (hspacing / 2.0)))
				feature.addAttribute(1, QVariant(y + (vspacing / 2.0)))
				outfile.addFeature(feature)
				linecount = linecount + 1
				y = y + hspacing;

			x = x + vspacing

	elif gridtype == "Diamond (polygon)":
		x = originx
		colnum = 0
		while x < (originx + width):
			if (colnum % 2) == 0:
				y = originy
			else:
				y = originy + (vspacing / 2.0)

			while y < (originy + height):
				polyline = []
				polyline.append(QgsPoint(x + (hspacing / 2.0), y))
				polyline.append(QgsPoint(x + hspacing, y + (vspacing / 2.0)))
				polyline.append(QgsPoint(x + (hspacing / 2.0), y + vspacing))
				polyline.append(QgsPoint(x, y + (vspacing / 2.0)))

				geometry = QgsGeometry()
				feature = QgsFeature()
				feature.setGeometry(geometry.fromPolygon([polyline]))
				feature.addAttribute(0, QVariant(x + (hspacing / 2.0)))
				feature.addAttribute(1, QVariant(y + (vspacing / 2.0)))
				outfile.addFeature(feature)
				linecount = linecount + 1
				y = y + vspacing;

			x = x + (hspacing / 2.0)
			colnum = colnum + 1

	elif gridtype == "Hexagon (polygon)":
		# To preserve symmetry, hspacing is fixed relative to vspacing
		xvertexlo = 0.288675134594813 * vspacing;
		xvertexhi = 0.577350269189626 * vspacing;
		hspacing = xvertexlo + xvertexhi

		x = originx + xvertexhi
		# print str(x) + ", " + str(originx + width)

		colnum = 0
		while x < (originx + width):
			if (colnum % 2) == 0:
				y = originy + (vspacing / 2.0)
			else:
				y = originy + vspacing

			# print str(x) + "," + str(y)

			while y < (originy + height):
				polyline = []
				polyline.append(QgsPoint(x + xvertexhi, y))
				polyline.append(QgsPoint(x + xvertexlo, y + (vspacing / 2.0)))
				polyline.append(QgsPoint(x - xvertexlo, y + (vspacing / 2.0)))
				polyline.append(QgsPoint(x - xvertexhi, y))
				polyline.append(QgsPoint(x - xvertexlo, y - (vspacing / 2.0)))
				polyline.append(QgsPoint(x + xvertexlo, y - (vspacing / 2.0)))

				geometry = QgsGeometry()
				feature = QgsFeature()
				feature.setGeometry(geometry.fromPolygon([polyline]))
				feature.addAttribute(0, QVariant(x))
				feature.addAttribute(1, QVariant(y))
				outfile.addFeature(feature)
				linecount = linecount + 1
				y = y + vspacing;

			x = x + hspacing
			colnum = colnum + 1

	del outfile

	if addlayer:
		qgis.addVectorLayer(savename, os.path.basename(savename), "ogr")
		
	qgis.mainWindow().statusBar().showMessage(unicode(linecount) + " feature grid shapefile created")

	return None

# --------------------------------------------------------
#    mmqgisx_gridify - Snap shape verticies to grid
# --------------------------------------------------------

def mmqgisx_gridify_layer(qgis, layername, hspacing, vspacing, savename, addlayer):
	layer = mmqgisx_find_layer(layername)
	if not layer:
		return "Project has no active vector layer to gridify"
	
	if (hspacing <= 0) or (vspacing <= 0):
		return "Invalid grid spacing: " + unicode(hspacing) + "/" + unicode(vspacing)

	if len(savename) <= 0:
		return "No output filename given"

	if QFile(savename).exists():
		if not QgsVectorFileWriter.deleteShapeFile(QString(savename)):
			return "Failure deleting existing shapefile: " + savename

	outfile = QgsVectorFileWriter(QString(savename), QString("System"), layer.dataProvider().fields(),
			layer.dataProvider().geometryType(), layer.dataProvider().crs())

	if (outfile.hasError() != QgsVectorFileWriter.NoError):
		return "Failure creating output shapefile: " + unicode(outfile.errorMessage())

	point_count = 0
	deleted_points = 0

	feature_number = 0
	feature_count = layer.dataProvider().featureCount()
	layer.dataProvider().select(layer.dataProvider().attributeIndexes())
	layer.dataProvider().rewind()

	feature = QgsFeature()
        while layer.dataProvider().nextFeature(feature):
		qgis.mainWindow().statusBar().showMessage("Gridifying feature " + unicode(feature_number + 1))

		geometry = feature.geometry()

		if geometry.wkbType() == QGis.WKBPoint:
			points, added, deleted = mmqgisx_gridify_points(hspacing, vspacing, [geometry.asPoint()])
			geometry = geometry.fromPoint(points[0])
			point_count += added
			deleted_points += deleted

		elif geometry.wkbType() == QGis.WKBLineString:
			#print "LineString"
			polyline, added, deleted = mmqgisx_gridify_points(hspacing, vspacing, geometry.asPolyline())
			if len(polyline) < 2:
				geometry = None
			else:
				geometry = geometry.fromPolyline(polyline)
			point_count += added
			deleted_points += deleted

		elif geometry.wkbType() == QGis.WKBPolygon:
			newpolygon = []
			for polyline in geometry.asPolygon():
				newpolyline, added, deleted = mmqgisx_gridify_points(hspacing, vspacing, polyline)
				point_count += added
				deleted_points += deleted

				if len(newpolyline) > 1:
					newpolygon.append(newpolyline)

			if len(newpolygon) <= 0:
				geometry = None
			else:
				geometry = geometry.fromPolygon(newpolygon)

		elif geometry.wkbType() == QGis.WKBMultiPoint:
			multipoints, added, deleted = mmqgisx_gridify_points(hspacing, vspacing, geometry.asMultiPoint())
			geometry = geometry.fromMultiPoint(multipoints)
			point_count += added
			deleted_points += deleted

		elif geometry.wkbType() == QGis.WKBMultiLineString:
			#print "MultiLineString"
			newmultipolyline = []
			for polyline in geometry.asMultiPolyline():
				newpolyline, added, deleted = mmqgisx_gridify_points(hspacing, vspacing, polyline)
				if len(newpolyline) > 1:
					newmultipolyline.append(newpolyline)

				if len(newmultipolyline) <= 0:
					geometry = None
				else:
					geometry = geometry.fromMultiPolyline(newmultipolyline)

				point_count += added
				deleted_points += deleted

		elif geometry.wkbType() == QGis.WKBMultiPolygon:
			#print "MultiPolygon"
			newmultipolygon = []
			for polygon in geometry.asMultiPolygon():
				newpolygon = []
				for polyline in polygon:
					newpolyline, added, deleted = mmqgisx_gridify_points(hspacing, vspacing, polyline)

					if len(newpolyline) > 2:
						newpolygon.append(newpolyline)

					point_count += added
					deleted_points += deleted

				if len(newpolygon) > 0:
					newmultipolygon.append(newpolygon)

			if len(newmultipolygon) <= 0:
				geometry = None
			else:
				geometry = geometry.fromMultiPolygon(newmultipolygon)

		else:
			return "Unknown geometry type " + unicode(geometry.wkbType()) + \
				" on feature " + unicode(feature_number + 1)

		# print "Closing feature"
	
		if geometry != None:
			out_feature = QgsFeature()
			out_feature.setGeometry(geometry)
			out_feature.setAttributeMap(feature.attributeMap())
			outfile.addFeature(out_feature)

		feature_number += 1

	del outfile

	if addlayer:
		vlayer = qgis.addVectorLayer(savename, os.path.basename(savename), "ogr")
			
	qgis.mainWindow().statusBar().showMessage("Gridified shapefile created (" + \
		unicode(deleted_points) + " of " + unicode(point_count) + " points deleted)")

	return None


# --------------------------------------------------------
#    mmqgisx_hub_distance - Create shapefile of distances
#			   from points to nearest hub
# --------------------------------------------------------

class mmqgisx_hub:
	def __init__(self, point, newname):
		self.point = point
		self.name = newname


def mmqgisx_hub_distance(qgis, sourcename, destname, nameattributename, units, addlines, savename, addlayer):

	# Error checks
	sourcelayer = mmqgisx_find_layer(sourcename)
	if (sourcelayer == None) or (sourcelayer.featureCount() <= 0):
		return "Origin Layer " + sourcename + " not found"

	hubslayer = mmqgisx_find_layer(destname)
	if (hubslayer == None) or (hubslayer.featureCount() <= 0):
		return "Hub layer " + destname + " not found"

	if sourcename == destname:
		return "Same layer given for both hubs and spokes"

	nameindex = hubslayer.dataProvider().fieldNameIndex(nameattributename)
	if nameindex < 0:
		return "Invalid name attribute: " + nameattributename

	outputtype = QGis.WKBPoint
	if addlines:
		outputtype = QGis.WKBLineString

	# Create output file
	if len(savename) <= 0:
		return "Invalid output filename given"

	if QFile(savename).exists():
		if not QgsVectorFileWriter.deleteShapeFile(QString(savename)):
			return "Failure deleting existing shapefile: " + savename


	outfields = sourcelayer.dataProvider().fields()
	outfields[len(outfields)] = QgsField(QString("HubName"), QVariant.String)
	outfields[len(outfields)] = QgsField(QString("HubDist"), QVariant.Double)

	outfile = QgsVectorFileWriter(QString(savename), QString("System"), outfields, \
		outputtype, sourcelayer.dataProvider().crs())

	if (outfile.hasError() != QgsVectorFileWriter.NoError):
		return "Failure creating output shapefile: " + unicode(outfile.errorMessage())


	# Create array of hubs in memory
	hubs = []
	feature = QgsFeature()
	hubslayer.dataProvider().select(hubslayer.dataProvider().attributeIndexes())
	hubslayer.dataProvider().rewind()
	while hubslayer.dataProvider().nextFeature(feature):
		qgis.mainWindow().statusBar().showMessage("Reading hub " + unicode(feature.id()))
		hubs.append(mmqgisx_hub(feature.geometry().boundingBox().center(), \
				feature.attributeMap()[nameindex].toString()))

	del hubslayer

	# Scan source points, find nearest hub, and write to output file
	writecount = 0
	feature = QgsFeature()
	sourcelayer.dataProvider().select(sourcelayer.dataProvider().attributeIndexes())
	sourcelayer.dataProvider().rewind()
	while sourcelayer.dataProvider().nextFeature(feature):
		source = feature.geometry().boundingBox().center()
		distance = QgsDistanceArea()
		distance.setSourceCrs(sourcelayer.dataProvider().crs().srsid())
		distance.setProjectionsEnabled(1)

		closest = hubs[0]
		hubdist = distance.measureLine(source, closest.point)

		#print unicode(feature.attributeMap()[0])
		for hub in hubs:
			thisdist = distance.measureLine(source, hub.point)
			if thisdist < hubdist:
				closest = hub
				hubdist = thisdist

		attributes = feature.attributeMap()
		attributes[len(attributes)] = QVariant(closest.name)
		if units == "Feet":
			hubdist = hubdist * 3.2808399
		elif units == "Miles":
			hubdist = hubdist * 0.000621371192
		elif units == "Kilometers":
			hubdist = hubdist / 1000
		elif units != "Meters":
                	hubdist = sqrt(pow(source.x() - closest.point.x(), 2.0) + pow(source.y() - closest.point.y(), 2.0))

		#print str(hubdist) + " " + units

		attributes[len(attributes)] = QVariant(hubdist)

		outfeature = QgsFeature()
		outfeature.setAttributeMap(attributes)

		if outputtype == QGis.WKBPoint:
			geometry = QgsGeometry()
			outfeature.setGeometry(geometry.fromPoint(source))
		else:
			polyline = []
			polyline.append(source)
			polyline.append(closest.point)
			geometry = QgsGeometry()
			outfeature.setGeometry(geometry.fromPolyline(polyline))

		outfile.addFeature(outfeature)

		writecount += 1
		qgis.mainWindow().statusBar().showMessage("Writing feature " + unicode(writecount) +\
			" of " + unicode(sourcelayer.dataProvider().featureCount()))

	del outfile

	if addlayer:
		vlayer = qgis.addVectorLayer(savename, os.path.basename(savename), "ogr")
			
	qgis.mainWindow().statusBar().showMessage("Hub distance file created from " + sourcename)

	return None

# --------------------------------------------------------
#    mmqgisx_hub_lines - Create shapefile of lines from
#			spoke points to matching hubs
# --------------------------------------------------------


def mmqgisx_hub_lines(qgis, hubname, hubattr, spokename, spokeattr, savename, addlayer):

	# Find layers
	if hubname == spokename:
		return "Same layer given for both hubs and spokes"

	hublayer = mmqgisx_find_layer(hubname)
	if (hublayer == None) or (hublayer.featureCount() <= 0):
		return "Hub layer " + destname + " not found"

	spokelayer = mmqgisx_find_layer(spokename)
	if spokelayer == None:
		return "Spoke Point Layer " + sourcename + " not found"

	# Find Hub ID attribute indices
	hubindex = hublayer.dataProvider().fieldNameIndex(hubattr)
	if hubindex < 0:
		return "Invalid name attribute: " + hubattr

	spokeindex = spokelayer.dataProvider().fieldNameIndex(spokeattr)
	if spokeindex < 0:
		return "Invalid name attribute: " + spokeattr

	# Create output file
	if len(savename) <= 0:
		return "No output filename given"

	if QFile(savename).exists():
		if not QgsVectorFileWriter.deleteShapeFile(QString(savename)):
			return "Failure deleting existing shapefile: " + savename

	outfields = spokelayer.dataProvider().fields()

	outfile = QgsVectorFileWriter(QString(savename), QString("System"), outfields, \
		QGis.WKBLineString, spokelayer.dataProvider().crs())

	if (outfile.hasError() != QgsVectorFileWriter.NoError):
		return "Failure creating output shapefile: " + unicode(outfile.errorMessage())

	# Scan spoke points
	linecount = 0
	spokepoint = QgsFeature()
	spokelayer.dataProvider().select(spokelayer.dataProvider().attributeIndexes())
	spokelayer.dataProvider().rewind()
	while spokelayer.dataProvider().nextFeature(spokepoint):
		spokex = spokepoint.geometry().boundingBox().center().x()
		spokey = spokepoint.geometry().boundingBox().center().y()
		spokeid = unicode(spokepoint.attributeMap()[spokeindex].toString())
		qgis.mainWindow().statusBar().showMessage("Reading spoke " + unicode(spokepoint.id()))

		# Scan hub points to find first matching hub
		hubpoint = QgsFeature()
		hublayer.dataProvider().select(hublayer.dataProvider().attributeIndexes())
		hublayer.dataProvider().rewind()
		while hublayer.dataProvider().nextFeature(hubpoint):
			hubid = unicode(hubpoint.attributeMap()[hubindex].toString())
			if hubid == spokeid:
				hubx = hubpoint.geometry().boundingBox().center().x()
				huby = hubpoint.geometry().boundingBox().center().y()

				# Write line to the output file
				outfeature = QgsFeature()
				outfeature.setAttributeMap(spokepoint.attributeMap())

				polyline = []
				polyline.append(QgsPoint(spokex, spokey))
				polyline.append(QgsPoint(hubx, huby))
				geometry = QgsGeometry()
				outfeature.setGeometry(geometry.fromPolyline(polyline))
				outfile.addFeature(outfeature)
				linecount = linecount + 1
				break

	del spokelayer
	del hublayer
	del outfile

	if linecount <= 0:
		return "No spoke/hub matches found to create lines"

	if addlayer:
		qgis.addVectorLayer(savename, os.path.basename(savename), "ogr")

	qgis.mainWindow().statusBar().showMessage(unicode(linecount) + " hub/spoke lines written to " + savename)

	return None

# --------------------------------------------------------
#    mmqgisx_label - Create single label points for
#		    single- or multi-feature items
# --------------------------------------------------------

class mmqgisx_label():
	def __init__(self, name, attributemap):
		self.name = name
		self.xsum = 0
		self.ysum = 0
		self.feature_count = 0
		self.attributes = attributemap

def mmqgisx_label_point(qgis, layername, labelattributename, savename, addlayer):
	layer = mmqgisx_find_layer(layername)
	if layer == None:
		return "Invalid layer name " . layername

	labelindex = layer.dataProvider().fieldNameIndex(labelattributename)
	if labelindex < 0:
		return "Invalid label field name: " + labelattributename

	# print  "labelindex = " + str(labelindex)

	if len(savename) <= 0:
		return "No output filename given"

	# Open file (delete any existing)
	if QFile(savename).exists():
		if not QgsVectorFileWriter.deleteShapeFile(QString(savename)):
			return "Failure deleting existing shapefile: " + savename

	outfile = QgsVectorFileWriter(QString(savename), QString("System"), layer.dataProvider().fields(),
				QGis.WKBPoint, layer.dataProvider().crs())

	if (outfile.hasError() != QgsVectorFileWriter.NoError):
		return "Failure creating output shapefile: " + unicode(outfile.errorMessage())

	# Build dictionary of items, averaging center for multi-feature items
	features = {}
	feature = QgsFeature()
	readcount = 0
	feature_count = layer.featureCount()
	layer.dataProvider().select(layer.dataProvider().attributeIndexes())
	layer.dataProvider().rewind()

	while layer.dataProvider().nextFeature(feature):
		key = unicode(feature.attributeMap()[labelindex].toString())
		if not features.has_key(key):
			features[key] = mmqgisx_label(key, feature.attributeMap())

		center = feature.geometry().boundingBox().center();
		features[key].xsum += center.x()
		features[key].ysum += center.y()
		features[key].feature_count += 1

		readcount += 1
		if not (readcount % 10):
			qgis.mainWindow().statusBar().showMessage( \
				"Reading feature " + unicode(readcount) + " of " + unicode(feature_count))



	# Sort keys so features appear in alphabetical order
	keys = features.keys()
	keys.sort()

	# Calculate points and write them to the output file
	writecount = 0
	for key in keys:
		label_point = features[key]
		point = QgsPoint(label_point.xsum / label_point.feature_count, 
			label_point.ysum / label_point.feature_count)

		feature = QgsFeature()
		geometry = QgsGeometry()
		feature.setGeometry(geometry.fromPoint(point))
		feature.setAttributeMap(label_point.attributes)

		if not outfile.addFeature(feature):
			return "Failure writing feature to shapefile"

		writecount += 1
		if not (writecount % 10):
			qgis.mainWindow().statusBar().showMessage( \
				"Writing feature " + unicode(writecount) + " of " + unicode(len(features)))

	del outfile

	if addlayer:
		qgis.addVectorLayer(savename, os.path.basename(savename), "ogr")
		
	qgis.mainWindow().statusBar().showMessage(unicode(writecount) + " label shapefile created from " + layername)

	return None

# --------------------------------------------------------
#    mmqgisx_merge - Merge layers to single shapefile
# --------------------------------------------------------

def mmqgisx_merge(qgis, layernames, savename, addlayer):
	fields = {}
	layers = []
	totalfeaturecount = 0

	for x in range(0, len(layernames)):
		layername = layernames[x]
		layer = mmqgisx_find_layer(layername)
		if layer == None:
			return "Layer " + layername + " not found"

		# Verify that all layers are the same type (point, polygon, etc)
		if (len(layers) > 0):
			if (layer.dataProvider().geometryType() != layers[0].dataProvider().geometryType()):
				return "Merged layers must all be same type of geometry (" + \
					mmqgisx_wkbtype_to_text(layer.dataProvider().geometryType()) + " != " + \
					mmqgisx_wkbtype_to_text(layers[0].dataProvider().geometryType()) + ")"

			#if (layer.dataProvider().crs() != layers[0].dataProvider().crs()):
			#	QMessageBox.critical(qgis.mainWindow(), 
			#		"Merge Layers", "Merged layers must all have same coordinate system")
			#	return None
				
		layers.append(layer)
		totalfeaturecount += layer.featureCount()

		# Add any fields not in the composite field list
		for sindex, sfield in layer.dataProvider().fields().iteritems():
			found = None
			for dindex, dfield in fields.iteritems():
				if (dfield.name() == sfield.name()) and (dfield.type() == sfield.type()):
					found = dfield

			if not found:
				fields[len(fields)] = sfield
			
	if (len(layers) <= 0):
		return "No layers given to merge"
	
	# Create the output shapefile
	if len(savename) <= 0:
		return "No output filename given"

	if QFile(savename).exists():
		if not QgsVectorFileWriter.deleteShapeFile(QString(savename)):
			return "Failure deleting existing shapefile: " + savename

	outfile = QgsVectorFileWriter(QString(savename), QString("System"), fields,
		layers[0].dataProvider().geometryType(), layers[0].dataProvider().crs())

	if (outfile.hasError() != QgsVectorFileWriter.NoError):
		return "Failure creating output shapefile: " + unicode(outfile.errorMessage())

	# Copy layer features to output file
	featurecount = 0
	for layer in layers:
		feature = QgsFeature()
		layer.dataProvider().select(layer.dataProvider().attributeIndexes())
		layer.dataProvider().rewind()
                while layer.dataProvider().nextFeature(feature):
			sattributes = feature.attributeMap()
			dattributes = {}
			for dindex, dfield in fields.iteritems():
				dattributes[dindex] = QVariant(dfield.type())
				for sindex, sfield in layer.dataProvider().fields().iteritems():
					if (sfield.name() == dfield.name()) and (sfield.type() == dfield.type()):
						dattributes[dindex] = sattributes[sindex]
						break

			#for dindex, dfield in dattributes.iteritems():
			#	print layer.name() + " (" + str(dindex) + ") " + str(dfield.toString())

			feature.setAttributeMap(dattributes)
			outfile.addFeature(feature)
			featurecount += 1
			qgis.mainWindow().statusBar().showMessage("Writing feature " + \
				unicode(featurecount) + " of " + unicode(totalfeaturecount))

	del outfile

	# Add the merged layer to the project
	if addlayer:
		qgis.addVectorLayer(savename, os.path.basename(savename), "ogr")

	qgis.mainWindow().statusBar().showMessage(unicode(featurecount) + " records exported")

	return None

# ----------------------------------------------------------
#    mmqgisx_select - Select features by attribute
# ----------------------------------------------------------

def mmqgisx_select(qgis, layername, selectattributename, comparisonvalue, comparisonname, savename, addlayer):
	layer = mmqgisx_find_layer(layername)
	if layer == None:
		return "Project has no active vector layer to select from"

	selectindex = layer.dataProvider().fieldNameIndex(selectattributename)
	if selectindex < 0:
		return "Invalid select field name: " + selectattributename

	# print  "selectindex = " + str(selectindex) + " " + comparisonname + " " + comparisonvalue

	if (not comparisonvalue) or (len(comparisonvalue) <= 0):
		return "No comparison value given"

	if (not savename) or (len(savename) <= 0):
		return "No output filename given"

	if QFile(savename).exists():
		if not QgsVectorFileWriter.deleteShapeFile(QString(savename)):
			return "Failure deleting existing shapefile: " + savename

	outfile = QgsVectorFileWriter(QString(savename), QString("System"), layer.dataProvider().fields(),
			layer.dataProvider().geometryType(), layer.dataProvider().crs())

	if (outfile.hasError() != QgsVectorFileWriter.NoError):
		return "Failure creating output shapefile: " + unicode(outfile.errorMessage())

	readcount = 0
	writecount = 0
	feature = QgsFeature()
	layer.dataProvider().select(layer.dataProvider().attributeIndexes())
	layer.dataProvider().rewind()

	while layer.dataProvider().nextFeature(feature):
		if (comparisonname == 'begins with') or (comparisonname == 'contains') or \
		   (feature.attributeMap()[selectindex].type() == QVariant.String):
			x = unicode(feature.attributeMap()[selectindex].toString())
			y = unicode(comparisonvalue)
		else:
			# print feature.attributeMap()[selectindex].typeName()
			x = float(feature.attributeMap()[selectindex].toString())
			y = float(comparisonvalue)

		match = False
		if (comparisonname == '=='):
			match = (x == y)
		elif (comparisonname == '!='):
			match = (x != y)
		elif (comparisonname == '>'):
			match = (x > y)
		elif (comparisonname == '>='):
			match = (x >= y)
		elif (comparisonname == '<'):
			match = (x < y)
		elif (comparisonname == '<='):
			match = (x <= y)
		elif (comparisonname == 'begins with'):
			match = x.startswith(y)
		elif (comparisonname == 'contains'):
			match = (x.find(y) >= 0)
	
		readcount += 1
		if (match):
			outfile.addFeature(feature)
			writecount += 1

		qgis.mainWindow().statusBar().showMessage("Scanning feature " + \
			unicode(readcount) + " of " + unicode(layer.dataProvider().featureCount()) + \
			"(" + unicode(writecount) + " selected)")

	del outfile

	if addlayer:
		vlayer = qgis.addVectorLayer(savename, os.path.basename(savename), "ogr")
		
	qgis.mainWindow().statusBar().showMessage("Selected " + unicode(writecount) + " features to " + savename)

	return None

# --------------------------------------------------------
#    mmqgisx_sort - Sort shapefile by attribute
# --------------------------------------------------------

def mmqgisx_sort(qgis, layername, sortattributename, savename, direction, addlayer):
	layer = mmqgisx_find_layer(layername)
	if layer == None:
		return "Project has no active vector layer to sort"

	#sortindex = -1
	#sortattributename = self.sortattribute.currentText()
	#for index, fieldname in layer.dataProvider().fields().iteritems():
	#	if fieldname.name() == sortattributename:
	#		sortindex = index

	sortindex = layer.dataProvider().fieldNameIndex(sortattributename)
	if sortindex < 0:
		return "Invalid sort field name: " + sortattributename
	
	# print  "sortindex = " + str(sortindex)

	if len(savename) <= 0:
		return "No output filename given"

	if QFile(savename).exists():
		if not QgsVectorFileWriter.deleteShapeFile(QString(savename)):
			return "Failure deleting existing shapefile: " + savename

	outfile = QgsVectorFileWriter(QString(savename), QString("System"), layer.dataProvider().fields(),
			layer.dataProvider().geometryType(), layer.dataProvider().crs())

	if (outfile.hasError() != QgsVectorFileWriter.NoError):
		return "Failure creating output shapefile: " + unicode(outfile.errorMessage())

	table = []
	feature = QgsFeature()
	layer.dataProvider().select(layer.dataProvider().attributeIndexes())
	layer.dataProvider().rewind()
	while layer.dataProvider().nextFeature(feature):
		featuretype = feature.attributeMap()[sortindex].type()
		if (featuretype == QVariant.Int):
			record = feature.id(), feature.attributeMap()[sortindex].toInt()
		elif (featuretype == QVariant.Double):
			record = feature.id(), feature.attributeMap()[sortindex].toDouble()
		else:
			record = feature.id(), unicode(feature.attributeMap()[sortindex].toString())

		qgis.mainWindow().statusBar().showMessage("Reading feature " + unicode(feature.id()))
		table.append(record)

	if (direction.lower() == "descending"):
		table.sort(key = operator.itemgetter(1), reverse=True)
	else:
		table.sort(key = operator.itemgetter(1))

	writecount = 0
	for record in table:
		feature = QgsFeature()
		layer.featureAtId(record[0], feature)
		outfile.addFeature(feature)
		writecount += 1
		qgis.mainWindow().statusBar().showMessage("Writing feature " + unicode(writecount) +\
			" of " + unicode(len(table)))

	del outfile

	if addlayer:
		vlayer = qgis.addVectorLayer(savename, os.path.basename(savename), "ogr")
		
	qgis.mainWindow().statusBar().showMessage("Sorted shapefile created from " + layername)

	return None

# --------------------------------------------------------
#    mmqgisx_geocode_street_layer - Geocode addresses from street 
#			     address finder shapefile
# --------------------------------------------------------

def mmqgisx_geocode_street_layer(qgis, layername, csvname, addressfield, shapefilename, streetname, 
fromx, fromy, tox, toy, leftfrom, rightfrom, leftto, rightto, setback, notfoundfile, addlayer):

	layer = mmqgisx_find_layer(layername)
	if layer == None:
		return "Address layer not found: " + layername

	if len(csvname) <= 0:
		return "No input CSV file given"

	# Read the CSV file data into memory
	try:
		infile = open(csvname, 'r')
	except:
		return "Failure opening " + csvname

	try:
		dialect = csv.Sniffer().sniff(infile.read(2048))
	except:
		return "Bad CSV file (verify that your delimiters are consistent): " + csvname

	infile.seek(0)
	reader = csv.reader(infile, dialect)

	fields = {}
	header = reader.next()
	addressfield_index = -1
	for field in header:
		if field == addressfield:
			addressfield_index = len(fields)
		fields[len(fields)] = QgsField(field[0:9].strip(), QVariant.String)

	longfield = len(fields)
	latfield = longfield + 1
	fields[longfield] = QgsField("Longitude", QVariant.Double, "real", 24, 16)
	fields[latfield] = QgsField("Latitude", QVariant.Double, "real", 24, 16)

	csv_found = []
	csv_attributes = []
	csv_streetname = []
	csv_streetnumber = []

	for row in reader:
		address = mmqgisx_searchable_streetname(row[addressfield_index])
		x = 0
		while (x < len(address)) and (address[x].isdigit() or (address[x] == "-")):
			x += 1

		if (x > 0):
			number = int(address[0:x].replace("-", ""))
		else:
			number = 0
		csv_streetnumber.append(number)

		street = address[x:].strip().lower()
		csv_streetname.append(street)

		# print str(x) + " \"" + address + "\", " + str(number) + ", " + street

		attributes = {}
		for field in row:
			attributes[len(attributes)] = QVariant(field)
			
		csv_attributes.append(attributes)
		csv_found.append(0)

	del reader
	del infile

	# Create the output shapefile
	if QFile(shapefilename).exists():
		if not QgsVectorFileWriter.deleteShapeFile(QString(shapefilename)):
			return "Failure deleting existing shapefile: " + shapefilename

	outfile = QgsVectorFileWriter(QString(shapefilename), QString("System"), \
		fields, QGis.WKBPoint, layer.dataProvider().crs())

	if (outfile.hasError() != QgsVectorFileWriter.NoError):
		return "Failure creating output shapefile: " + unicode(outfile.errorMessage())

	streetname_attribute = layer.dataProvider().fieldNameIndex(streetname)
	fromx_attribute = layer.dataProvider().fieldNameIndex(fromx)
	fromy_attribute = layer.dataProvider().fieldNameIndex(fromy)
	tox_attribute = layer.dataProvider().fieldNameIndex(tox)
	toy_attribute = layer.dataProvider().fieldNameIndex(toy)
	leftfrom_attribute = layer.dataProvider().fieldNameIndex(leftfrom)
	rightfrom_attribute = layer.dataProvider().fieldNameIndex(rightfrom)
	leftto_attribute = layer.dataProvider().fieldNameIndex(leftto)
	rightto_attribute = layer.dataProvider().fieldNameIndex(rightto)

	# Iterate through each feature in the source layer
	matched_count = 0
	feature_count = layer.dataProvider().featureCount()

	feature = QgsFeature()
	layer.dataProvider().select(layer.dataProvider().attributeIndexes())
	layer.dataProvider().rewind()

	while layer.dataProvider().nextFeature(feature):
		if (feature.id() % 20) == 0:
			qgis.mainWindow().statusBar().showMessage("Searching street " + \
				unicode(feature.id()) + " of " + unicode(feature_count) + \
				" (" + unicode(matched_count) + " matched)")

		attributes = feature.attributeMap()
		key = mmqgisx_searchable_streetname(unicode(attributes[streetname_attribute].toString()))

		# Check each address against this feature
		for row in range(0, len(csv_attributes)):
			if (not csv_found[row]) and (csv_streetname[row] == key):
				(leftto, test) = attributes[leftto_attribute].toInt()
				(leftfrom, test) = attributes[leftfrom_attribute].toInt()
				(rightto, test) = attributes[rightto_attribute].toInt()
				(rightfrom, test) = attributes[rightfrom_attribute].toInt()
				# print "Street found \"" + key + "\", " + str(leftfrom) + ", " + str(leftto)

				if ((csv_streetnumber[row] >= leftfrom) and (csv_streetnumber[row] <= leftto) \
				    and ((leftfrom % 2) == (csv_streetnumber[row] % 2))) or \
				   ((csv_streetnumber[row] >= rightfrom) and (csv_streetnumber[row] <= rightto) \
				    and ((rightfrom % 2) == (csv_streetnumber[row] % 2))):
					left = ((leftfrom % 2) == (csv_streetnumber[row] % 2))
					if left:
						if (leftfrom == leftto):
							ratio = 0.5
						else:
							ratio = float(csv_streetnumber[row] - leftfrom) \
								/ float(leftto - leftfrom)
					else:
						if (rightfrom == rightto):
							ratio = 0.5
						else:
							ratio = float(csv_streetnumber[row] - rightfrom) \
								/ float(rightto - rightfrom)
				
					(tox, test) = attributes[tox_attribute].toDouble()
					(toy, test) = attributes[toy_attribute].toDouble()
					(fromx, test) = attributes[fromx_attribute].toDouble()
					(fromy, test) = attributes[fromy_attribute].toDouble()

					# setback from corner
					angle = atan2(toy - fromy, tox - fromx)
					setback_fromx = fromx + (setback * cos(angle))
					setback_tox = tox - (setback * cos(angle))
					setback_fromy = fromy + (setback * sin(angle))
					setback_toy = toy - (setback * sin(angle))

					x = setback_fromx + ((setback_tox - setback_fromx) * ratio)
					y = setback_fromy + ((setback_toy - setback_fromy) * ratio)

					# setback from street center
					if left:
						y += (setback * cos(angle))
						x -= (setback * sin(angle))
					else:
						y -= (setback * cos(angle))
						x += (setback * sin(angle))

					csv_attributes[row][longfield] = QVariant(x)
					csv_attributes[row][latfield] = QVariant(y)

					newfeature = QgsFeature()
					newfeature.setAttributeMap(csv_attributes[row])
					geometry = QgsGeometry.fromPoint(QgsPoint(x, y))
					newfeature.setGeometry(geometry)
					outfile.addFeature(newfeature)
					matched_count += 1
					csv_found[row] += 1

	del outfile

	# Write records that were not joined to the notfound file
	try:
		outfile = open(notfoundfile, 'w')
	except:
		return "Failure opening " + notfoundfile
	else:
		writer = csv.writer(outfile, dialect)
		writer.writerow(header)
		for x in range(0, len(csv_attributes)):
			if not csv_found[x]:
				row = []
				for key, value in csv_attributes[x].iteritems():
					row.append(value.toString())
				writer.writerow(row)
		del outfile

	if matched_count and addlayer:
		vlayer = qgis.addVectorLayer(shapefilename, os.path.basename(shapefilename), "ogr")
		
	qgis.mainWindow().statusBar().showMessage(unicode(matched_count) + " of " + unicode(len(csv_attributes)) \
		+ " addresses geocoded from " + unicode(feature_count) + " street records")

	return None

def mmqgisx_searchable_streetname(name):
	# Use common address abbreviations to reduce naming discrepancies and improve hit ratio
	# print "searchable_name(" + str(name) + ")"
	if not name:
		return ""
	name = name.strip().lower()
	name = name.replace(".", "")
	name = name.replace(" street", " st")
	name = name.replace(" avenue", " av")
	name = name.replace(" plaza", " plz")
	name = name.replace(" ave", " av")
	name = name.replace("saint ", "st ")
	name = name.replace("fort ", "ft ")

	name = name.replace("east", "e")
	name = name.replace("west", "w")
	name = name.replace("north", "n")
	name = name.replace("south", "s")
	name = name.replace("1st", "1")
	name = name.replace("2nd", "2")
	name = name.replace("3rd", "3")
	name = name.replace("4th", "4")
	name = name.replace("5th", "5")
	name = name.replace("6th", "6")
	name = name.replace("7th", "7")
	name = name.replace("8th", "8")
	name = name.replace("9th", "9")
	name = name.replace("0th", "0")
	name = name.replace("1th", "1")
	name = name.replace("2th", "2")
	name = name.replace("3th", "3")
	return name

# ---------------------------------------------------------
#    mmqgisx_text_to_float - Change text fields to numbers
# ---------------------------------------------------------

def mmqgisx_text_to_float(qgis, layername, attributes, savename, addlayer):
	layer = mmqgisx_find_layer(layername)
	if layer == None:
		return "Project has no active vector layer to convert: " + layername

	if len(savename) <= 0:
		return "No output filename given"

	# Build dictionary of fields with selected fields for conversion to floating point
	destfields = {};
	changecount = 0
	for index, field in layer.dataProvider().fields().iteritems():
		destfields[index] = QgsField (field.name(), field.type(), field.typeName(), \
			field.length(), field.precision(), field.comment())
 
		#print "Scan " + str(destfields[index].name())
		if field.name() in attributes:
			#print "Change " + str(destfields[index].name())
			#if (field.type() != QVariant.String) and (field.type() != QVariant.Double) \
			#    and (field.type() != QVariant.Int):
			#	QMessageBox.critical(self.iface.mainWindow(), "Text to Float", \
			#		"Field " + str(field.name()) + " is not a string or number (" + \
			#		str(field.type()) + " " + str(field.typeName()) + ")")
			#	return
			if (field.type() == QVariant.String) or (field.type() == QVariant.Int):
				changecount += 1
				destfields[index].setType(QVariant.Double)
				destfields[index].setLength(20)

	if (changecount <= 0):
		return "No string or integer fields selected for conversion to floating point"


	# Create the output file
	if QFile(savename).exists():
		if not QgsVectorFileWriter.deleteShapeFile(QString(savename)):
			return "Failure deleting existing shapefile: " + savename

	outfile = QgsVectorFileWriter(QString(savename), QString("System"), destfields,
			layer.dataProvider().geometryType(), layer.dataProvider().crs())

	if (outfile.hasError() != QgsVectorFileWriter.NoError):
		return "Failure creating output shapefile: " + unicode(outfile.errorMessage())


	# Write the features with modified attributes
	feature = QgsFeature()
	featurecount = layer.dataProvider().featureCount();
	layer.dataProvider().select(layer.dataProvider().attributeIndexes())
	layer.dataProvider().rewind()
	while layer.dataProvider().nextFeature(feature):
		qgis.mainWindow().statusBar().showMessage("Writing feature " + \
			unicode(feature.id()) + " of " + unicode(featurecount))

		attributes = feature.attributeMap()
		for index, field in layer.dataProvider().fields().iteritems():
			if (field.type() != destfields[index].type()):
				string = unicode(attributes[index].toString())
				multiplier = 1.0
				if string.find("%") >= 0:
					multiplier = 1 / 100.0
					string = string.replace("%", "")
				if string.find(",") >= 0:
					string = string.replace(",", "")

				try:	
					value = float(string) * multiplier
				except:
					value = 0
						
				attributes[index] = QVariant(value)

		#for index, field in attributes.iteritems():
		#	print "  " + str(index) + " = " + str(field.toString())

		feature.setAttributeMap(attributes)
		outfile.addFeature(feature)

	del outfile

	if addlayer:
		vlayer = qgis.addVectorLayer(savename, os.path.basename(savename), "ogr")
		
	qgis.mainWindow().statusBar().showMessage(unicode(changecount) + " text converted to numeric")

	return None


# --------------------------------------------------------
#    mmqgisx_voronoi - Voronoi diagram creation
# --------------------------------------------------------

def mmqgisx_voronoi_diagram(qgis, sourcelayer, savename, addlayer):
	layer = mmqgisx_find_layer(sourcelayer)
	if layer == None:
		return "Layer " + sourcename + " not found"
	
	if len(savename) <= 0:
		return "No output filename given"

	if QFile(savename).exists():
		if not QgsVectorFileWriter.deleteShapeFile(QString(savename)):
			return "Failure deleting existing shapefile: " + savename

	outfile = QgsVectorFileWriter(QString(savename), QString("System"), layer.dataProvider().fields(), \
			QGis.WKBPolygon, layer.dataProvider().crs())

	if (outfile.hasError() != QgsVectorFileWriter.NoError):
		return "Failure creating output shapefile: " + unicode(outfile.errorMessage())

	points = []
	xmin = 0
	xmax = 0
	ymin = 0
	ymax = 0
	feature = QgsFeature()
	layer.dataProvider().select(layer.dataProvider().attributeIndexes())
	layer.dataProvider().rewind()

	while layer.dataProvider().nextFeature(feature):
		# Re-read by feature ID because nextFeature() doesn't always seem to read attributes
		layer.featureAtId(feature.id(), feature)
		geometry = feature.geometry()
		qgis.mainWindow().statusBar().showMessage("Reading feature " + unicode(feature.id()))
		# print str(feature.id()) + ": " + str(geometry.wkbType())
		if geometry.wkbType() == QGis.WKBPoint:
			points.append( (geometry.asPoint().x(), geometry.asPoint().y(), feature.attributeMap()) )
			if (len(points) <= 1) or (xmin > geometry.asPoint().x()):
				xmin = geometry.asPoint().x()
			if (len(points) <= 1) or (xmax < geometry.asPoint().x()):
				xmax = geometry.asPoint().x()
			if (len(points) <= 1) or (ymin > geometry.asPoint().y()):
				ymin = geometry.asPoint().y()
			if (len(points) <= 1) or (ymax < geometry.asPoint().y()):
				ymax = geometry.asPoint().y()

	if (len(points) < 3):
		return "Too few points to create diagram"

	for center in points:
	# for center in [ points[17] ]:
		# print "\nCenter, " + str(center[0]) + ", " + str(center[1])
		qgis.mainWindow().statusBar().showMessage("Processing point " + \
			unicode(center[0]) + ", " + unicode(center[1]))

		# Borders are tangents to midpoints between all neighbors
		tangents = []
		for neighbor in points:
			border = mmqgisx_voronoi_line((center[0] + neighbor[0]) / 2.0, (center[1] + neighbor[1]) / 2.0)
			if ((neighbor[0] != center[0]) or (neighbor[1] != center[1])):
				tangents.append(border)

		# Add edge intersections to clip to extent of points
		offset = (xmax - xmin) * 0.01
		tangents.append(mmqgisx_voronoi_line(xmax + offset, center[1]))
		tangents.append(mmqgisx_voronoi_line(center[0], ymax + offset))
		tangents.append(mmqgisx_voronoi_line(xmin - offset, center[1]))
		tangents.append(mmqgisx_voronoi_line(center[0], ymin - offset))
		#print "Extent x = " + str(xmax) + " -> " + str(xmin) + ", y = " + str(ymax) + " -> " + str(ymin)

		# Find vector distance and angle to border from center point
		for scan in range(0, len(tangents)):
			run = tangents[scan].x - center[0]
			rise = tangents[scan].y - center[1]
			tangents[scan].distance = sqrt((run * run) + (rise * rise))
			if (tangents[scan].distance <= 0):
				tangents[scan].angle = 0
			elif (tangents[scan].y >= center[1]):
				tangents[scan].angle = acos(run / tangents[scan].distance)
			elif (tangents[scan].y < center[1]):
				tangents[scan].angle = (2 * pi) - acos(run / tangents[scan].distance)
			elif (tangents[scan].x > center[0]):
				tangents[scan].angle = pi / 2.0
			else:
				tangents[scan].angle = 3 * pi / 4

			#print "  Tangent, " + str(tangents[scan].x) + ", " + str(tangents[scan].y) + \
			#	", angle " + str(tangents[scan].angle * 180 / pi) + ", distance " + \
			#	str(tangents[scan].distance)


		# Find the closest line - guaranteed to be a border
		closest = -1
		for scan in range(0, len(tangents)):
			if ((closest == -1) or (tangents[scan].distance < tangents[closest].distance)):
				closest = scan

		# Use closest as the first border
		border = mmqgisx_voronoi_line(tangents[closest].x, tangents[closest].y)
		border.angle = tangents[closest].angle
		border.distance = tangents[closest].distance
		borders = [ border ]

		#print "  Border 0) " + str(closest) + " of " + str(len(tangents)) + ", " \
		#	+ str(border.x) + ", " + str(border.y) \
		#	+ ", (angle " + str(border.angle * 180 / pi) + ", distance " \
		#	+ str(border.distance) + ")"

		# Work around the tangents in a CCW circle
		circling = 1
		while circling:
			next = -1
			scan = 0
			while (scan < len(tangents)):
				anglebetween = tangents[scan].angle - borders[len(borders) - 1].angle
				if (anglebetween < 0):
					anglebetween += (2 * pi)
				elif (anglebetween > (2 * pi)):
					anglebetween -= (2 * pi)

				#print "    Scanning " + str(scan) + " of " + str(len(borders)) + \
				#	", " + str(tangents[scan].x) + ", " + str(tangents[scan].y) + \
				#	", angle " + str(tangents[scan].angle * 180 / pi) + \
				#	", anglebetween " + str(anglebetween * 180 / pi)

				# If border intersects to the left
				if (anglebetween < pi) and (anglebetween > 0):
					# A typo here with a reversed slash cost 8/13/2009 debugging
					tangents[scan].iangle = atan2( (tangents[scan].distance / 
						borders[len(borders) - 1].distance) \
						- cos(anglebetween), sin(anglebetween))
					tangents[scan].idistance = borders[len(borders) - 1].distance \
						/ cos(tangents[scan].iangle)

					tangents[scan].iangle += borders[len(borders) - 1].angle

					# If the rightmost intersection so far, it's a candidate for next border
					if (next < 0) or (tangents[scan].iangle < tangents[next].iangle):
						# print "      Take idistance " + str(tangents[scan].idistance)
						next = scan

				scan += 1

			# iangle/distance are for intersection of border with next border
			borders[len(borders) - 1].iangle = tangents[next].iangle
			borders[len(borders) - 1].idistance = tangents[next].idistance

			# Stop circling if back to the beginning
			if (borders[0].x == tangents[next].x) and (borders[0].y == tangents[next].y):
				circling = 0

			else:
				# Add the next border
				border = mmqgisx_voronoi_line(tangents[next].x, tangents[next].y)
				border.angle = tangents[next].angle
				border.distance = tangents[next].distance
				border.iangle = tangents[next].iangle
				border.idistance = tangents[next].idistance
				borders.append(border)
				#print "  Border " + str(len(borders) - 1) + \
				#	") " + str(next) + ", " + str(border.x) + \
				#	", " + str(border.y) + ", angle " + str(border.angle * 180 / pi) +\
				#	", iangle " + str(border.iangle * 180 / pi) +\
				#	", idistance " + str(border.idistance) + "\n"

			# Remove the border from the list so not repeated
			tangents.pop(next)
			if (len(tangents) <= 0):
				circling = 0

		if len(borders) >= 3:
			polygon = []
			for border in borders:
				ix = center[0] + (border.idistance * cos(border.iangle))
				iy = center[1] + (border.idistance * sin(border.iangle))
				#print "  Node, " + str(ix) + ", " + str(iy) + \
				#	", angle " + str(border.angle * 180 / pi) + \
				#	", iangle " + str(border.iangle * 180 / pi) + \
				#	", idistance " + str(border.idistance) + ", from " \
				#	+ str(border.x) + ", " + str(border.y)
				polygon.append(QgsPoint(ix, iy))

			# attributes = { 0:QVariant(center[0]), 1:QVariant(center[1]) }

			geometry = QgsGeometry.fromPolygon([ polygon ])
			feature = QgsFeature()
			feature.setGeometry(geometry)
			feature.setAttributeMap(center[2])
			outfile.addFeature(feature)
				
	del outfile

	if addlayer:
		qgis.addVectorLayer(savename, os.path.basename(savename), "ogr")

	qgis.mainWindow().statusBar().showMessage("Created " + unicode(len(points)) + " polygon Voronoi diagram")

	return None

class mmqgisx_voronoi_line:
	def __init__(self, x, y):
		self.x = x
		self.y = y
		self.angle = 0
		self.distance = 0

	def list(self, title):
		print title + ", " + unicode(self.x) + ", " + unicode(self.y) + \
			", angle " + unicode(self.angle * 180 / pi) + ", distance " + unicode(self.distance)

	def angleval(self):
		return self.angle


