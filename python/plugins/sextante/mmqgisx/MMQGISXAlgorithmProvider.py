# --------------------------------------------------------
#    MMQGISX - MMQGIS Wrapper for Sextante
#
#    begin                : 18 May 2010
#    copyright            : (c) 2012 by Michael Minn
#    email                : See michaelminn.com
#
#   MMQGIS is free software and is offered without guarantee
#   or warranty. You can redistribute it and/or modify it 
#   under the terms of version 2 of the GNU General Public 
#   License (GPL v2) as published by the Free Software 
#   Foundation (www.gnu.org).
# --------------------------------------------------------

import os
from PyQt4 import QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from sextante.core.AlgorithmProvider import AlgorithmProvider
from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from sextante.core.SextanteLog import SextanteLog
from sextante.core.QGisLayers import QGisLayers
from sextante.parameters.ParameterFile import ParameterFile
from sextante.parameters.ParameterNumber import ParameterNumber
from sextante.parameters.ParameterSelection import ParameterSelection
from sextante.parameters.ParameterString import ParameterString
from sextante.parameters.ParameterTable import ParameterTable
from sextante.parameters.ParameterTableField import ParameterTableField
from sextante.parameters.ParameterVector import ParameterVector
from sextante.outputs.OutputTable import OutputTable
from sextante.outputs.OutputVector import OutputVector
from sextante.mmqgisx.mmqgisx_library import *

class MMQGISXAlgorithmProvider(AlgorithmProvider):

	def __init__(self):
		AlgorithmProvider.__init__(self)
		self.alglist = [ mmqgisx_attribute_export_dialog(), 
			mmqgisx_attribute_join_dialog(), 
			mmqgisx_delete_columns_dialog(), 
			mmqgisx_delete_duplicate_geometries_dialog(),
			mmqgisx_geocode_google_dialog(),
			mmqgisx_geometry_convert_dialog(),
			mmqgisx_geometry_export_dialog(),
			mmqgisx_geometry_import_dialog(),
			mmqgisx_grid_dialog(),
			mmqgisx_gridify_dialog(),
			mmqgisx_hub_distance_dialog(),
			mmqgisx_hub_lines_dialog(),
			mmqgisx_label_point_dialog(),
			mmqgisx_merge_dialog(),
			mmqgisx_select_dialog(),
			mmqgisx_sort_dialog(),
			mmqgisx_text_to_float_dialog(),
			mmqgisx_voronoi_dialog() ]

	def getDescription(self):
		return "MMQGISX (Vector and table tools)"

	def getName(self):
		return "mmqgisx"

	def getIcon(self):
		return  QtGui.QIcon(os.path.dirname(__file__) + "/icons/mmqgis.png")

	def _loadAlgorithms(self):
		self.algs = self.alglist

	def getSupportedOutputTableExtensions(self):
		return ["csv"]


class mmqgisx_attribute_export_dialog(GeoAlgorithm):

	OUTFILENAME = "OUTFILENAME"
	LAYERNAME = "LAYERNAME"
	FIELDDELIMITER = "FIELDDELIMITER"
	LINETERMINATOR = "LINETERMINATOR"

	def defineCharacteristics(self):
		self.name = "Attribute Export"
		self.group = "Transfer"

		self.addParameter(ParameterVector(self.LAYERNAME, "SourceLayer", ParameterVector.VECTOR_TYPE_ANY))

		self.delimiters = ["Comma", "Bar", "Space"]
		self.addParameter(ParameterSelection(self.FIELDDELIMITER, "Delimiter", self.delimiters, default = 0))

		self.terminators = ["CRLF", "LF"]
		self.addParameter(ParameterSelection(self.LINETERMINATOR, "Delimiter", self.terminators, default = 0))

		self.addOutput(OutputTable(self.OUTFILENAME, "Output CSV File"))

	def getIcon(self):
		return  QtGui.QIcon(os.path.dirname(__file__) + "/icons/mmqgis_attribute_export.png")

	def processAlgorithm(self, progress):
		# Include must be done here to avoid cyclic import
		from sextante.core.Sextante import Sextante
		qgis = Sextante.getInterface()

		outfilename = self.getOutputValue(self.OUTFILENAME)
		layer = QGisLayers.getObjectFromUri(self.getParameterValue(self.LAYERNAME))
		layername = layer.name()

		if self.getParameterValue(self.FIELDDELIMITER) == 1:
			field_delimiter = "|"
		elif self.getParameterValue(self.FIELDDELIMITER) == 2:
			field_delimiter = " "
		else:
			field_delimiter = ","

		if self.getParameterValue(self.LINETERMINATOR) == 1:
			line_terminator = "\n"
		else:
			line_terminator = "\r\n"

		message = mmqgisx_attribute_export(qgis, outfilename, layername, None, 
			field_delimiter, line_terminator)

		if message:
			raise GeoAlgorithmExecutionException(message)

