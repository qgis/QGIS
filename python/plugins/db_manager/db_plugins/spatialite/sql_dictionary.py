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
    "changes", "coalesce", "glob", "ifnull", "hex", "last_insert_rowid",
    "nullif", "quote", "random",
    "randomblob", "replace", "round", "soundex", "total_change", 
    "typeof", "zeroblob", "date", "datetime", "julianday", "strftime"
]
operators=[
' AND ',' OR ','||',' < ',' <= ',' > ',' >= ',' = ',' <> ',' IS ',' IS NOT ',' IN ',' LIKE ',' GLOB ',' MATCH ',' REGEXP '
]

math_functions = [
    # SQL math functions
    "Abs", "ACos", "ASin", "ATan", "Cos", "Cot", "Degrees", "Exp", "Floor", "Log", "Log2",
    "Log10", "Pi", "Radians", "Round", "Sign", "Sin", "Sqrt", "StdDev_Pop", "StdDev_Samp", "Tan",
    "Var_Pop", "Var_Samp" ]

string_functions=["Length", "Lower", "Upper", "Like", "Trim", "LTrim", "RTrim", "Replace", "Substr"]

aggregate_functions=[
"Max","Min","Avg","Count","Sum","Group_Concat","Total","Var_Pop","Var_Samp","StdDev_Pop","StdDev_Samp"
]

spatialite_functions = [  # from www.gaia-gis.it/spatialite-2.3.0/spatialite-sql-2.3.0.html
                          # SQL utility functions for BLOB objects
                          "*iszipblob", "*ispdfblob", "*isgifblob", "*ispngblob", "*isjpegblob", "*isexifblob",
                          "*isexifgpsblob", "*geomfromexifgpsblob", "MakePoint", "BuildMbr", "*buildcirclembr", "ST_MinX",
                          "ST_MinY", "ST_MaxX", "ST_MaxY",
                          # SQL functions for constructing a geometric object given its Well-known Text Representation
                          "ST_GeomFromText", "*pointfromtext",
                          # SQL functions for constructing a geometric object given its Well-known Binary Representation
                          "*geomfromwkb", "*pointfromwkb",
                          # SQL functions for obtaining the Well-known Text / Well-known Binary Representation of a geometric object
                          "ST_AsText", "ST_AsBinary",
                          # SQL functions supporting exotic geometric formats
                          "*assvg", "*asfgf", "*geomfromfgf",
                          # SQL functions on type Geometry
                          "ST_Dimension", "ST_GeometryType", "ST_Srid", "ST_SetSrid", "ST_isEmpty", "ST_isSimple", "ST_isValid", "ST_Boundary",
                          "ST_Envelope",
                          # SQL functions on type Point
                          "ST_X", "ST_Y",
                          # SQL functions on type Curve [Linestring or Ring]
                          "ST_StartPoint", "ST_EndPoint", "ST_Length", "ST_isClosed", "ST_isRing", "ST_Simplify",
                          "*simplifypreservetopology",
                          # SQL functions on type LineString
                          "ST_NumPoints", "ST_PointN",
                          # SQL functions on type Surface [Polygon or Ring]
                          "ST_Centroid", "ST_PointOnSurface", "ST_Area",
                          # SQL functions on type Polygon
                          "ST_ExteriorRing", "ST_InteriorRingN",
                          # SQL functions on type GeomCollection
                          "ST_NumGeometries", "ST_GeometryN",
                          # SQL functions that test approximative spatial relationships via MBRs
                          "MbrEqual", "MbrDisjoint", "MbrTouches", "MbrWithin", "MbrOverlaps", "MbrIntersects",
                          "MbrContains",
                          # SQL functions that test spatial relationships
                          "ST_Equals", "ST_Disjoint", "ST_Touches", "ST_Within", "ST_Overlaps", "ST_Crosses", "ST_Intersects", "ST_Contains",
                          "ST_Relate",
                          # SQL functions for distance relationships
                          "ST_Distance",
                          # SQL functions that implement spatial operators
                          "ST_Intersection", "ST_Difference", "ST_Union", "ST_SymDifference", "ST_Buffer", "ST_ConvexHull",
                          # SQL functions for coordinate transformations
                          "ST_Transform",
                          # SQL functions for Spatial-MetaData and Spatial-Index handling
                          "*initspatialmetadata", "*addgeometrycolumn", "*recovergeometrycolumn", "*discardgeometrycolumn",
                          "*createspatialindex", "*creatembrcache", "*disablespatialindex",
                          # SQL functions implementing FDO/OGR compatibily
                          "*checkspatialmetadata", "*autofdostart", "*autofdostop", "*initfdospatialmetadata",
                          "*addfdogeometrycolumn", "*recoverfdogeometrycolumn", "*discardfdogeometrycolumn",
                          # SQL functions for MbrCache-based queries
                          "*filtermbrwithin", "*filtermbrcontains", "*filtermbrintersects", "*buildmbrfilter"
]

# constants
constants = ["null", "false", "true"]
spatialite_constants = []

def getSqlDictionary(spatial=True):
    def strip_star(s):
        if s[0] == '*':
            return s.lower()[1:]
        else:
            return s.lower()

    k, c, f = list(keywords), list(constants), list(functions)

    if spatial:
        k += spatialite_keywords
        f += spatialite_functions
        c += spatialite_constants

    return {'keyword': map(strip_star,k), 'constant': map(strip_star,c), 'function': map(strip_star,f)}

def getQueryBuilderDictionary():
    # concat functions
    def ff( l ):
        return filter( lambda s:s[0] != '*', l )
    def add_paren( l ):
        return map( lambda s:s+"(", l )
    foo = sorted(add_paren(ff( list(set.union(set(functions), set(spatialite_functions))) )))
    m = sorted(add_paren(ff( math_functions )))
    agg = sorted(add_paren(ff(aggregate_functions)))
    op = ff(operators)
    s = sorted(add_paren(ff(string_functions)))
    return {'function': foo, 'math' : m, 'aggregate': agg, 'operator': op, 'string': s }
