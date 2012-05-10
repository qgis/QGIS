# --------------------------------------------------------
#    mmqgis_library - mmqgis operation functions
#
#    begin                : 10 May 2010
#    copyright            : (c) 2010 by Michael Minn
#    email                : See michaelminn.com
#
#    Modified to be executed from SEXTANTE by Victor Olaya
#
#   MMQGIS is free software and is offered without guarantee
#   or warranty. You can redistribute it and/or modify it
#   under the terms of version 2 of the GNU General Public
#   License (GPL v2) as published by the Free Software
#   Foundation (www.gnu.org).
# --------------------------------------------------------

import csv
import time
import urllib
import os.path
import operator
import tempfile

from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from math import *
from sextante.core.QGisLayers import QGisLayers


# ----------------------------------------------------------
#    mmqgis_attribute_export - Export attributes to CSV file
# ----------------------------------------------------------

def mmqgis_export_attributes(qgis, outfilename, layername, attribute_names, field_delimiter, line_terminator):
	layer = mmqgis_find_layer(layername)
	if layer == None:
		QMessageBox.critical(qgis.mainWindow(), "Attribute Export", "Layer " + layername + " not found")
		return None;

	# Find attribute indices
	attribute_indices = []
	for x in range(0, len(attribute_names)):
		index = layer.dataProvider().fieldNameIndex(attribute_names[x])
		if index == -1:
			QMessageBox.critical(qgis.mainWindow(), "Attribute Export", \
				"Layer " + layername + " has no attribute " + attribute_names[x])
			return None;
		attribute_indices.append(index)

	# Create the CSV file
	try:
		outfile = open(outfilename, 'w')
    	except:
		QMessageBox.critical(qgis.mainWindow(), "Attribute Export", "Failure opening " + outfilename)
		return

	writer = csv.writer(outfile, delimiter = field_delimiter, lineterminator = line_terminator)
	writer.writerow(attribute_names) # header


	# Iterate through each feature in the source layer
	feature_count = layer.dataProvider().featureCount()

	feature = QgsFeature()
	layer.dataProvider().select(layer.dataProvider().attributeIndexes())
	layer.dataProvider().rewind()
        while layer.dataProvider().nextFeature(feature):
		qgis.mainWindow().statusBar().showMessage \
			("Exporting feature " + str(feature.id()) + " of " + str(feature_count))
		attributes = feature.attributeMap()

		row = []
		for column in attribute_indices:
			row.append(unicode(attributes[column].toString()).encode("iso-8859-1"))

		# print unicode(feature.id()).encode("iso-8859-1") + " = " + unicode(row[0]).encode("iso-8859-1")

		writer.writerow(row)

	del writer

	qgis.mainWindow().statusBar().showMessage(str(feature_count) + " records exported")

	return feature_count

# --------------------------------------------------------
#    mmqgis_attribute_join - Join attributes from a CSV
#                            file to a shapefile
# --------------------------------------------------------

def mmqgis_attribute_join(qgis, layername, infilename, joinfield, joinattribute, outfilename, notfoundname):
	layer = mmqgis_find_layer(layername)
	if layer == None:
		QMessageBox.critical(qgis.mainWindow(), "Attribute Export", "Layer " + layername + " not found")
		return None;

	joinattribute_index = layer.fieldNameIndex(joinattribute)
	if joinattribute_index < 0:
		QMessageBox.critical(qgis.mainWindow(), "Attribute Join", "Invalid join attribute " + joinattribute)
		return

	# Create a combined field list from the source layer and the CSV file header
	try:
		infile = open(infilename, 'r')
	except:
		QMessageBox.critical(qgis.mainWindow(), "Attribute Join", "Failure opening " + infilename)
		return

	dialect = csv.Sniffer().sniff(infile.read(1024))
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
		QMessageBox.critical(qgis.mainWindow(), "Attribute Join",
			"Join field " + joinfield + " not found in " + infilename)
		return

	# Create the output shapefile
	if QFile(outfilename).exists():
		if not QgsVectorFileWriter.deleteShapeFile(QString(outfilename)):
			QMessageBox.critical(qgis.mainWindow(), "Attribute Join",
					"Failure deleting existing shapefile: " + outfilename)
			return

	#print newfields

	outfile = QgsVectorFileWriter(QString(outfilename), QString("System"), \
			newfields, layer.dataProvider().geometryType(), layer.dataProvider().crs())

	if (outfile.hasError() != QgsVectorFileWriter.NoError):
		QMessageBox.critical(qgis.mainWindow(), "Attribute Join",
			"Failure creating output shapefile\n" + str(QString(outfilename)) + "\n" \
				+ str(outfile.errorMessage()))
		return

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
		qgis.mainWindow().statusBar().showMessage("Joining feature " + str(feature.id()) + \
				" of " + str(feature_count) + " (" + str(matched_count) + " matched)")
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
		QMessageBox.critical(qgis.mainWindow(), "Attribute Join", "Failure opening " + notfoundname)

	else:
		writer = csv.writer(outfile, dialect)
		writer.writerow(header)
		for x in range(0, len(csv_data)):
			if not csv_found[x]:
				writer.writerow(csv_data[x])
		del writer
		del outfile

	vlayer = qgis.addVectorLayer(outfilename, "joined", "ogr")

	qgis.mainWindow().statusBar().showMessage(str(matched_count) + " records joined from " + \
		str(feature_count) + " shape records and " + str(len(csv_data)) + " CSV file records")

	return matched_count


# --------------------------------------------------------
#    mmqgis_color_map - Robust layer coloring
# --------------------------------------------------------

def mmqgis_find_layer(layername):
	return QGisLayers.getObjectFromUri(layername)