class mmqgisx_attribute_join_dialog(GeoAlgorithm):

	LAYERNAME = "LAYERNAME"
	INFILENAME = "INFILENAME"
	JOINFIELD = "JOINFIELD"
	JOINATTRIBUTE = "JOINATTRIBUTE"
	OUTFILENAME = "OUTFILENAME"
	NOTFOUNDNAME = "NOTFOUNDNAME"

	def defineCharacteristics(self):
		self.name = "Attribute Join"
		self.group = "Transfer"

		self.addParameter(ParameterTable(self.INFILENAME, "Input CSV File", False))
		self.addParameter(ParameterTableField(self.JOINFIELD, "CSV File Field", self.INFILENAME))
		
		self.addParameter(ParameterVector(self.LAYERNAME, "Join Layer", ParameterVector.VECTOR_TYPE_ANY))
		self.addParameter(ParameterTableField(self.JOINATTRIBUTE,
			"Join Layer Attribute", mmqgisx_attribute_join_dialog.LAYERNAME))

		self.addOutput(OutputVector(self.OUTFILENAME, "Output Shapefile"))
		self.addOutput(OutputTable(self.NOTFOUNDNAME, "Not Found CSV Output List"))

	def getIcon(self):
		return  QtGui.QIcon(os.path.dirname(__file__) + "/icons/mmqgis_attribute_join.png")

	def processAlgorithm(self, progress):
		# Include must be done here to avoid cyclic import
		from sextante.core.Sextante import Sextante
		qgis = Sextante.getInterface()

		layer = QGisLayers.getObjectFromUri(self.getParameterValue(self.LAYERNAME))
		layername = layer.name()

		table = QGisLayers.getObjectFromUri(self.getParameterValue(self.INFILENAME))
		infilename = table.name()

		joinfield = self.getParameterValue(self.JOINFIELD)
		joinattribute = self.getParameterValue(self.JOINATTRIBUTE)

		outfilename = self.getOutputValue(self.OUTFILENAME)
		notfoundname = self.getOutputValue(self.NOTFOUNDNAME)

		message = mmqgisx_attribute_join(qgis, layername, infilename, joinfield, 
			joinattribute, outfilename, notfoundname, False)

		if message:
			raise GeoAlgorithmExecutionException(message)

class mmqgisx_delete_columns_dialog(GeoAlgorithm):

	LAYERNAME = "LAYERNAME"
	COLUMN = "COLUMN"
	SAVENAME = "SAVENAME"

	def defineCharacteristics(self):
		self.name = "Delete Column"
		self.group = "Modify"

		self.addParameter(ParameterVector(self.LAYERNAME, "Source Layer", ParameterVector.VECTOR_TYPE_ANY))
		self.addParameter(ParameterTableField(self.COLUMN, "Fields to Delete", self.LAYERNAME))
		self.addOutput(OutputVector(self.SAVENAME, "Output Shapefile"))

	def getIcon(self):
		return  QtGui.QIcon(os.path.dirname(__file__) + "/icons/mmqgis_attribute_join.png")

	def processAlgorithm(self, progress):
		# Include must be done here to avoid cyclic import
		from sextante.core.Sextante import Sextante
		qgis = Sextante.getInterface()

		layer = QGisLayers.getObjectFromUri(self.getParameterValue(self.LAYERNAME))
		layername = layer.name()

		column = self.getParameterValue(self.COLUMN)
		savename = self.getOutputValue(self.SAVENAME)

		message = mmqgisx_delete_columns(qgis, layername, [ column ], savename, False)

		if message:
			raise GeoAlgorithmExecutionException(message)


class mmqgisx_delete_duplicate_geometries_dialog(GeoAlgorithm):

	LAYERNAME = "LAYERNAME"
	SAVENAME = "SAVENAME"

	def defineCharacteristics(self):
		self.name = "Delete Duplicate Geometries"
		self.group = "Modify"

		self.addParameter(ParameterVector(self.LAYERNAME, "Source Layer", ParameterVector.VECTOR_TYPE_ANY))
		self.addOutput(OutputVector(self.SAVENAME, "Output Shapefile"))

	def getIcon(self):
		return  QtGui.QIcon(os.path.dirname(__file__) + "/icons/mmqgis_attribute_join.png")

	def processAlgorithm(self, progress):
		# Include must be done here to avoid cyclic import
		from sextante.core.Sextante import Sextante
		qgis = Sextante.getInterface()

		layer = QGisLayers.getObjectFromUri(self.getParameterValue(self.LAYERNAME))
		layername = layer.name()

		savename = self.getOutputValue(self.SAVENAME)

		message = mmqgisx_delete_duplicate_geometries(qgis, layername, savename, False)

		if message:
			raise GeoAlgorithmExecutionException(message)

