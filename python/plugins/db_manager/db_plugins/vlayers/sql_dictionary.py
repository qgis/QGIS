"""
***************************************************************************
    sql_dictionary.py
    ---------------------
    Date                 : December 2015
    Copyright            : (C) 2015 by Hugo Mercier
    Email                : hugo dot mercier at oslandia dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Hugo Mercier'
__date__ = 'December 2015'
__copyright__ = '(C) 2015, Hugo Mercier'

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
operators = [
    ' AND ', ' OR ', '||', ' < ', ' <= ', ' > ', ' >= ', ' = ', ' <> ', ' IS ', ' IS NOT ', ' IN ', ' LIKE ', ' GLOB ', ' MATCH ', ' REGEXP '
]

math_functions = [
    # SQL math functions
    "Abs", "ACos", "ASin", "ATan", "Cos", "Cot", "Degrees", "Exp", "Floor", "Log", "Log2",
    "Log10", "Pi", "Radians", "Round", "Sign", "Sin", "Sqrt", "StdDev_Pop", "StdDev_Samp", "Tan",
    "Var_Pop", "Var_Samp"]

string_functions = ["Length", "Lower", "Upper", "Like", "Trim", "LTrim", "RTrim", "Replace", "Substr"]

aggregate_functions = [
    "Max", "Min", "Avg", "Count", "Sum", "Group_Concat", "Total", "Var_Pop", "Var_Samp", "StdDev_Pop", "StdDev_Samp"
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
                          # SQL functions implementing FDO/OGR compatibility
                          "*checkspatialmetadata", "*autofdostart", "*autofdostop", "*initfdospatialmetadata",
                          "*addfdogeometrycolumn", "*recoverfdogeometrycolumn", "*discardfdogeometrycolumn",
                          # SQL functions for MbrCache-based queries
                          "*filtermbrwithin", "*filtermbrcontains", "*filtermbrintersects", "*buildmbrfilter"
]

qgis_functions = [
    "atan2", "round", "rand", "randf", "clamp", "scale_linear", "scale_polynomial", "scale_exponential", "_pi", "to_int", "toint", "to_real", "toreal",
    "to_string", "tostring", "to_datetime", "todatetime", "to_date", "todate", "to_time", "totime", "to_interval", "tointerval",
    "regexp_match", "now", "_now", "age", "year", "month", "week", "day", "hour", "minute", "second", "day_of_week", "title",
    "levenshtein", "longest_common_substring", "hamming_distance", "wordwrap", "regexp_replace", "regexp_substr", "concat",
    "strpos", "_left", "_right", "rpad", "lpad", "format", "format_number", "format_date", "color_rgb", "color_rgba", "color_rgbf", "ramp_color", "ramp_color_object",
    "color_hsl", "color_hsla", "color_hslf", "color_hsv", "color_hsva", "color_hsvf", "color_cmyk", "color_cmyka", "color_cmykf", "color_part", "darker", "lighter",
    "set_color_part", "point_n", "start_point", "end_point", "nodes_to_points", "segments_to_lines", "make_point",
    "make_point_m", "make_line", "make_polygon", "x_min", "xmin", "x_max", "xmax", "y_min", "ymin", "y_max", "ymax", "geom_from_wkt",
    "geomFromWKT", "geom_from_gml", "relate", "intersects_bbox", "bbox", "translate", "buffer", "point_on_surface", "reverse",
    "exterior_ring", "interior_ring_n", "geometry_n", "bounds", "num_points", "num_interior_rings", "num_rings", "num_geometries",
    "bounds_width", "bounds_height", "is_closed", "convex_hull", "sym_difference", "combine", "_union", "geom_to_wkt", "geomToWKT",
    "transform", "uuid", "_uuid", "layer_property", "var", "_specialcol_", "project_color", "project_color_object"]


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
        f += qgis_functions
        c += spatialite_constants

    return {'keyword': list(map(strip_star, k)), 'constant': list(map(strip_star, c)), 'function': list(map(strip_star, f))}


def getQueryBuilderDictionary():
    # concat functions
    def ff(l):
        return [s for s in l if s[0] != '*']

    def add_paren(l):
        return [s + "(" for s in l]

    foo = sorted(add_paren(ff(list(set.union(set(functions), set(spatialite_functions), set(qgis_functions))))))
    m = sorted(add_paren(ff(math_functions)))
    agg = sorted(add_paren(ff(aggregate_functions)))
    op = ff(operators)
    s = sorted(add_paren(ff(string_functions)))
    return {'function': foo, 'math': m, 'aggregate': agg, 'operator': op, 'string': s}