def mmqgis_set_color_map(qgis, layername, bandname, lowvalue, midvalue, highvalue, steps, lowcolor, midcolor, highcolor):
	layer = mmqgis_find_layer(layername)
	if layer == None:
		QMessageBox.critical(qgis.mainWindow(), "Set Color Map", "Layer " + layername + " not found")
		return None

	# temp_filename = "/tmp/mmqgis.qml"
	# outfile = open(temp_filename, 'w')
	# if outfile == None:
	try:
		outfile, temp_filename = tempfile.mkstemp()
	except:
		QMessageBox.critical(qgis.mainWindow(), "Set Color", "Failure opening temporary style file")
		return None

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
		os.write(outfile, "<min>" + str(lowvalue) + "</min>")
		os.write(outfile, "<max>" + str(highvalue) + "</max>")
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
				red = str(int(round(lowred + ((midred - lowred) * interpolate))))
				green = str(int(round(lowgreen + ((midgreen - lowgreen) * interpolate))))
				blue = str(int(round(lowblue + ((midblue - lowblue) * interpolate))))
				os.write(outfile, "<colorRampEntry red=\"" + red + "\" blue=\"" + blue +
					"\" green=\"" + green + "\" value=\"" + str(value) + "\" label=\"\"/>\n")
			else:
				interpolate = (interpolate - 0.5) * 2.0
				value = midvalue + ((highvalue - midvalue) * interpolate)
				red = str(int(round(midred + ((highred - midred) * interpolate))))
				green = str(int(round(midgreen + ((highgreen - midgreen) * interpolate))))
				blue = str(int(round(midblue + ((highblue - midblue) * interpolate))))
				os.write(outfile, "<colorRampEntry red=\"" + red + "\" blue=\"" + blue +
					"\" green=\"" + green + "\" value=\"" + str(value) + "\" label=\"\"/>\n")

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
				red = str(int(round(lowred + ((midred - lowred) * interpolate))))
				green = str(int(round(lowgreen + ((midgreen - lowgreen) * interpolate))))
				blue = str(int(round(lowblue + ((midblue - lowblue) * interpolate))))
			else:
				interpolate = (interpolate - 0.5) * 2.0
				red = str(int(round(midred + ((highred - midred) * interpolate))))
				green = str(int(round(midgreen + ((highgreen - midgreen) * interpolate))))
				blue = str(int(round(midblue + ((highblue - midblue) * interpolate))))

			os.write(outfile, "<symbol>\n")
			os.write(outfile, "<lowervalue>" + str(values[x]) + "</lowervalue>\n")
			os.write(outfile, "<uppervalue>" + str(values[x + 1]) + "</uppervalue>\n")
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
			os.write(outfile, "<fillcolor red=\"" + str(red) + "\" blue=\"" +
					str(blue) + "\" green=\"" + str(green) + "\"/>")
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

	return

# --------------------------------------------------------------
#    mmqgis_geocode_google - Geocode CSV points from Google Maps
# --------------------------------------------------------------

def mmqgis_geocode_google(qgis, csvname, shapefilename, notfoundfile, keys):
	# Read the CSV file header
	try:
		infile = open(csvname, 'r')
	except:
		QMessageBox.critical(qgis.mainWindow(), "Geocode", "Failure opening " + csvname)
		return None

	dialect = csv.Sniffer().sniff(infile.read(1024))
	infile.seek(0)
	reader = csv.reader(infile, dialect)

	fields = {}
	indices = []
	header = reader.next()
	for x in range(0, len(header)):
		for y in range(0, len(keys)):
			if header[x] == keys[y]:
				indices.append(x)

		fieldname = header[x].strip()
		fields[len(fields)] = QgsField(fieldname[0:9], QVariant.String)

	if len(fields) <= 0:
		QMessageBox.critical(qgis.mainWindow(), "Geocode", "No valid fields in " + csvname)
		return None

	if len(indices) <= 0:
		QMessageBox.critical(qgis.mainWindow(), "Geocode", "No valid address fields in " + csvname)
		return None


	# Create the CSV file for ungeocoded records
	try:
		notfound = open(notfoundfile, 'w')
	except:
		QMessageBox.critical(qgis.mainWindow(), "CSV File", "Failure opening " + notfoundfile)
		return None

	notwriter = csv.writer(notfound, dialect)
	notwriter.writerow(header)


	# Create the output shapefile
	if QFile(shapefilename).exists():
		if not QgsVectorFileWriter.deleteShapeFile(QString(shapefilename)):
			QMessageBox.critical(qgis.mainWindow(), "Geocode", \
				"Failure deleting existing shapefile: " + savename)
			return None

	crs = QgsCoordinateReferenceSystem()
	crs.createFromSrid(4326)
	outfile = QgsVectorFileWriter(QString(shapefilename), QString("System"),
			fields, QGis.WKBPoint, crs)

	if (outfile.hasError() != QgsVectorFileWriter.NoError):
		QMessageBox.critical(qgis.mainWindow(), "Geocode", \
			"Failure creating output shapefile:" + str(outfile.hasError()));
		return None

	# Geocode and import

	recordcount = 0
	notfoundcount = 0
	for row in reader:
		time.sleep(0.5) # to avoid Google rate quota limits

		recordcount += 1
		qgis.mainWindow().statusBar().showMessage("Geocoding " + str(recordcount) +
			" (" + str(notfoundcount) + " not found)")

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

	if (recordcount > notfoundcount) and (recordcount > 0):
		vlayer = qgis.addVectorLayer(shapefilename, shapefilename, "ogr")

	qgis.mainWindow().statusBar().showMessage(str(recordcount - notfoundcount) + " of " + str(recordcount)
		+ " addresses geocoded with Google")

# --------------------------------------------------------
#    mmqgis_geometry_export_to_csv - Shape node dump to CSV
# --------------------------------------------------------