class mmqgisx_geocode_google_dialog(GeoAlgorithm):

	CSVNAME = "CSVNAME"
	ADDRESS = "ADDRESS"
	CITY = "CITY"
	STATE = "STATE"
	COUNTRY = "COUNTRY"
	SHAPEFILENAME = "SHAPEFILENAME"
	NOTFOUNDFILE = "NOTFOUNDFILE"

	def defineCharacteristics(self):
		self.name = "Geocode Google"
		self.group = "Transfer"

		self.addParameter(ParameterTable(self.CSVNAME, "Input CSV File", False))
		self.addParameter(ParameterTableField(self.ADDRESS, "Address", self.CSVNAME))
		self.addParameter(ParameterTableField(self.CITY, "City", self.CSVNAME))
		self.addParameter(ParameterTableField(self.STATE, "State", self.CSVNAME))
		self.addParameter(ParameterTableField(self.COUNTRY, "Country", self.CSVNAME))

		self.addOutput(OutputVector(self.SHAPEFILENAME, "Output Shapefile"))
		self.addOutput(OutputTable(self.NOTFOUNDFILE, "Not Found CSV Output List"))

	def getIcon(self):
		return  QtGui.QIcon(os.path.dirname(__file__) + "/icons/mmqgis_geocode_google.png")

	def processAlgorithm(self, progress):
		# Include must be done here to avoid cyclic import
		from sextante.core.Sextante import Sextante
		qgis = Sextante.getInterface()

		table = QGisLayers.getObjectFromUri(self.getParameterValue(self.CSVNAME))
		csvname = table.name()

		params = [ self.getParameterValue(self.ADDRESS), self.getParameterValue(self.CITY),
			self.getParameterValue(self.STATE), self.getParameterValue(self.COUNTRY) ]

		keys = []
		for param in params:
			if not (param in keys):
				keys.append(param)

		shapefilename = self.getOutputValue(self.SHAPEFILENAME)
		notfoundfile = self.getOutputValue(self.NOTFOUNDFILE)

		message = mmqgisx_geocode_google(qgis, csvname, shapefilename, notfoundfile, keys, False)

		if message:
			raise GeoAlgorithmExecutionException(message)

class mmqgisx_geometry_convert_dialog(GeoAlgorithm):

	LAYERNAME = "LAYERNAME"
	NEWTYPE = "NEWTYPE"
	SAVENAME = "SAVENAME"

	def defineCharacteristics(self):
		self.name = "Convert Geometry Type"
		self.group = "Modify"

		self.addParameter(ParameterVector(self.LAYERNAME, "SourceLayer", ParameterVector.VECTOR_TYPE_ANY))

		self.newtypes = ["Centroids", "Nodes", "Linestrings", "Multilinestrings", "Polygons"]
		self.addParameter(ParameterSelection(self.NEWTYPE, "New Geometry Type", self.newtypes, default = 0))

		self.addOutput(OutputVector(self.SAVENAME, "Output Shapefile"))

	def getIcon(self):
		return  QtGui.QIcon(os.path.dirname(__file__) + "/icons/mmqgis_attribute_export.png")

	def processAlgorithm(self, progress):
		# Include must be done here to avoid cyclic import
		from sextante.core.Sextante import Sextante
		qgis = Sextante.getInterface()

		layer = QGisLayers.getObjectFromUri(self.getParameterValue(self.LAYERNAME))
		layername = layer.name()
		savename = self.getOutputValue(self.SAVENAME)

		index = self.getParameterValue(self.NEWTYPE)

		splitnodes = 0
		if index == 0:
			newtype = QGis.WKBPoint
		elif index == 1:
			newtype = QGis.WKBPoint
			splitnodes = 1
		elif index == 2:
			newtype = QGis.WKBLineString
		elif index == 3:
			newtype = QGis.WKBMultiLineString
		elif index == 4:
			newtype = QGis.WKBPolygon
		else:
			newtype = QGis.WKBPoint

		message = mmqgisx_geometry_convert(qgis, layername, newtype, splitnodes, savename, False)

		if message:
			raise GeoAlgorithmExecutionException(message)


