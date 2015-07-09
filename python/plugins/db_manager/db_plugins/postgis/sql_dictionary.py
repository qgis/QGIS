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

    "absolute", "admin", "aggregate", "alias", "allocate", "analyse", "any", "are",
    "array", "asensitive", "assertion", "asymmetric", "at", "atomic",
    "authorization", "avg", "bigint", "binary", "bit", "bit_length", "blob",
    "boolean", "both", "breadth", "call", "called", "cardinality", "cascaded",
    "catalog", "ceil", "ceiling", "char", "character", "character_length",
    "char_length", "class", "clob", "close", "coalesce", "collation", "collect",
    "completion", "condition", "connect", "connection", "constraints",
    "constructor", "continue", "convert", "corr", "corresponding", "count",
    "covar_pop", "covar_samp", "cube", "cume_dist", "current",
    "current_default_transform_group", "current_path", "current_role",
    "current_transform_group_for_type", "current_user", "cursor", "cycle", "data",
    "date", "day", "deallocate", "dec", "decimal", "declare", "dense_rank",
    "depth", "deref", "describe", "descriptor", "destroy", "destructor",
    "deterministic", "diagnostics", "dictionary", "disconnect", "do", "domain",
    "double", "dynamic", "element", "end-exec", "equals", "every", "exception",
    "exec", "execute", "exp", "external", "extract", "false", "fetch", "filter",
    "first", "float", "floor", "found", "free", "freeze", "function", "fusion",
    "general", "get", "global", "go", "goto", "grant", "grouping", "hold", "host",
    "hour", "identity", "ilike", "indicator", "initialize", "inout", "input",
    "insensitive", "int", "integer", "intersection", "interval", "isolation",
    "iterate", "language", "large", "last", "lateral", "leading", "less", "level",
    "ln", "local", "localtime", "localtimestamp", "locator", "lower", "map", "max",
    "member", "merge", "method", "min", "minute", "mod", "modifies", "modify",
    "module", "month", "multiset", "names", "national", "nchar", "nclob", "new",
    "next", "none", "normalize", "nullif", "numeric", "object", "octet_length",
    "off", "old", "only", "open", "operation", "option", "ordinality", "out",
    "output", "over", "overlaps", "overlay", "pad", "parameter", "parameters",
    "partial", "partition", "path", "percentile_cont", "percentile_disc",
    "percent_rank", "placing", "position", "postfix", "power", "precision",
    "prefix", "preorder", "prepare", "preserve", "prior", "privileges",
    "procedure", "public", "range", "rank", "read", "reads", "real", "recursive",
    "ref", "referencing", "regr_avgx", "regr_avgy", "regr_count", "regr_intercept",
    "regr_r2", "regr_slope", "regr_sxx", "regr_sxy", "regr_syy", "relative",
    "result", "return", "returning", "returns", "revoke", "role", "rollup",
    "routine", "rows", "row_number", "schema", "scope", "scroll", "search",
    "second", "section", "sensitive", "sequence", "session", "session_user",
    "sets", "similar", "size", "smallint", "some", "space", "specific",
    "specifictype", "sql", "sqlcode", "sqlerror", "sqlexception", "sqlstate",
    "sqlwarning", "sqrt", "start", "state", "statement", "static", "stddev_pop",
    "stddev_samp", "structure", "submultiset", "substring", "sum", "symmetric",
    "system", "system_user", "tablesample", "terminate", "than", "time",
    "timestamp", "timezone_hour", "timezone_minute", "trailing", "translate",
    "translation", "treat", "trim", "true", "uescape", "under", "unknown",
    "unnest", "upper", "usage", "user", "value", "varchar", "variable", "varying",
    "var_pop", "var_samp", "verbose", "whenever", "width_bucket", "window", "with",
    "within", "without", "work", "write", "xml", "xmlagg", "xmlattributes",
    "xmlbinary", "xmlcomment", "xmlconcat", "xmlelement", "xmlforest",
    "xmlnamespaces", "xmlparse", "xmlpi", "xmlroot", "xmlserialize", "year", "zone"
]
postgis_keywords = []