def mmqgis_geometry_export_to_csv(qgis, layername, node_filename, attribute_filename, field_delimiter, line_terminator):
	layer = mmqgis_find_layer(layername)

	if (layer == None) and (layer.type() != QgsMapLayer.VectorLayer):
		QMessageBox.critical(qgis.mainWindow(), "Geometry Export", "Invalid Vector Layer " + layername)
		return False;

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
		QMessageBox.critical(qgis.mainWindow(), "Geometry Export", "Failure opening " + node_filename)
		return False

	node_writer = csv.writer(nodefile, delimiter = field_delimiter, lineterminator = line_terminator)
	node_writer.writerow(node_header)

	if (layer.geometryType() != QGis.Point):
		try:
			attributefile = open(attribute_filename, 'w')
 	   	except:
			QMessageBox.critical(qgis.mainWindow(), "Geometry Export", "Failure opening " + attribute_filename)
			return False

		attribute_writer = csv.writer(attributefile, delimiter = field_delimiter, lineterminator = line_terminator)
		attribute_writer.writerow(attribute_header)


	# Iterate through each feature in the source layer
	feature_count = layer.dataProvider().featureCount()

	feature = QgsFeature()
	layer.dataProvider().select(layer.dataProvider().attributeIndexes())
	layer.dataProvider().rewind()
        while layer.dataProvider().nextFeature(feature):
		shapeid = str(feature.id()).strip()

		qgis.mainWindow().statusBar().showMessage("Exporting feature " + shapeid + " of " + str(feature_count))

		# if (feature.geometry().wkbType() == QGis.WKBPoint):
		if (feature.geometry().type() == QGis.Point):
			point = feature.geometry().asPoint()
			row = [ shapeid, str(point.x()), str(point.y()) ]
			for attindex, attribute in feature.attributeMap().iteritems():
				row.append(unicode(attribute.toString()).encode("iso-8859-1"))
			node_writer.writerow(row)

		# elif (feature.geometry().wkbType() == QGis.WKBLineString):
		elif (feature.geometry().type() == QGis.Line):
			polyline = feature.geometry().asPolyline()
			#for point in polyline.iteritems():
			for point in polyline:
				row = [ shapeid, str(point.x()), str(point.y()) ]
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
					row = [ shapeid, str(point.x()), str(point.y()) ]
					node_writer.writerow(row)

			row = [shapeid]
			for attindex, attribute in feature.attributeMap().iteritems():
				row.append(unicode(attribute.toString()).encode("iso-8859-1"))
			attribute_writer.writerow(row)

	del nodefile
	if (layer.geometryType() != QGis.Point):
		del attributefile

	qgis.mainWindow().statusBar().showMessage(str(feature_count) + " records exported")

	return feature_count


# --------------------------------------------------------
#    mmqgis_geometry_import_from_csv - Shape node import from CSV
# --------------------------------------------------------

def mmqgis_geometry_import_from_csv(qgis, node_filename, long_colname, lat_colname,
	shapeid_colname, geometry_type, shapefile_name):
	try:
		infile = open(node_filename, 'r')
	except:
		QMessageBox.critical(qgis.mainWindow(), "Import Geometry", "Failure opening " + node_filename)
		return False

	dialect = csv.Sniffer().sniff(infile.read(1024))
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
		QMessageBox.critical(qgis.mainWindow(), "Import Geometry",
			"Invalid latitude column name: " + lat_colname)
		return False

	if (long_col < 0):
		QMessageBox.critical(qgis.mainWindow(), "Import Geometry",
			"Invalid longitude column name: " + long_colname)
		return False

	if (shapeid_col < 0):
		QMessageBox.critical(qgis.mainWindow(), "Import Geometry",
			"Invalid shape ID column name: " + shapeid_colname)
		return False

	if (geometry_type == "Point"):
		wkb_type = QGis.WKBPoint

	elif (geometry_type == "Polyline"):
		wkb_type = QGis.WKBLineString

	elif (geometry_type == "Polygon"):
		wkb_type = QGis.WKBPolygon
	else:
		QMessageBox.critical(qgis.mainWindow(), "Import Geometry", "Invalid geometry type: " + geometry_type)
		return False

	# Create the output shapefile
	if QFile(shapefile_name).exists():
		if not QgsVectorFileWriter.deleteShapeFile(QString(shapefile_name)):
			QMessageBox.critical(qgis.mainWindow(), "Geocode", \
				"Failure deleting existing shapefile: " + savename)
			return None

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
		QMessageBox.critical(qgis.mainWindow(), "Geocode", \
			"Failure creating output shapefile " + shapefile_name + ": " + str(outfile.hasError()));
		return None

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
				and is_float(row[long_col]) and is_float(row[lat_col]):
			node_count += 1
			qgis.mainWindow().statusBar().showMessage("Importing node " + str(node_count))
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

	qgis.addVectorLayer(shapefile_name, os.path.basename(shapefile_name), "ogr")

	qgis.mainWindow().statusBar().showMessage("Loaded " + str(shape_count) + " shapes (" + str(node_count) + " nodes")


# --------------------------------------------------------
#    mmqgis_grid - Grid shapefile creation
# --------------------------------------------------------