# Has bug when no attribute file with points

class mmqgisx_geometry_export_dialog(GeoAlgorithm):

	LAYERNAME = "LAYERNAME"
	NODEFILENAME = "NODEFILENAME"
	ATTRIBUTEFILENAME = "ATTRIBUTEFILENAME"
	FIELDDELIMITER = "FIELDDELIMITER"
	LINETERMINATOR = "LINETERMINATOR"

	def defineCharacteristics(self):
		self.name = "Geometry Export"
		self.group = "Transfer"

		self.addParameter(ParameterVector(self.LAYERNAME, "Source Layer", ParameterVector.VECTOR_TYPE_ANY))

		self.addOutput(OutputTable(self.NODEFILENAME, "Node CSV Output File Name"))
		self.addOutput(OutputTable(self.ATTRIBUTEFILENAME, "Attribute CSV Output File Name (non-points only)"))

		self.delimiters = ["Comma", "Bar", "Space"]
		self.addParameter(ParameterSelection(self.FIELDDELIMITER, "Delimiter", self.delimiters, default = 0))

		self.terminators = ["CRLF", "LF"]
		self.addParameter(ParameterSelection(self.LINETERMINATOR, "Delimiter", self.terminators, default = 0))

	def getIcon(self):
		return  QtGui.QIcon(os.path.dirname(__file__) + "/icons/mmqgis_attribute_export.png")

	def processAlgorithm(self, progress):
		# Include must be done here to avoid cyclic import
		from sextante.core.Sextante import Sextante
		qgis = Sextante.getInterface()

		layer = QGisLayers.getObjectFromUri(self.getParameterValue(self.LAYERNAME))
		layername = layer.name()

		node_filename = self.getOutputValue(self.NODEFILENAME)
		attribute_filename = self.getOutputValue(self.ATTRIBUTEFILENAME)
		print "Layer: " + str(layername)
		print "Nodes: " + str(node_filename)
		print "Attributes: " + str(attribute_filename)

		if self.getParameterValue(self.FIELDDELIMITER) == 1:
			field_delimiter = "|"
		elif self.getParameterValue(self.FIELDDELIMITER) == 2:
			field_delimiter = " "
		else:
			field_delimiter = ","

		if self.getParameterValue(self.LINETERMINATOR) == 1:
			line_terminator = "\n"
		else:
			line_terminator = "\r\n"

		message = mmqgisx_geometry_export_to_csv(qgis, layername, node_filename, attribute_filename, 
			field_delimiter, line_terminator)

		if message:
			raise GeoAlgorithmExecutionException(message)

class mmqgisx_geometry_import_dialog(GeoAlgorithm):

	NODEFILENAME = "NODEFILENAME"
	LONGITUDE = "LONGITUDE"
	LATITUDE = "LATITUDE"
	SHAPEID = "SHAPEID"
	GEOMETRYTYPE = "GEOMETRYTYPE"
	SHAPEFILENAME = "SHAPEFILENAME"

	def defineCharacteristics(self):
		self.name = "Geometry Import"
		self.group = "Transfer"

		self.addParameter(ParameterTable(self.NODEFILENAME, "Input CSV Nodes File", False))
		self.addParameter(ParameterTableField(self.LONGITUDE, "Longitude Column", self.NODEFILENAME))
		self.addParameter(ParameterTableField(self.LATITUDE, "Latitude Column", self.NODEFILENAME))
		self.addParameter(ParameterTableField(self.SHAPEID, "Shape ID COlumn", self.NODEFILENAME))
		self.geotypes = ['Point', 'Polyline', 'Polygon']
		self.addParameter(ParameterSelection(self.GEOMETRYTYPE, "Geometry Type", self.geotypes, default = 0))
		self.addOutput(OutputVector(self.SHAPEFILENAME, "Output Shapefile"))

	def getIcon(self):
		return  QtGui.QIcon(os.path.dirname(__file__) + "/icons/mmqgis_geometry_import.png")

	def processAlgorithm(self, progress):
		# Include must be done here to avoid cyclic import
		from sextante.core.Sextante import Sextante
		qgis = Sextante.getInterface()

		table = QGisLayers.getObjectFromUri(self.getParameterValue(self.NODEFILENAME))
		node_filename = table.name()

		longitude = self.getParameterValue(self.LONGITUDE)
		latitude = self.getParameterValue(self.LATITUDE)
		shapeid = self.getParameterValue(self.SHAPEID)
		geometrytype = self.geotypes[self.getParameterValue(self.GEOMETRYTYPE)]
		shapefilename = self.getOutputValue(self.SHAPEFILENAME)

		message = mmqgisx_geometry_import_from_csv(qgis, node_filename, longitude, latitude, 
			shapeid, geometrytype, shapefilename, False)

		if message:
			raise GeoAlgorithmExecutionException(message)