# functions
functions = [
    "coalesce",
    "nullif", "quote", "random",
    "replace", "soundex"
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

postgis_functions = [  # from http://www.postgis.org/docs/reference.html
                       # 7.1. PostgreSQL PostGIS Types
                       "*box2d", "*box3d", "*box3d_extent", "*geometry", "*geometry_dump", "*geography",
                       # 7.2. Management Functions
                       "*addgeometrycolumn", "*dropgeometrycolumn", "*dropgeometrytable", "*postgis_full_version",
                       "*postgis_geos_version", "*postgis_libxml_version", "*postgis_lib_build_date",
                       "*postgis_lib_version", "*postgis_proj_version", "*postgis_scripts_build_date",
                       "*postgis_scripts_installed", "*postgis_scripts_released", "*postgis_uses_stats", "*postgis_version",
                       "*populate_geometry_columns", "*probe_geometry_columns", "*updategeometrysrid",
                       # 7.3. Geometry Constructors
                       "*ST_bdpolyfromtext", "*ST_bdmpolyfromtext", "*ST_geogfromtext", "*ST_geographyfromtext",
                       "*ST_geogfromwkb", "*ST_geomcollfromtext", "*ST_geomfromewkb", "*ST_geomfromewkt",
                       "*ST_geometryfromtext", "*ST_geomfromgml", "*ST_geomfromkml", "*ST_gmltosql", "*ST_geomfromtext",
                       "*ST_geomfromwkb", "*ST_linefrommultipoint", "*ST_linefromtext", "*ST_linefromwkb",
                       "*ST_linestringfromwkb", "*ST_makebox2d", "*ST_makebox3d", "ST_MakeLine", "*ST_makeenvelope",
                       "ST_MakePolygon", "ST_MakePoint", "ST_MakePointM", "*ST_MLinefromtext", "*ST_mpointfromtext",
                       "*ST_mpolyfromtext", "ST_Point", "*ST_pointfromtext", "*ST_pointfromwkb", "ST_Polygon",
                       "*ST_polygonfromtext", "*ST_wkbtosql", "*ST_wkttosql",
                       # 7.4. Geometry Accessors
                       "GeometryType", "ST_Boundary", "*ST_coorddim", "ST_Dimension", "ST_EndPoint", "ST_Envelope",
                       "ST_ExteriorRing", "ST_GeometryN", "ST_GeometryType", "ST_InteriorRingN", "ST_isClosed",
                       "ST_isEmpty", "ST_isRing", "ST_isSimple", "ST_isValid", "ST_isValidReason", "ST_M", "ST_NDims",
                       "ST_NPoints", "ST_NRings", "ST_NumGeometries", "ST_NumInteriorrings", "ST_NumInteriorring",
                       "ST_NumPoints", "ST_PointN", "ST_Srid", "ST_StartPoint", "ST_Summary", "ST_X", "ST_Y", "ST_Z",
                       "*ST_zmflag",
                       # 7.5. Geometry Editors
                       "ST_AddPoint", "ST_Affine", "ST_Force2D", "*ST_Force3D", "*ST_Force3dZ", "*ST_Force3DM",
                       "*ST_Force_4d", "*ST_force_collection", "*ST_forcerhr", "*ST_linemerge", "*ST_collectionextract",
                       "ST_Multi", "*ST_removepoint", "*ST_reverse", "*ST_rotate", "*ST_rotatex", "*ST_rotatey",
                       "*ST_rotatez", "*ST_scale", "*ST_segmentize", "*ST_setpoint", "ST_SetSrid", "ST_SnapToGrid",
                       "ST_Transform", "ST_Translate", "*ST_transscale",
                       # 7.6. Geometry Outputs
                       "*ST_asbinary", "*ST_asewkb", "*ST_asewkt", "*ST_asgeojson", "*ST_asgml", "*ST_ashexewkb", "*ST_askml",
                       "*ST_assvg", "*ST_geohash", "ST_Astext",
                       # 7.7. Operators
                       # 7.8. Spatial Relationships and Measurements
                       "ST_Area", "ST_Azimuth", "ST_Centroid", "ST_ClosestPoint", "ST_Contains", "ST_ContainsProperly",
                       "ST_Covers", "ST_CoveredBy", "ST_Crosses", "*ST_linecrossingdirection", "ST_Cisjoint",
                       "ST_Distance", "*ST_hausdorffdistance", "*ST_maxdistance", "ST_Distance_Sphere",
                       "ST_Distance_Spheroid", "*ST_DFullyWithin", "ST_DWithin", "ST_Equals", "*ST_hasarc",
                       "ST_Intersects", "ST_Length", "*ST_Length2d", "*ST_length3d", "ST_Length_Spheroid",
                       "*ST_length2d_spheroid", "*ST_length3d_spheroid", "*ST_longestline", "*ST_orderingequals",
                       "ST_Overlaps", "*ST_perimeter", "*ST_perimeter2d", "*ST_perimeter3d", "ST_PointOnSurface",
                       "ST_Relate", "ST_ShortestLine", "ST_Touches", "ST_Within",
                       # 7.9. Geometry Processing Functions
                       "ST_Buffer", "ST_BuildArea", "ST_Collect", "ST_ConvexHull", "*ST_curvetoline", "ST_Difference",
                       "ST_Dump", "*ST_dumppoints", "*ST_dumprings", "ST_Intersection", "*ST_linetocurve", "*ST_memunion",
                       "*ST_minimumboundingcircle", "*ST_polygonize", "*ST_shift_longitude", "ST_Simplify",
                       "ST_SimplifyPreserveTopology", "ST_SymDifference", "ST_Union",
                       # 7.10. Linear Referencing
                       "ST_Line_Interpolate_Point", "ST_Line_Locate_Point", "ST_Line_Substring",
                       "*ST_locate_along_measure", "*ST_locate_between_measures", "*ST_locatebetweenelevations",
                       "*ST_addmeasure",
                       # 7.11. Long Transactions Support
                       "*addauth", "*checkauth", "*disablelongtransactions", "*enablelongtransactions", "*lockrow",
                       "*unlockrows",
                       # 7.12. Miscellaneous Functions
                       "*ST_accum", "*box2d", "*box3d", "*ST_estimated_extent", "*ST_expand", "ST_Extent", "*ST_extent3d",
                       "*find_srid", "*ST_mem_size", "*ST_point_inside_circle", "ST_XMax", "ST_XMin", "ST_YMax", "ST_YMin",
                       "ST_ZMax", "ST_ZMin",
                       # 7.13. Exceptional Functions
                       "*postgis_addbbox", "*postgis_dropbbox", "*postgis_hasbbox"
]

# constants
constants = ["null", "false", "true"]
postgis_constants = []


def getSqlDictionary(spatial=True):
    def strip_star(s):
        if s[0] == '*':
            return s.lower()[1:]
        else:
            return s.lower()

    k, c, f = list(keywords), list(constants), list(functions)

    if spatial:
        k += postgis_keywords
        f += postgis_functions
        c += postgis_constants

    return {'keyword': map(strip_star,k), 'constant': map(strip_star,c), 'function': map(strip_star,f)}

def getQueryBuilderDictionary():
    # concat functions
    def ff( l ):
        return filter( lambda s:s[0] != '*', l )
    def add_paren( l ):
        return map( lambda s:s+"(", l )
    foo = sorted(add_paren(ff( list(set.union(set(functions), set(postgis_functions))) )))
    m = sorted(add_paren(ff( math_functions )))
    agg = sorted(add_paren(ff(aggregate_functions)))
    op = ff(operators)
    s = sorted(add_paren(ff(string_functions)))
    return {'function': foo, 'math' : m, 'aggregate': agg, 'operator': op, 'string': s }