def mmqgis_grid(qgis, savename, hspacing, vspacing, width, height, originx, originy, gridtype):
	if savename == "":
		QMessageBox.critical(qgis.mainWindow(), "Grid", "No output filename given")
		return None

	if (hspacing <= 0) or (vspacing <= 0):
		QMessageBox.critical(qgis.mainWindow(), "Grid", "Invalid grid spacing: "
			+ str(hspacing) + " / " + str(vspacing))
		return None

	if (width <= hspacing) or (width < vspacing):
		QMessageBox.critical(qgis.mainWindow(), "Grid", "Invalid width / height: "
			+ str(width) + " / " + str(height))
		return None

	fields = {
		0 : QgsField("longitude", QVariant.Double, "real", 24, 16, "Longitude"),
		1 : QgsField("latitude", QVariant.Double, "real", 24, 16, "Latitude") }

	if QFile(savename).exists():
		if not QgsVectorFileWriter.deleteShapeFile(QString(savename)):
			QMessageBox.critical(qgis.mainWindow(), "Grid",
				"Failure deleting existing shapefile: " + savename)
			return None


	if gridtype.find("polygon") >= 0:
		shapetype = QGis.WKBPolygon
	else:
		shapetype = QGis.WKBLineString

	# print gridtype + "," + str(shapetype)

	outfile = QgsVectorFileWriter(QString(savename), QString("System"), fields,
		shapetype, QgsCoordinateReferenceSystem());

	if (outfile.hasError() != QgsVectorFileWriter.NoError):
		QMessageBox.critical(qgis.mainWindow(), "Grid",
			"Failure creating output shapefile:" + str(outfile.hasError()));
		return None

	linecount = 0
	if gridtype == "Rectangle (line)":
		x = originx
		while x < (originx + width):
			polyline = []
			geometry = QgsGeometry()
			feature = QgsFeature()

			y = originy
			while y < (originy + height):
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
		while y < (originy + height):
			polyline = []
			geometry = QgsGeometry()
			feature = QgsFeature()

			x = originx
			while x < (originx + width):
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

	vlayer = qgis.addVectorLayer(savename, "grid", "ogr")

	qgis.mainWindow().statusBar().showMessage(str(linecount) + " feature grid shapefile created")

# --------------------------------------------------------
#    mmqgis_gridify - Snap shape verticies to grid
# --------------------------------------------------------

def mmqgis_gridify_layer(qgis, layername, hspacing, vspacing, savename, add_to_project):
	layer = mmqgis_find_layer(layername)

	if not layer:
		QMessageBox.critical(qgis.mainWindow(), "Gridify Layer", \
			"Project has no active vector layer to gridify")
		return

	if QFile(savename).exists():
		if not QgsVectorFileWriter.deleteShapeFile(QString(savename)):
			QMessageBox.critical(qgis.mainWindow(), "Grid",
				"Failure deleting existing shapefile: " + savename)
			return

	outfile = QgsVectorFileWriter(QString(savename), QString("System"), layer.dataProvider().fields(),
			layer.dataProvider().geometryType(), layer.dataProvider().crs())

	if (outfile.hasError() != QgsVectorFileWriter.NoError):
		QMessageBox.critical(qgis.mainWindow(), "Grid",
			"Failure creating output shapefile:" + str(outfile.hasError()));
		return

	point_count = 0
	deleted_points = 0

	feature_number = 0
	feature_count = layer.dataProvider().featureCount()
	layer.dataProvider().select(layer.dataProvider().attributeIndexes())
	layer.dataProvider().rewind()

	feature = QgsFeature()
        while layer.dataProvider().nextFeature(feature):
		qgis.mainWindow().statusBar().showMessage("Gridifying feature " + str(feature_number + 1))

		geometry = feature.geometry()

		if geometry.wkbType() == QGis.WKBPoint:
			points, added, deleted = mmqgis_gridify_points(hspacing, vspacing, [geometry.asPoint()])
			geometry = geometry.fromPoint(points[0])
			point_count += added
			deleted_points += deleted

		elif geometry.wkbType() == QGis.WKBLineString:
			#print "LineString"
			polyline, added, deleted = mmqgis_gridify_points(hspacing, vspacing, geometry.asPolyline())
			if len(polyline) < 2:
				geometry = None
			else:
				geometry = geometry.fromPolyline(polyline)
			point_count += added
			deleted_points += deleted

		elif geometry.wkbType() == QGis.WKBPolygon:
			newpolygon = []
			for polyline in geometry.asPolygon():
				newpolyline, added, deleted = mmqgis_gridify_points(hspacing, vspacing, polyline)
				point_count += added
				deleted_points += deleted

				if len(newpolyline) > 1:
					newpolygon.append(newpolyline)

			if len(newpolygon) <= 0:
				geometry = None
			else:
				geometry = geometry.fromPolygon(newpolygon)

		elif geometry.wkbType() == QGis.WKBMultiPoint:
			multipoints, added, deleted = mmqgis_gridify_points(hspacing, vspacing, geometry.asMultiPoint())
			geometry = geometry.fromMultiPoint(multipoints)
			point_count += added
			deleted_points += deleted

		elif geometry.wkbType() == QGis.WKBMultiLineString:
			#print "MultiLineString"
			newmultipolyline = []
			for polyline in geometry.asMultiPolyline():
				newpolyline, added, deleted = mmqgis_gridify_points(hspacing, vspacing, polyline)
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
					newpolyline, added, deleted = mmqgis_gridify_points(hspacing, vspacing, polyline)

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
			QMessageBox.critical(qgis.mainWindow(), "Gridify Layer", \
				"Unknown geometry type " + str(geometry.wkbType()) + \
				" on feature " + str(feature_number + 1))
			return

		# print "Closing feature"

		if geometry != None:
			out_feature = QgsFeature()
			out_feature.setGeometry(geometry)
			out_feature.setAttributeMap(feature.attributeMap())
			outfile.addFeature(out_feature)

		feature_number += 1

	del outfile

	if add_to_project:
		vlayer = qgis.addVectorLayer(savename, "gridified", "ogr")

	qgis.mainWindow().statusBar().showMessage("Gridified shapefile created (" + \
		str(deleted_points) + " of " + str(point_count) + " points deleted)")


def mmqgis_gridify_points(hspacing, vspacing, points):
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


# --------------------------------------------------------
#    mmqgis_hub_distance - Create shapefile of distances
#			   from points to nearest hub
# --------------------------------------------------------