class mmqgisx_grid_dialog(GeoAlgorithm):

	HSPACING = "HSPACING"
	VSPACING = "VSPACING"
	WIDTH = "WIDTH"
	HEIGHT = "HEIGHT"
	CENTERX = "CENTERX"
	CENTERY = "CENTERY"
	GRIDTYPE = "GRIDTYPE"
	SAVENAME = "SAVENAME"

	def defineCharacteristics(self):
		self.name = "Create Grid"
		self.group = "Create"

		self.addParameter(ParameterNumber(self.HSPACING, "Horizontal Spacing", default = 10))
		self.addParameter(ParameterNumber(self.VSPACING, "Vertical Spacing", default = 10))
		self.addParameter(ParameterNumber(self.WIDTH, "Width", default = 360))
		self.addParameter(ParameterNumber(self.HEIGHT, "Height", default = 180))
		self.addParameter(ParameterNumber(self.CENTERX, "Center X", default = 0))
		self.addParameter(ParameterNumber(self.CENTERY, "Center Y", default = 0))
		self.gridtype_options = ["Rectangle (line)","Rectangle (polygon)","Diamond (polygon)","Hexagon (polygon)"]
		self.addParameter(ParameterSelection(self.GRIDTYPE, "Grid Type",
			self.gridtype_options, default = 0))
		self.addOutput(OutputVector(self.SAVENAME, "Output Shapefile"))
		# self.addParameter(ParameterSelection(self.ADDLAYER, "Add Layer to Project",
		#	["No", "Yes"], default = 1))

	def getIcon(self):
		return  QtGui.QIcon(os.path.dirname(__file__) + "/icons/mmqgis_grid.png")

	def processAlgorithm(self, progress):
		# Include must be done here to avoid cyclic import
		from sextante.core.Sextante import Sextante
		qgis = Sextante.getInterface()

		savename = self.getOutputValue(self.SAVENAME)
		hspacing = self.getParameterValue(self.HSPACING)
		vspacing = self.getParameterValue(self.VSPACING)
		width = self.getParameterValue(self.WIDTH)
		height = self.getParameterValue(self.HEIGHT)
		centerx = self.getParameterValue(self.CENTERX)
		centery = self.getParameterValue(self.CENTERY)
		originx = centerx - (width / 2.0)
		originy = centery - (height / 2.0)
		gridtype = self.gridtype_options[self.getParameterValue(self.GRIDTYPE)]

		message = mmqgisx_grid(qgis, savename, hspacing, vspacing, width, 
			height, originx, originy, gridtype, 0)

		if message:
			raise GeoAlgorithmExecutionException(message)

class mmqgisx_gridify_dialog(GeoAlgorithm):

	LAYERNAME = "LAYERNAME"
	HSPACING = "HSPACING"
	VSPACING = "VSPACING"
	SAVENAME = "SAVENAME"

	def defineCharacteristics(self):
		self.name = "Gridify"
		self.group = "Modify"

		self.addParameter(ParameterVector(self.LAYERNAME, "Source Layer", ParameterVector.VECTOR_TYPE_ANY))
		self.addParameter(ParameterNumber(self.HSPACING, "Horizontal Spacing", default = 0.1))
		self.addParameter(ParameterNumber(self.VSPACING, "Vertical Spacing", default = 0.1))
		self.addOutput(OutputVector(self.SAVENAME, "Output Shapefile"))

	def getIcon(self):
		return  QtGui.QIcon(os.path.dirname(__file__) + "/icons/mmqgis_gridify.png")

	def processAlgorithm(self, progress):
		# Include must be done here to avoid cyclic import
		from sextante.core.Sextante import Sextante
		qgis = Sextante.getInterface()

		layer = QGisLayers.getObjectFromUri(self.getParameterValue(self.LAYERNAME))
		layername = layer.name()
		hspacing = self.getParameterValue(self.HSPACING)
		vspacing = self.getParameterValue(self.VSPACING)
		savename = self.getOutputValue(self.SAVENAME)

		message = mmqgisx_gridify_layer(qgis, layername, hspacing, vspacing, savename, False)

		if message:
			raise GeoAlgorithmExecutionException(message)

