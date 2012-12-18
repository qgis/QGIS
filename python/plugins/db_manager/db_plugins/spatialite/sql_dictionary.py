# -*- coding: utf-8 -*-

"""
***************************************************************************
    sql_dictionary.py
    ---------------------
    Date                 : April 2012
    Copyright            : (C) 2012 by Giuseppe Sucameli
    Email                : brush dot tyler at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Giuseppe Sucameli'
__date__ = 'April 2012'
__copyright__ = '(C) 2012, Giuseppe Sucameli'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

# keywords
keywords = [
# TODO get them from a reference page
"action", "add", "after", "all", "alter", "analyze", "and", "as", "asc",
"before", "begin", "between", "by", "cascade", "case", "cast", "check",
"collate", "column", "commit", "constraint", "create", "cross", "current_date",
"current_time", "current_timestamp", "default", "deferrable", "deferred",
"delete", "desc", "distinct", "drop", "each", "else", "end", "escape",
"except", "exists", "for", "foreign", "from", "full", "group", "having",
"ignore", "immediate", "in", "initially", "inner", "insert", "intersect",
"into", "is", "isnull", "join", "key", "left", "like", "limit", "match",
"natural", "no", "not", "notnull", "null", "of", "offset", "on", "or", "order",
"outer", "primary", "references", "release", "restrict", "right", "rollback",
"row", "savepoint", "select", "set", "table", "temporary", "then", "to",
"transaction", "trigger", "union", "unique", "update", "using", "values",
"view", "when", "where",

"abort", "attach", "autoincrement", "conflict", "database", "detach",
"exclusive", "explain", "fail", "glob", "if", "index", "indexed", "instead",
"plan", "pragma", "query", "raise", "regexp", "reindex", "rename", "replace",
"temp", "vacuum", "virtual"
]
spatialite_keywords = []

# functions
functions = [
# TODO get them from a reference page
"abs", "changes", "coalesce", "glob", "ifnull", "hex", "last_insert_rowid",
"length", "like", "lower", "ltrim", "max", "min", "nullif", "quote", "random",
"randomblob", "replace", "round", "rtrim", "soundex", "total_change", "trim",
"typeof", "upper", "zeroblob", "date", "datetime", "julianday", "strftime",
"avg", "count", "group_concat", "sum", "total"
]
spatialite_functions = [	# from www.gaia-gis.it/spatialite-2.3.0/spatialite-sql-2.3.0.html
# SQL math functions
"abs", "acos", "asin", "atan", "cos", "cot", "degrees", "exp", "floor", "log", "log2", "log10", "pi", "radians", "round", "sign", "sin", "sqrt", "stddev_pop", "stddev_samp", "tan", "var_pop", "var_samp",
# SQL utility functions for BLOB objects
"iszipblob", "ispdfblob", "isgifblob", "ispngblob", "isjpegblob", "isexifblob", "isexifgpsblob", "geomfromexifgpsblob", "makepoint", "buildmbr", "buildcirclembr", "mbrminx", "mbrminy", "mbrmaxx", "mbrmaxy",
# SQL functions for constructing a geometric object given its Well-known Text Representation
"geomfromtext", "pointfromtext",
# SQL functions for constructing a geometric object given its Well-known Binary Representation
"geomfromwkb", "pointfromwkb",
# SQL functions for obtaining the Well-known Text / Well-known Binary Representation of a geometric object
"astext", "asbinary",
# SQL functions supporting exotic geometric formats
"assvg", "asfgf", "geomfromfgf",
# SQL functions on type Geometry
"dimension", "geometrytype", "srid", "setsrid", "isempty", "issimple", "isvalid", "boundary", "envelope",
# SQL functions on type Point
"x", "y",
# SQL functions on type Curve [Linestring or Ring]
"startpoint", "endpoint", "glength", "isclosed", "isring", "simplify", "simplifypreservetopology",
# SQL functions on type LineString
"numpoints", "pointn",
# SQL functions on type Surface [Polygon or Ring]
"centroid", "pointonsurface", "area",
# SQL functions on type Polygon
"exteriorring", "interiorringn",
# SQL functions on type GeomCollection
"numgeometries", "geometryn",
# SQL functions that test approximative spatial relationships via MBRs
"mbrequal", "mbrdisjoint", "mbrtouches", "mbrwithin", "mbroverlaps", "mbrintersects", "mbrcontains",
# SQL functions that test spatial relationships
"equals", "disjoint", "touches", "within", "overlaps", "crosses", "intersects", "contains", "relate",
# SQL functions for distance relationships
"distance",
# SQL functions that implement spatial operators
"intersection", "difference", "gunion", "gunion", "symdifference", "buffer", "convexhull",
# SQL functions for coordinate transformations
"transform",
# SQL functions for Spatial-MetaData and Spatial-Index handling
"initspatialmetadata", "addgeometrycolumn", "recovergeometrycolumn", "discardgeometrycolumn", "createspatialindex", "creatembrcache", "disablespatialindex",
# SQL functions implementing FDO/OGR compatibily
"checkspatialmetadata", "autofdostart", "autofdostop", "initfdospatialmetadata", "addfdogeometrycolumn", "recoverfdogeometrycolumn", "discardfdogeometrycolumn",
# SQL functions for MbrCache-based queries
"filtermbrwithin", "filtermbrcontains", "filtermbrintersects", "buildmbrfilter"
]

# constants
constants = [ "null", "false", "true" ]
spatialite_constants = []

def getSqlDictionary(spatial=True):
	k, c, f = list(keywords), list(constants), list(functions)

	if spatial:
		k += spatialite_keywords
		f += spatialite_functions
		c += spatialite_constants

	return { 'keyword' : k, 'constant' : c, 'function' : f }