class mmqgis_hub:
	def __init__(self, newx, newy, newname):
		self.x = newx
		self.y = newy
		self.name = newname


def mmqgis_hub_distance(qgis, sourcename, destname, nameattributename, addlines, addlayer, savename):

	# Source layer
	sourcelayer = mmqgis_find_layer(sourcename)
	if sourcelayer == None:
		QMessageBox.critical(qgis.mainWindow(), "Hub Distance", "Origin Layer " + sourcename + " not found")
		return None

	# Destination layer
	hubslayer = mmqgis_find_layer(destname)
	if (hubslayer == None) or (hubslayer.featureCount() <= 0):
		QMessageBox.critical(qgis.mainWindow(), "Hub Distance", "Hub layer " + destname + " not found")
		return

	# Name attributes
	nameindex = hubslayer.dataProvider().fieldNameIndex(nameattributename)
	if nameindex < 0:
		QMessageBox.critical(qgis.mainWindow(), "Hub Distance", \
				"Invalid name attribute: " + nameattributename)

	outputtype = QGis.WKBPoint
	if addlines:
		outputtype = QGis.WKBLineString

	# Create output file
	if savename == "":
		QMessageBox.critical(qgis.mainWindow(), "Hub Distance", "No output filename given")
		return

	if QFile(savename).exists():
		if not QgsVectorFileWriter.deleteShapeFile(QString(savename)):
			QMessageBox.critical(qgis.mainWindow(), "Sort",
				"Failure deleting existing shapefile: " + savename)
			return

	outfields = sourcelayer.dataProvider().fields()
	outfields[len(outfields)] = QgsField(QString("HubName"), QVariant.String)
	outfields[len(outfields)] = QgsField(QString("HubDist"), QVariant.Double)

	outfile = QgsVectorFileWriter(QString(savename), QString("System"), outfields, \
		outputtype, sourcelayer.dataProvider().crs())

	if (outfile.hasError() != QgsVectorFileWriter.NoError):
		QMessageBox.critical(qgis.mainWindow(), "Sort",
			"Failure creating output shapefile:" + str(outfile.hasError()));
		return


	# Create array of hubs
	hubs = []
	feature = QgsFeature()
	hubslayer.dataProvider().select(hubslayer.dataProvider().attributeIndexes())
	hubslayer.dataProvider().rewind()
	while hubslayer.dataProvider().nextFeature(feature):
		qgis.mainWindow().statusBar().showMessage("Reading hub " + str(feature.id()))
		hub = mmqgis_hub(feature.geometry().boundingBox().center().x(), \
				feature.geometry().boundingBox().center().y(), \
				feature.attributeMap()[nameindex].toString())
		hubs.append(hub)

	del hubslayer

	# Scan source points, find nearest hub, and write to output file
	writecount = 0
	feature = QgsFeature()
	sourcelayer.dataProvider().select(sourcelayer.dataProvider().attributeIndexes())
	sourcelayer.dataProvider().rewind()
	while sourcelayer.dataProvider().nextFeature(feature):
		sourcex = feature.geometry().boundingBox().center().x()
		sourcey = feature.geometry().boundingBox().center().y()

		hubx = hubs[0].x
		huby = hubs[0].y
		hubname = hubs[0].name
		hubdist = sqrt(pow(sourcex - hubx, 2.0) + pow(sourcey - huby, 2.0))
		for hub in hubs:
			#print "Scanning hub: " + str(hub.x) + "," + str(hub.y) + " " + hub.name
			thisdist = sqrt(pow(sourcex - hub.x, 2.0) + pow(sourcey - hub.y, 2.0))
			if thisdist < hubdist:
				hubx = hub.x
				huby = hub.y
				hubdist = thisdist
				hubname = hub.name

		attributes = feature.attributeMap()
		attributes[len(attributes)] = QVariant(hubname)
		attributes[len(attributes)] = QVariant(hubdist)

		outfeature = QgsFeature()
		outfeature.setAttributeMap(attributes)

		if outputtype == QGis.WKBPoint:
			geometry = QgsGeometry()
			outfeature.setGeometry(geometry.fromPoint(QgsPoint(sourcex, sourcey)))
		else:
			polyline = []
			polyline.append(QgsPoint(sourcex, sourcey))
			polyline.append(QgsPoint(hubx, huby))
			geometry = QgsGeometry()
			outfeature.setGeometry(geometry.fromPolyline(polyline))

		outfile.addFeature(outfeature)

		writecount += 1
		qgis.mainWindow().statusBar().showMessage("Writing feature " + str(writecount) +\
			" of " + str(sourcelayer.dataProvider().featureCount()))

	del outfile

	if addlayer:
		vlayer = qgis.addVectorLayer(savename, "hubdistance", "ogr")

	qgis.mainWindow().statusBar().showMessage("Hub distance file created from " + sourcename)


# --------------------------------------------------------
#    mmqgis_label - Create single label points for
#		    single- or multi-feature items
# --------------------------------------------------------

class mmqgis_label():
	def __init__(self, name, attributemap):
		self.name = name
		self.xsum = 0
		self.ysum = 0
		self.feature_count = 0
		self.attributes = attributemap