class mmqgisx_hub_distance_dialog(GeoAlgorithm):

	SOURCENAME = "SOURCENAME"
	DESTNAME = "DESTNAME"
	NAMEATTRIBUTE = "NAMEATTRIBUTE"
	SHAPETYPE = "SHAPETYPE"
	UNITS = "UNITS"
	SAVENAME = "SAVENAME"

	def defineCharacteristics(self):
		self.name = "Distance to Nearest Hub"
		self.group = "Create"

		self.addParameter(ParameterVector(self.SOURCENAME, "Source Points Layer", ParameterVector.VECTOR_TYPE_ANY))
		self.addParameter(ParameterVector(self.DESTNAME, "Destination Hubs Layer", ParameterVector.VECTOR_TYPE_ANY))
		self.addParameter(ParameterTableField(self.NAMEATTRIBUTE, "Hub Layer Name Attribute", self.DESTNAME))

		self.shapetypes = ["Point", "Line to Hub"]
		self.addParameter(ParameterSelection(self.SHAPETYPE, "Output Shape Type", self.shapetypes, default = 0))
		self.unitlist = ["Meters", "Feet", "Miles", "Kilometers", "Layer Units"]
		self.addParameter(ParameterSelection(self.UNITS, "Measurement Unit", self.unitlist, default = 0))

		self.addOutput(OutputVector(self.SAVENAME, "Output Shapefile"))

	def getIcon(self):
		return  QtGui.QIcon(os.path.dirname(__file__) + "/icons/mmqgis_hub_distance.png")

	def processAlgorithm(self, progress):
		# Include must be done here to avoid cyclic import
		from sextante.core.Sextante import Sextante
		qgis = Sextante.getInterface()

		layer = QGisLayers.getObjectFromUri(self.getParameterValue(self.SOURCENAME))
		sourcename = layer.name()

		layer = QGisLayers.getObjectFromUri(self.getParameterValue(self.DESTNAME))
		destname = layer.name()

		nameattribute = self.getParameterValue(self.NAMEATTRIBUTE)
		units = self.unitlist[self.getParameterValue(self.UNITS)]
		addlines = self.getParameterValue(self.SHAPETYPE)
		savename = self.getOutputValue(self.SAVENAME)

		message = mmqgisx_hub_distance(qgis, sourcename, destname, nameattribute, units, addlines, savename, False)

		if message:
			raise GeoAlgorithmExecutionException(message)

class mmqgisx_hub_lines_dialog(GeoAlgorithm):

	HUBNAME = "HUBNAME"
	HUBATTRIBUTE = "HUBATTRIBUTE"
	SPOKENAME = "SPOKENAME"
	SPOKEATTRIBUTE = "SPOKEATTRIBUTE"
	SAVENAME = "SAVENAME"

	def defineCharacteristics(self):
		self.name = "Hub Lines"
		self.group = "Create"

		self.addParameter(ParameterVector(self.HUBNAME, "Hub Point Layer", ParameterVector.VECTOR_TYPE_ANY))
		self.addParameter(ParameterTableField(self.HUBATTRIBUTE, "Hub ID Attribute", self.HUBNAME))
		self.addParameter(ParameterVector(self.SPOKENAME, "Spoke Point Layer", ParameterVector.VECTOR_TYPE_ANY))
		self.addParameter(ParameterTableField(self.SPOKEATTRIBUTE, "Spoke Hub ID Attribute", self.SPOKENAME))
		self.addOutput(OutputVector(self.SAVENAME, "Output Shapefile"))

	def getIcon(self):
		return  QtGui.QIcon(os.path.dirname(__file__) + "/icons/mmqgis_hub_distance.png")

	def processAlgorithm(self, progress):
		# Include must be done here to avoid cyclic import
		from sextante.core.Sextante import Sextante
		qgis = Sextante.getInterface()

		layer = QGisLayers.getObjectFromUri(self.getParameterValue(self.HUBNAME))
		hubname = layer.name()

		layer = QGisLayers.getObjectFromUri(self.getParameterValue(self.SPOKENAME))
		spokename = layer.name()

		hubattribute = self.getParameterValue(self.HUBATTRIBUTE)
		spokeattribute = self.getParameterValue(self.SPOKEATTRIBUTE)

		savename = self.getOutputValue(self.SAVENAME)

		message = mmqgisx_hub_lines(qgis, hubname, hubattribute, spokename, spokeattribute, savename, False)

		if message:
			raise GeoAlgorithmExecutionException(message)