def mmqgis_label_point(qgis, layername, labelattributename, savename):
	layer = mmqgis_find_layer(layername)
	if layer == None:
		QMessageBox.critical(qgis.mainWindow(), "Label Layer", "Invalid layer name " . layername)
		return None

	labelindex = layer.dataProvider().fieldNameIndex(labelattributename)
	if labelindex < 0:
		QMessageBox.critical(qgis.mainWindow(), "Label Layer", \
			"Invalid label field name: " + labelattributename)
		return None

	# print  "labelindex = " + str(labelindex)

	if savename == "":
		QMessageBox.critical(qgis.mainWindow(), "Label Layer", "No output filename given")
		return None

	# Open file (delete any existing)
	if QFile(savename).exists():
		if not QgsVectorFileWriter.deleteShapeFile(QString(savename)):
			QMessageBox.critical(qgis.mainWindow(), "Label Layer",
				"Failure deleting existing shapefile: " + savename)
			return None

	outfile = QgsVectorFileWriter(QString(savename), QString("System"), layer.dataProvider().fields(),
				QGis.WKBPoint, layer.dataProvider().crs())

	if (outfile.hasError() != QgsVectorFileWriter.NoError):
		QMessageBox.critical(qgis.mainWindow(), "Label",
			"Failure creating output shapefile:" + str(outfile.hasError()));
		return None

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
			features[key] = mmqgis_label(key, feature.attributeMap())

		center = feature.geometry().boundingBox().center();
		features[key].xsum += center.x()
		features[key].ysum += center.y()
		features[key].feature_count += 1

		readcount += 1
		if not (readcount % 10):
			qgis.mainWindow().statusBar().showMessage( \
				"Reading feature " + str(readcount) + " of " + str(feature_count))



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
			QMessageBox.critical(qgis.mainWindow(), "Label Layer", \
				"Failure writing feature to shapefile")
			break

		writecount += 1
		if not (writecount % 10):
			qgis.mainWindow().statusBar().showMessage( \
				"Writing feature " + str(writecount) + " of " + str(len(features)))

	del outfile

	layer = qgis.addVectorLayer(savename, "labels", "ogr")
	# layer.enableLabels()

	qgis.mainWindow().statusBar().showMessage(str(writecount) + " label shapefile created from " + layername)


# --------------------------------------------------------
#    mmqgis_merge - Merge layers to single shapefile
# --------------------------------------------------------

def mmqgis_merge(qgis, layernames, savename):
	fields = {}
	layers = []
	totalfeaturecount = 0

	for x in range(0, len(layernames)):
		layername = layernames[x]
		layer = mmqgis_find_layer(layername)
		if layer == None:
			QMessageBox.critical(qgis.mainWindow(), "Merge Layers", "Layer " + layername + " not found")
			return None

		# Verify that all layers are the same type (point, polygon, etc)
		if (len(layers) > 0):
			if (layer.dataProvider().geometryType() != layers[0].dataProvider().geometryType()):
				QMessageBox.critical(qgis.mainWindow(),
					"Merge Layers", "Merged layers must all be same type of geometry (" +
					unicode(layer.dataProvider().geometryType()) + " != " +
					unicode(layers[0].dataProvider().geometryType()) + ")")
				return None

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
		QMessageBox.critical(qgis.mainWindow(), "Merge Layers", "No layers given to merge")
		return

	# Create the output shapefile
	if savename == "":
		QMessageBox.critical(qgis.mainWindow(), "Merge Layers", "No output filename given")
		return

	if QFile(savename).exists():
		if not QgsVectorFileWriter.deleteShapeFile(QString(savename)):
			QMessageBox.critical(qgis.mainWindow(), "Merge Layers",
				"Failure deleting existing shapefile: " + savename)
			return None

	outfile = QgsVectorFileWriter(QString(savename), QString("System"), fields,
		layers[0].dataProvider().geometryType(), layers[0].dataProvider().crs())

	if (outfile.hasError() != QgsVectorFileWriter.NoError):
		QMessageBox.critical(qgis.mainWindow(), "Merge Layers",
			"Failure creating output shapefile:" + unicode(outfile.hasError()));
		return None

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
				str(featurecount) + " of " + str(totalfeaturecount))

	del outfile

	# Add the merged layer to the project
	qgis.addVectorLayer(savename, "merged", "ogr")

	qgis.mainWindow().statusBar().showMessage(str(featurecount) + " records exported")


# ----------------------------------------------------------
#    mmqgis_select - Select features by attribute
# ----------------------------------------------------------

def mmqgis_select(qgis, layername, selectattributename, comparisonvalue, comparisonname, savename, addlayer):
	layer = mmqgis_find_layer(layername)
	if layer == None:
		QMessageBox.critical(qgis.mainWindow(), "Select Layer", \
			"Project has no active vector layer to select from")
		return None

	selectindex = layer.dataProvider().fieldNameIndex(selectattributename)
	if selectindex < 0:
		QMessageBox.critical(qgis.mainWindow(), "Select", \
			"Invalid select field name: " + selectattributename)
		return None

	# print  "selectindex = " + str(selectindex) + " " + comparisonname + " " + comparisonvalue

	if comparisonvalue == "":
		QMessageBox.critical(qgis.mainWindow(), "Select", "No comparison value given")
		return None

	if savename == "":
		QMessageBox.critical(qgis.mainWindow(), "Select", "No output filename given")
		return None

	if QFile(savename).exists():
		if not QgsVectorFileWriter.deleteShapeFile(QString(savename)):
			QMessageBox.critical(qgis.mainWindow(), "Select",
				"Failure deleting existing shapefile: " + savename)
			return

	outfile = QgsVectorFileWriter(QString(savename), QString("System"), layer.dataProvider().fields(),
			layer.dataProvider().geometryType(), layer.dataProvider().crs())

	if (outfile.hasError() != QgsVectorFileWriter.NoError):
		QMessageBox.critical(qgis.mainWindow(), "Select",
			"Failure creating output shapefile:" + str(outfile.hasError()));
		return

	readcount = 0
	writecount = 0
	feature = QgsFeature()
	layer.dataProvider().select(layer.dataProvider().attributeIndexes())
	layer.dataProvider().rewind()

	while layer.dataProvider().nextFeature(feature):
		if (comparisonname == 'begins with') or (comparisonname == 'contains') or \
		   (feature.attributeMap()[selectindex].type() == QVariant.String):
			x = str(feature.attributeMap()[selectindex].toString())
			y = comparisonvalue
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
			str(readcount) + " of " + str(layer.dataProvider().featureCount()) + \
			"(" + str(writecount) + " selected)")

	del outfile

	if addlayer:
		vlayer = qgis.addVectorLayer(savename, savename, "ogr")

	qgis.mainWindow().statusBar().showMessage("Selected " + str(writecount) + " features to " + savename)


# --------------------------------------------------------
#    mmqgis_sort - Sort shapefile by attribute
# --------------------------------------------------------

def mmqgis_sort(qgis, layername, sortattributename, savename, direction):
	layer = mmqgis_find_layer(layername)
	if layer == None:
		QMessageBox.critical(qgis.mainWindow(), "Sort Layer", \
			"Project has no active vector layer to sort")
		return None

	#sortindex = -1
	#sortattributename = self.sortattribute.currentText()
	#for index, fieldname in layer.dataProvider().fields().iteritems():
	#	if fieldname.name() == sortattributename:
	#		sortindex = index

	sortindex = layer.dataProvider().fieldNameIndex(sortattributename)
	if sortindex < 0:
		QMessageBox.critical(qgis.mainWindow(), "Sort", \
			"Invalid sort field name: " + sortattributename)

	# print  "sortindex = " + str(sortindex)

	if savename == "":
		QMessageBox.critical(qgis.mainWindow(), "Sort", "No output filename given")
		return None

	if QFile(savename).exists():
		if not QgsVectorFileWriter.deleteShapeFile(QString(savename)):
			QMessageBox.critical(qgis.mainWindow(), "Sort",
				"Failure deleting existing shapefile: " + savename)
			return None

	outfile = QgsVectorFileWriter(QString(savename), QString("System"), layer.dataProvider().fields(),
			layer.dataProvider().geometryType(), layer.dataProvider().crs())

	if (outfile.hasError() != QgsVectorFileWriter.NoError):
		QMessageBox.critical(qgis.mainWindow(), "Sort",
			"Failure creating output shapefile:" + str(outfile.hasError()));
		return None

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
			record = feature.id(), str(feature.attributeMap()[sortindex].toString())

		qgis.mainWindow().statusBar().showMessage("Reading feature " + str(feature.id()))
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
		qgis.mainWindow().statusBar().showMessage("Writing feature " + str(writecount) +\
			" of " + str(len(table)))

	del outfile

	vlayer = qgis.addVectorLayer(savename, "sorted", "ogr")

	qgis.mainWindow().statusBar().showMessage("Sorted shapefile created from " + layername)


# --------------------------------------------------------
#    mmqgis_geocode_street_layer - Geocode addresses from street
#			     address finder shapefile
# --------------------------------------------------------

def mmqgis_geocode_street_layer(qgis, layername, csvname, addressfield, shapefilename, streetname,
fromx, fromy, tox, toy, leftfrom, rightfrom, leftto, rightto, setback, notfoundfile, addlayer):

	layer = mmqgis_find_layer(layername)
	if layer == None:
		QMessageBox.critical(qgis.mainWindow(), "Address Geocode", "Address layer not found")
		return None

	if len(csvname) <= 0:
		QMessageBox.critical(qgis.mainWindow(), "Address Geocode", "No input CSV file given")
		return None

	# Read the CSV file data into memory
	try:
		infile = open(csvname, 'r')
	except:
		QMessageBox.critical(qgis.mainWindow(), "CSV File", "Failure opening " + csvname)
		return None

	dialect = csv.Sniffer().sniff(infile.read(1024))
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
		address = mmqgis_searchable_streetname(row[addressfield_index])
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
			QMessageBox.critical(qgis.mainWindow(), "Address Geocode", \
				"Failure deleting existing shapefile: " + savename)
			return

	outfile = QgsVectorFileWriter(QString(shapefilename), QString("System"), \
		fields, QGis.WKBPoint, layer.dataProvider().crs())

	if (outfile.hasError() != QgsVectorFileWriter.NoError):
		QMessageBox.critical(qgis.mainWindow(), "Address Geocode", \
			"Failure creating output shapefile:" + str(outfile.hasError()));
		return

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
				str(feature.id()) + " of " + str(feature_count) + \
				" (" + str(matched_count) + " matched)")

		attributes = feature.attributeMap()
		key = mmqgis_searchable_streetname(str(attributes[streetname_attribute].toString()))

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
		QMessageBox.critical(qgis.mainWindow(), "CSV File", "Failure opening " + notfoundfile)
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
		vlayer = qgis.addVectorLayer(shapefilename, "addresses", "ogr")

	qgis.mainWindow().statusBar().showMessage(str(matched_count) + " of " + str(len(csv_attributes)) \
		+ " addresses geocoded from " + str(feature_count) + " street records")


def mmqgis_searchable_streetname(name):
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
#    mmqgis_text_to_float - Change text fields to numbers
# ---------------------------------------------------------