class mmqgisx_label_point_dialog(GeoAlgorithm):

	LAYERNAME = "LAYERNAME"
	LABELATTRIBUTE = "LABELATTRIBUTE"
	SAVENAME = "SAVENAME"

	def defineCharacteristics(self):
		self.name = "Create Label Layer"
		self.group = "Create"

		self.addParameter(ParameterVector(self.LAYERNAME, "Source Layer", ParameterVector.VECTOR_TYPE_ANY))
		self.addParameter(ParameterTableField(self.LABELATTRIBUTE, "Label Attribute", self.LAYERNAME))
		self.addOutput(OutputVector(self.SAVENAME, "Output Shapefile"))

	def getIcon(self):
		return  QtGui.QIcon(os.path.dirname(__file__) + "/icons/mmqgis_label.png")

	def processAlgorithm(self, progress):
		# Include must be done here to avoid cyclic import
		from sextante.core.Sextante import Sextante
		qgis = Sextante.getInterface()

		layer = QGisLayers.getObjectFromUri(self.getParameterValue(self.LAYERNAME))
		layername = layer.name()

		labelattribute = self.getParameterValue(self.LABELATTRIBUTE)

		savename = self.getOutputValue(self.SAVENAME)

		message = mmqgisx_label_point(qgis, layername, labelattribute, savename, False)

		if message:
			raise GeoAlgorithmExecutionException(message)

class mmqgisx_merge_dialog(GeoAlgorithm):

	LAYER1 = "LAYER1"
	LAYER2 = "LAYER2"
	SAVENAME = "SAVENAME"

	def defineCharacteristics(self):
		self.name = "Merge Layers"
		self.group = "Transfer"

		self.addParameter(ParameterVector(self.LAYER1, "Source Layer 1", ParameterVector.VECTOR_TYPE_ANY))
		self.addParameter(ParameterVector(self.LAYER2, "Source Layer 2", ParameterVector.VECTOR_TYPE_ANY))
		self.addOutput(OutputVector(self.SAVENAME, "Output Shapefile"))

	def getIcon(self):
		return  QtGui.QIcon(os.path.dirname(__file__) + "/icons/mmqgis_merge.png")

	def processAlgorithm(self, progress):
		# Include must be done here to avoid cyclic import
		from sextante.core.Sextante import Sextante
		qgis = Sextante.getInterface()

		layer = QGisLayers.getObjectFromUri(self.getParameterValue(self.LAYER1))
		layer1 = layer.name()

		layer = QGisLayers.getObjectFromUri(self.getParameterValue(self.LAYER2))
		layer2 = layer.name()

		savename = self.getOutputValue(self.SAVENAME)

		message = mmqgisx_merge(qgis, [ layer1, layer2 ], savename, False)

		if message:
			raise GeoAlgorithmExecutionException(message)

class mmqgisx_select_dialog(GeoAlgorithm):

	LAYERNAME = "LAYERNAME"
	ATTRIBUTE = "ATTRIBUTE"
	COMPARISONVALUE = "COMPARISONVALUE"
	COMPARISON = "COMPARISON"
	SAVENAME = "SAVENAME"

	def defineCharacteristics(self):
		self.name = "Select"
		self.group = "Transfer"

		self.addParameter(ParameterVector(self.LAYERNAME, "Source Layer", ParameterVector.VECTOR_TYPE_ANY))
		self.addParameter(ParameterTableField(self.ATTRIBUTE, "Selection Attribute", self.LAYERNAME))
		self.comparisons = ['==', '!=', '>', '>=', '<', '<=', 'begins with', 'contains']
		self.addParameter(ParameterSelection(self.COMPARISON, "Comparison", self.comparisons, default = 0))
		self.addParameter(ParameterString(self.COMPARISONVALUE, "Value", default = "0"))

		self.addOutput(OutputVector(self.SAVENAME, "Output Shapefile"))

	def getIcon(self):
		return  QtGui.QIcon(os.path.dirname(__file__) + "/icons/mmqgis_attribute_export.png")

	def processAlgorithm(self, progress):
		# Include must be done here to avoid cyclic import
		from sextante.core.Sextante import Sextante
		qgis = Sextante.getInterface()

		layer = QGisLayers.getObjectFromUri(self.getParameterValue(self.LAYERNAME))
		layername = layer.name()

		attribute = self.getParameterValue(self.ATTRIBUTE)
		comparison = self.comparisons [ self.getParameterValue(self.COMPARISON) ]
		# print str(self.getParameterValue(self.COMPARISON)) + ": " + str(comparison)
		comparisonvalue = self.getParameterValue(self.COMPARISONVALUE)
		savename = self.getOutputValue(self.SAVENAME)

		message = mmqgisx_select(qgis, layername, attribute, comparisonvalue, comparison, savename, False)

		if message:
			raise GeoAlgorithmExecutionException(message)

class mmqgisx_sort_dialog(GeoAlgorithm):

	LAYERNAME = "LAYERNAME"
	ATTRIBUTE = "ATTRIBUTE"
	DIRECTION = "DIRECTION"
	SAVENAME = "SAVENAME"

	def defineCharacteristics(self):
		self.name = "Sort"
		self.group = "Modify"

		self.addParameter(ParameterVector(self.LAYERNAME, "Source Layer", ParameterVector.VECTOR_TYPE_ANY))
		self.addParameter(ParameterTableField(self.ATTRIBUTE, "Selection Attribute", self.LAYERNAME))
		self.directions = ['Ascending', 'Descending']
		self.addParameter(ParameterSelection(self.DIRECTION, "Direction", self.directions, default = 0))
		self.addOutput(OutputVector(self.SAVENAME, "Output Shapefile"))

	def getIcon(self):
		return  QtGui.QIcon(os.path.dirname(__file__) + "/icons/mmqgis_sort.png")

	def processAlgorithm(self, progress):
		# Include must be done here to avoid cyclic import
		from sextante.core.Sextante import Sextante
		qgis = Sextante.getInterface()

		layer = QGisLayers.getObjectFromUri(self.getParameterValue(self.LAYERNAME))
		layername = layer.name()

		attribute = self.getParameterValue(self.ATTRIBUTE)
		direction = self.directions [ self.getParameterValue(self.DIRECTION) ]
		savename = self.getOutputValue(self.SAVENAME)

		message = mmqgisx_sort(qgis, layername, attribute, savename, direction, False)

		if message:
			raise GeoAlgorithmExecutionException(message)

# Too complex for Sextante GUI
# def mmqgisx_geocode_street_layer(GeoAlgorithm):

class mmqgisx_text_to_float_dialog(GeoAlgorithm):

	LAYERNAME = "LAYERNAME"
	ATTRIBUTE = "ATTRIBUTE"
	SAVENAME = "SAVENAME"

	def defineCharacteristics(self):
		self.name = "Text to Float"
		self.group = "Modify"

		self.addParameter(ParameterVector(self.LAYERNAME, "Source Layer", ParameterVector.VECTOR_TYPE_ANY))
		self.addParameter(ParameterTableField(self.ATTRIBUTE, "Text Attribute to Convert to Float", self.LAYERNAME))
		self.addOutput(OutputVector(self.SAVENAME, "Output Shapefile"))

	def getIcon(self):
		return  QtGui.QIcon(os.path.dirname(__file__) + "/icons/mmqgis_text_to_float.png")

	def processAlgorithm(self, progress):
		# Include must be done here to avoid cyclic import
		from sextante.core.Sextante import Sextante
		qgis = Sextante.getInterface()

		layer = QGisLayers.getObjectFromUri(self.getParameterValue(self.LAYERNAME))
		layername = layer.name()

		attribute = self.getParameterValue(self.ATTRIBUTE)
		savename = self.getOutputValue(self.SAVENAME)

		message = mmqgisx_text_to_float(qgis, layername, [ attribute ], savename, False)

		if message:
			raise GeoAlgorithmExecutionException(message)

class mmqgisx_voronoi_dialog(GeoAlgorithm):

	LAYERNAME = "LAYERNAME"
	SAVENAME = "SAVENAME"

	def defineCharacteristics(self):
		self.name = "Voronoi Diagram"
		self.group = "Create"

		self.addParameter(ParameterVector(self.LAYERNAME, "Points Layer", ParameterVector.VECTOR_TYPE_ANY))
		self.addOutput(OutputVector(self.SAVENAME, "Output Shapefile"))

	def getIcon(self):
		return  QtGui.QIcon(os.path.dirname(__file__) + "/icons/mmqgis_voronoi.png")

	def processAlgorithm(self, progress):
		# Include must be done here to avoid cyclic import
		from sextante.core.Sextante import Sextante
		qgis = Sextante.getInterface()

		layer = QGisLayers.getObjectFromUri(self.getParameterValue(self.LAYERNAME))
		layername = layer.name()

		savename = self.getOutputValue(self.SAVENAME)

		message = mmqgisx_voronoi_diagram(qgis, layername, savename, False)

		if message:
			raise GeoAlgorithmExecutionException(message)