def mmqgis_text_to_float(qgis, layername, attributes, savename, addlayer):
	layer = mmqgis_find_layer(layername)
	if layer == None:
		QMessageBox.critical(qgis.mainWindow(), "Text to Float", \
			"Project has no active vector layer to convert")
		return None

	if savename == "":
		QMessageBox.critical(qgis.mainWindow(), "Text to Float", "No output filename given")
		return None

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
		QMessageBox.critical(qgis.mainWindow(), "Text to Float",
			"No string or integer fields selected for conversion to floating point")
		return None


	# Create the output file
	if QFile(savename).exists():
		if not QgsVectorFileWriter.deleteShapeFile(QString(savename)):
			QMessageBox.critical(qgis.mainWindow(), "Text to Float",
				"Failure deleting existing shapefile: " + savename)
			return

	outfile = QgsVectorFileWriter(QString(savename), QString("System"), destfields,
			layer.dataProvider().geometryType(), layer.dataProvider().crs())

	if (outfile.hasError() != QgsVectorFileWriter.NoError):
		QMessageBox.critical(qgis.mainWindow(), "Text to Float",
			"Failure creating output shapefile:" + str(outfile.hasError()));
		return


	# Write the features with modified attributes
	feature = QgsFeature()
	featurecount = layer.dataProvider().featureCount();
	layer.dataProvider().select(layer.dataProvider().attributeIndexes())
	layer.dataProvider().rewind()
	while layer.dataProvider().nextFeature(feature):
		qgis.mainWindow().statusBar().showMessage("Writing feature " + \
			str(feature.id()) + " of " + str(featurecount))

		attributes = feature.attributeMap()
		for index, field in layer.dataProvider().fields().iteritems():
			if (field.type() != destfields[index].type()):
				#print str(index) + ") " + str(destfields[index].typeName()) + \
				#	": " + str(attributes[index].toString())
				string = str(attributes[index].toString())
				multiplier = 1.0
				if (string.find("%") >= 0):
					multiplier = 1 / 100.0
					string = string.replace("%", "")
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
		vlayer = qgis.addVectorLayer(savename, "numeric", "ogr")

	qgis.mainWindow().statusBar().showMessage(str(changecount) + " text converted to numeric")



# --------------------------------------------------------
#    mmqgis_voronoi - Voronoi diagram creation
# --------------------------------------------------------

def mmqgis_voronoi_diagram(qgis, sourcelayer, savename, addtoproject):
	layer = mmqgis_find_layer(sourcelayer)
	if layer == None:
		QMessageBox.critical(qgis.mainWindow(), "Voronoi", "Layer " + sourcename + " not found")
		return None

	if QFile(savename).exists():
		if not QgsVectorFileWriter.deleteShapeFile(QString(savename)):
			QMessageBox.critical(qgis.mainWindow(), "Voronoi Diagram",
				"Failure deleting existing shapefile: " + savename)
			return

	outfile = QgsVectorFileWriter(QString(savename), QString("System"), layer.dataProvider().fields(), \
			QGis.WKBPolygon, layer.dataProvider().crs())

	if (outfile.hasError() != QgsVectorFileWriter.NoError):
		QMessageBox.critical(qgis.mainWindow(), "Voronoi Diagram",
			"Failure creating output shapefile:" + str(outfile.hasError()));

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
		qgis.mainWindow().statusBar().showMessage("Reading feature " + str(feature.id()))
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
		QMessageBox.critical(qgis.mainWindow(), "Voronoi", "Too few points to create diagram")
		return

	for center in points:
	# for center in [ points[17] ]:
		# print "\nCenter, " + str(center[0]) + ", " + str(center[1])
		qgis.mainWindow().statusBar().showMessage("Processing point " + \
			str(center[0]) + ", " + str(center[1]))

		# Borders are tangents to midpoints between all neighbors
		tangents = []
		for neighbor in points:
			border = mmqgis_voronoi_line((center[0] + neighbor[0]) / 2.0, (center[1] + neighbor[1]) / 2.0)
			if ((neighbor[0] != center[0]) or (neighbor[1] != center[1])):
				tangents.append(border)

		# Add edge intersections to clip to extent of points
		offset = (xmax - xmin) * 0.01
		tangents.append(mmqgis_voronoi_line(xmax + offset, center[1]))
		tangents.append(mmqgis_voronoi_line(center[0], ymax + offset))
		tangents.append(mmqgis_voronoi_line(xmin - offset, center[1]))
		tangents.append(mmqgis_voronoi_line(center[0], ymin - offset))
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
		border = mmqgis_voronoi_line(tangents[closest].x, tangents[closest].y)
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
				border = mmqgis_voronoi_line(tangents[next].x, tangents[next].y)
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

	if addtoproject:
		vlayer = qgis.addVectorLayer(savename, "voronoi", "ogr")

	qgis.mainWindow().statusBar().showMessage("Created " + str(len(points)) + " polygon Voronoi diagram")


class mmqgis_voronoi_line:
	def __init__(self, x, y):
		self.x = x
		self.y = y
		self.angle = 0
		self.distance = 0

	def list(self, title):
		print title + ", " + str(self.x) + ", " + str(self.y) + \
			", angle " + str(self.angle * 180 / pi) + ", distance " + str(self.distance)

	def angleval(self):
		return self.angle


# --------------------------------------------------------
#    mmqgis_layer_attribute_bounds()
#
# This function is needed because the
# QgsVectorDataProvider::minimumValue() and maximumValue()
# do not work as of QGIS v1.5.0
# --------------------------------------------------------

def mmqgis_layer_attribute_bounds(layer, attribute_name):
	attribute_index = -1
	for index, field in layer.dataProvider().fields().iteritems():
		if str(field.name()) == attribute_name:
			attribute_index = index

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

# --------------------------------------------------------
#    mmqgis_read_csv_header()
#
# Reads the header (column names) from a CSV file
# --------------------------------------------------------

def mmqgis_read_csv_header(qgis, filename):
	try:
		infile = open(filename, 'r')
	except:
		QMessageBox.information(qgis.mainWindow(), "Input CSV File", "Failure opening " + filename)
		return None

	try:
		dialect = csv.Sniffer().sniff(infile.read(1024))
	except:
		QMessageBox.information(qgis.mainWindow(), "Input CSV File",
			"Bad CSV file - verify that your delimiters are consistent");
		return None

	infile.seek(0)
	reader = csv.reader(infile, dialect)

	header = reader.next()
	del reader
	del infile

	if len(header) <= 0:
		QMessageBox.information(qgis.mainWindow(), "Input CSV File",
			filename + " does not appear to be a CSV file")
		return None

	return header

def is_float(s):
	try:
		float(s)
		return True
	except:
		return False

