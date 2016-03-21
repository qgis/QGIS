# -*- coding: utf-8 -*-

"""
/***************************************************************************
Name                 : DB Manager
Description          : Database manager plugin for QGIS (Oracle)
Date                 : Aug 27, 2014
copyright            : (C) 2014 by Médéric RIBREUX
email                : mederic.ribreux@gmail.com

The content of this file is based on
- PG_Manager by Martin Dobias <wonder.sk@gmail.com> (GPLv2 license)
- DB Manager by Giuseppe Sucameli <brush.tyler@gmail.com> (GPLv2 license)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
"""

__author__ = 'Médéric RIBREUX'
__date__ = 'August 2014'
__copyright__ = '(C) 2014, Médéric RIBREUX'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

# keywords
keywords = [
    # From:
    # http://docs.oracle.com/cd/B19306_01/server.102/b14200/ap_keywd.htm
    "ACCESS", "ADD", "ALL", "ALTER", "AND", "ANY", "AS", "ASC",
    "AUDIT", "BETWEEN", "BY", "CHAR", "CHECK", "CLUSTER", "COLUMN",
    "COMMENT", "COMPRESS", "CONNECT", "CREATE", "CURRENT", "DATE",
    "DECIMAL", "DEFAULT", "DELETE", "DESC", "DISTINCT", "DROP",
    "ELSE", "EXCLUSIVE", "EXISTS", "FILE", "FLOAT", "FOR", "FROM",
    "GRANT", "GROUP", "HAVING", "IDENTIFIED", "IMMEDIATE", "IN",
    "INCREMENT", "INDEX", "INITIAL", "INSERT", "INTEGER", "INTERSECT",
    "INTO", "IS", "LEVEL", "LIKE", "LOCK", "LONG", "MAXEXTENTS",
    "MINUS", "MLSLABEL", "MODE", "MODIFY", "NOAUDIT", "NOCOMPRESS",
    "NOT", "NOWAIT", "NULL", "NUMBER", "OF", "OFFLINE", "ON",
    "ONLINE", "OPTION", "OR", "ORDER", "PCTFREE", "PRIOR",
    "PRIVILEGES", "PUBLIC", "RAW", "RENAME", "RESOURCE", "REVOKE",
    "ROW", "ROWID", "ROWNUM", "ROWS", "SELECT", "SESSION", "SET",
    "SHARE", "SIZE", "SMALLINT", "START", "SUCCESSFUL", "SYNONYM",
    "SYSDATE", "TABLE", "THEN", "TO", "TRIGGER", "UID", "UNION",
    "UNIQUE", "UPDATE", "USER", "VALIDATE", "VALUES", "VARCHAR",
    "VARCHAR2", "VIEW", "WHENEVER", "WHERE", "WITH",
    # From http://docs.oracle.com/cd/B13789_01/appdev.101/a42525/apb.htm
    "ADMIN", "CURSOR", "FOUND", "MOUNT", "AFTER", "CYCLE", "FUNCTION",
    "NEXT", "ALLOCATE", "DATABASE", "GO", "NEW", "ANALYZE",
    "DATAFILE", "GOTO", "NOARCHIVELOG", "ARCHIVE", "DBA", "GROUPS",
    "NOCACHE", "ARCHIVELOG", "DEC", "INCLUDING", "NOCYCLE",
    "AUTHORIZATION", "DECLARE", "INDICATOR", "NOMAXVALUE", "AVG",
    "DISABLE", "INITRANS", "NOMINVALUE", "BACKUP", "DISMOUNT",
    "INSTANCE", "NONE", "BEGIN", "DOUBLE", "INT", "NOORDER", "BECOME",
    "DUMP", "KEY", "NORESETLOGS", "BEFORE", "EACH", "LANGUAGE",
    "NORMAL", "BLOCK", "ENABLE", "LAYER", "NOSORT", "BODY", "END",
    "LINK", "NUMERIC", "CACHE", "ESCAPE", "LISTS", "OFF", "CANCEL",
    "EVENTS", "LOGFILE", "OLD", "CASCADE", "EXCEPT", "MANAGE", "ONLY",
    "CHANGE", "EXCEPTIONS", "MANUAL", "OPEN", "CHARACTER", "EXEC",
    "MAX", "OPTIMAL", "CHECKPOINT", "EXPLAIN", "MAXDATAFILES", "OWN",
    "CLOSE", "EXECUTE", "MAXINSTANCES", "PACKAGE", "COBOL", "EXTENT",
    "MAXLOGFILES", "PARALLEL", "COMMIT", "EXTERNALLY",
    "MAXLOGHISTORY", "PCTINCREASE", "COMPILE", "FETCH",
    "MAXLOGMEMBERS", "PCTUSED", "CONSTRAINT", "FLUSH", "MAXTRANS",
    "PLAN", "CONSTRAINTS", "FREELIST", "MAXVALUE", "PLI", "CONTENTS",
    "FREELISTS", "MIN", "PRECISION", "CONTINUE", "FORCE",
    "MINEXTENTS", "PRIMARY", "CONTROLFILE", "FOREIGN", "MINVALUE",
    "PRIVATE", "COUNT", "FORTRAN", "MODULE", "PROCEDURE", "PROFILE",
    "SAVEPOINT", "SQLSTATE", "TRACING", "QUOTA", "SCHEMA",
    "STATEMENT_ID", "TRANSACTION", "READ", "SCN", "STATISTICS",
    "TRIGGERS", "REAL", "SECTION", "STOP", "TRUNCATE", "RECOVER",
    "SEGMENT", "STORAGE", "UNDER", "REFERENCES", "SEQUENCE", "SUM",
    "UNLIMITED", "REFERENCING", "SHARED", "SWITCH", "UNTIL",
    "RESETLOGS", "SNAPSHOT", "SYSTEM", "USE", "RESTRICTED", "SOME",
    "TABLES", "USING", "REUSE", "SORT", "TABLESPACE", "WHEN", "ROLE",
    "SQL", "TEMPORARY", "WRITE", "ROLES", "SQLCODE", "THREAD", "WORK",
    "ROLLBACK", "SQLERROR", "TIME", "ABORT", "BETWEEN", "CRASH",
    "DIGITS", "ACCEPT", "BINARY_INTEGER", "CREATE", "DISPOSE",
    "ACCESS", "BODY", "CURRENT", "DISTINCT", "ADD", "BOOLEAN",
    "CURRVAL", "DO", "ALL", "BY", "CURSOR", "DROP", "ALTER", "CASE",
    "DATABASE", "ELSE", "AND", "CHAR", "DATA_BASE", "ELSIF", "ANY",
    "CHAR_BASE", "DATE", "END", "ARRAY", "CHECK", "DBA", "ENTRY",
    "ARRAYLEN", "CLOSE", "DEBUGOFF", "EXCEPTION", "AS", "CLUSTER",
    "DEBUGON", "EXCEPTION_INIT", "ASC", "CLUSTERS", "DECLARE",
    "EXISTS", "ASSERT", "COLAUTH", "DECIMAL", "EXIT", "ASSIGN",
    "COLUMNS", "DEFAULT", "FALSE", "AT", "COMMIT", "DEFINITION",
    "FETCH", "AUTHORIZATION", "COMPRESS", "DELAY", "FLOAT", "AVG",
    "CONNECT", "DELETE", "FOR", "BASE_TABLE", "CONSTANT", "DELTA",
    "FORM", "BEGIN", "COUNT", "DESC", "FROM", "FUNCTION", "NEW",
    "RELEASE", "SUM", "GENERIC", "NEXTVAL", "REMR", "TABAUTH", "GOTO",
    "NOCOMPRESS", "RENAME", "TABLE", "GRANT", "NOT", "RESOURCE",
    "TABLES", "GROUP", "NULL", "RETURN", "TASK", "HAVING", "NUMBER",
    "REVERSE", "TERMINATE", "IDENTIFIED", "NUMBER_BASE", "REVOKE",
    "THEN", "IF", "OF", "ROLLBACK", "TO", "IN", "ON", "ROWID", "TRUE",
    "INDEX", "OPEN", "ROWLABEL", "TYPE", "INDEXES", "OPTION",
    "ROWNUM", "UNION", "INDICATOR", "OR", "ROWTYPE", "UNIQUE",
    "INSERT", "ORDER", "RUN", "UPDATE", "INTEGER", "OTHERS",
    "SAVEPOINT", "USE", "INTERSECT", "OUT", "SCHEMA", "VALUES",
    "INTO", "PACKAGE", "SELECT", "VARCHAR", "IS", "PARTITION",
    "SEPARATE", "VARCHAR2", "LEVEL", "PCTFREE", "SET", "VARIANCE",
    "LIKE", "POSITIVE", "SIZE", "VIEW", "LIMITED", "PRAGMA",
    "SMALLINT", "VIEWS", "LOOP", "PRIOR", "SPACE", "WHEN", "MAX",
    "PRIVATE", "SQL", "WHERE", "MIN", "PROCEDURE", "SQLCODE", "WHILE",
    "MINUS", "PUBLIC", "SQLERRM", "WITH", "MLSLABEL", "RAISE",
    "START", "WORK", "MOD", "RANGE", "STATEMENT", "XOR", "MODE",
    "REAL", "STDDEV", "NATURAL", "RECORD", "SUBTYPE"
]

oracle_spatial_keywords = []

# SQL functions
# other than math/string/aggregate/date/conversion/xml/data mining
functions = [
    # FROM
    # https://docs.oracle.com/cd/B19306_01/server.102/b14200/functions001.htm
    "CAST", "COALESCE", "DECODE", "GREATEST", "LEAST", "LNNVL",
    "NULLIF", "NVL", "NVL2", "SET", "UID", "USER", "USERENV"
]

# SQL math functions
math_functions = [
    'ABS', 'ACOS', 'ASIN', 'ATAN', 'ATAN2', 'BITAND', 'CEIL', 'COS',
    'COSH', 'EXP', 'FLOOR', 'LN', 'LOG', 'MOD', 'NANVL', 'POWER',
    'REMAINDER', 'ROUND', 'SIGN', 'SIN', 'SINH', 'SQRT', 'TAN',
    'TANH', 'TRUNC', 'WIDTH_BUCKET'
]

# Strings functions
string_functions = [
    'CHR', 'CONCAT', 'INITCAP', 'LOWER', 'LPAD', 'LTRIM', 'NLS_INITCAP',
    'NLS_LOWER', 'NLSSORT', 'NLS_UPPER', 'REGEXP_REPLACE', 'REGEXP_SUBSTR',
    'REPLACE', 'RPAD', 'RTRIM', 'SOUNDEX', 'SUBSTR', 'TRANSLATE', 'TREAT',
    'TRIM', 'UPPER', 'ASCII', 'INSTR', 'LENGTH', 'REGEXP_INSTR'
]

# Aggregate functions
aggregate_functions = [
    'AVG', 'COLLECT', 'CORR', 'COUNT', 'COVAR_POP', 'COVAR_SAMP', 'CUME_DIST',
    'DENSE_RANK', 'FIRST', 'GROUP_ID', 'GROUPING', 'GROUPING_ID',
    'LAST', 'MAX', 'MEDIAN', 'MIN', 'PERCENTILE_CONT',
    'PERCENTILE_DISC', 'PERCENT_RANK', 'RANK',
    'STATS_BINOMIAL_TEST', 'STATS_CROSSTAB', 'STATS_F_TEST',
    'STATS_KS_TEST', 'STATS_MODE', 'STATS_MW_TEST',
    'STATS_ONE_WAY_ANOVA', 'STATS_WSR_TEST', 'STDDEV',
    'STDDEV_POP', 'STDDEV_SAMP', 'SUM', 'SYS_XMLAGG', 'VAR_POP',
    'VAR_SAMP', 'VARIANCE', 'XMLAGG'
]

oracle_spatial_functions = [
    # From http://docs.oracle.com/cd/B19306_01/appdev.102/b14255/toc.htm
    # Spatial operators
    "SDO_ANYINTERACT", "SDO_CONTAINS", "SDO_COVEREDBY", "SDO_COVERS",
    "SDO_EQUAL", "SDO_FILTER", "SDO_INSIDE", "SDO_JOIN", "SDO_NN",
    "SDO_NN_DISTANCE", "SDO_ON", "SDO_OVERLAPBDYDISJOINT",
    "SDO_OVERLAPBDYINTERSECT", "SDO_OVERLAPS", "SDO_RELATE",
    "SDO_TOUCH", "SDO_WITHIN_DISTANCE",
    # SPATIAL AGGREGATE FUNCTIONS
    "SDO_AGGR_CENTROID", "SDO_AGGR_CONCAT_LINES",
    "SDO_AGGR_CONVEXHULL", "SDO_AGGR_LRS_CONCAT", "SDO_AGGR_MBR",
    "SDO_AGGR_UNION",
    # COORDINATE SYSTEM TRANSFORMATION (SDO_CS)
    "SDO_CS.ADD_PREFERENCE_FOR_OP", "SDO_CS.CONVERT_NADCON_TO_XML",
    "SDO_CS.CONVERT_NTV2_TO_XML", "SDO_CS.CONVERT_XML_TO_NADCON",
    "SDO_CS.CONVERT_XML_TO_NTV2", "SDO_CS.CREATE_CONCATENATED_OP",
    "SDO_CS.CREATE_OBVIOUS_EPSG_RULES",
    "SDO_CS.CREATE_PREF_CONCATENATED_OP",
    "SDO_CS.DELETE_ALL_EPSG_RULES", "SDO_CS.DELETE_OP",
    "SDO_CS.DETERMINE_CHAIN", "SDO_CS.DETERMINE_DEFAULT_CHAIN",
    "SDO_CS.FIND_GEOG_CRS", "SDO_CS.FIND_PROJ_CRS",
    "SDO_CS.FROM_OGC_SIMPLEFEATURE_SRS", "SDO_CS.FROM_USNG",
    "SDO_CS.MAP_EPSG_SRID_TO_ORACLE",
    "SDO_CS.MAP_ORACLE_SRID_TO_EPSG",
    "SDO_CS.REVOKE_PREFERENCE_FOR_OP",
    "SDO_CS.TO_OGC_SIMPLEFEATURE_SRS", "SDO_CS.TO_USNG",
    "SDO_CS.TRANSFORM", "SDO_CS.TRANSFORM_LAYER",
    "SDO_CS.UPDATE_WKTS_FOR_ALL_EPSG_CRS",
    "SDO_CS.UPDATE_WKTS_FOR_EPSG_CRS",
    "SDO_CS.UPDATE_WKTS_FOR_EPSG_DATUM",
    "SDO_CS.UPDATE_WKTS_FOR_EPSG_ELLIPS",
    "SDO_CS.UPDATE_WKTS_FOR_EPSG_OP",
    "SDO_CS.UPDATE_WKTS_FOR_EPSG_PARAM",
    "SDO_CS.UPDATE_WKTS_FOR_EPSG_PM", "SDO_CS.VALIDATE_WKT",
    "SDO_CS.VIEWPORT_TRANSFORM",
    # GEOCODING (SDO_GCDR)
    "SDO_GCDR.GEOCODE", "SDO_GCDR.GEOCODE_ADDR",
    "SDO_GCDR.GEOCODE_ADDR_ALL", "SDO_GCDR.GEOCODE_ALL",
    "SDO_GCDR.GEOCODE_AS_GEOMETRY", "SDO_GCDR.REVERSE_GEOCODE",
    # GEOMETRY (SDO_GEOM)
    "SDO_GEOM.RELATE", "SDO_GEOM.SDO_ARC_DENSIFY",
    "SDO_GEOM.SDO_AREA", "SDO_GEOM.SDO_BUFFER",
    "SDO_GEOM.SDO_CENTROID", "SDO_GEOM.SDO_CONVEXHULL",
    "SDO_GEOM.SDO_DIFFERENCE", "SDO_GEOM.SDO_DISTANCE",
    "SDO_GEOM.SDO_INTERSECTION", "SDO_GEOM.SDO_LENGTH",
    "SDO_GEOM.SDO_MAX_MBR_ORDINATE", "SDO_GEOM.SDO_MBR",
    "SDO_GEOM.SDO_MIN_MBR_ORDINATE", "SDO_GEOM.SDO_POINTONSURFACE",
    "SDO_GEOM.SDO_UNION", "SDO_GEOM.SDO_XOR",
    "SDO_GEOM.VALIDATE_GEOMETRY_WITH_CONTEXT",
    "SDO_GEOM.VALIDATE_LAYER_WITH_CONTEXT",
    "SDO_GEOM.WITHIN_DISTANCE",
    # LINEAR REFERENCING SYSTEM (SDO_LRS)
    "SDO_LRS.CLIP_GEOM_SEGMENT", "SDO_LRS.CONCATENATE_GEOM_SEGMENTS",
    "SDO_LRS.CONNECTED_GEOM_SEGMENTS",
    "SDO_LRS.CONVERT_TO_LRS_DIM_ARRAY", "SDO_LRS.CONVERT_TO_LRS_GEOM",
    "SDO_LRS.CONVERT_TO_LRS_LAYER",
    "SDO_LRS.CONVERT_TO_STD_DIM_ARRAY", "SDO_LRS.CONVERT_TO_STD_GEOM",
    "SDO_LRS.CONVERT_TO_STD_LAYER", "SDO_LRS.DEFINE_GEOM_SEGMENT",
    "SDO_LRS.DYNAMIC_SEGMENT", "SDO_LRS.FIND_LRS_DIM_POS",
    "SDO_LRS.FIND_MEASURE", "SDO_LRS.FIND_OFFSET",
    "SDO_LRS.GEOM_SEGMENT_END_MEASURE", "SDO_LRS.GEOM_SEGMENT_END_PT",
    "SDO_LRS.GEOM_SEGMENT_LENGTH",
    "SDO_LRS.GEOM_SEGMENT_START_MEASURE",
    "SDO_LRS.GEOM_SEGMENT_START_PT", "SDO_LRS.GET_MEASURE",
    "SDO_LRS.GET_NEXT_SHAPE_PT", "SDO_LRS.GET_NEXT_SHAPE_PT_MEASURE",
    "SDO_LRS.GET_PREV_SHAPE_PT", "SDO_LRS.GET_PREV_SHAPE_PT_MEASURE",
    "SDO_LRS.IS_GEOM_SEGMENT_DEFINED",
    "SDO_LRS.IS_MEASURE_DECREASING", "SDO_LRS.IS_MEASURE_INCREASING",
    "SDO_LRS.IS_SHAPE_PT_MEASURE", "SDO_LRS.LOCATE_PT",
    "SDO_LRS.LRS_INTERSECTION", "SDO_LRS.MEASURE_RANGE",
    "SDO_LRS.MEASURE_TO_PERCENTAGE", "SDO_LRS.OFFSET_GEOM_SEGMENT",
    "SDO_LRS.PERCENTAGE_TO_MEASURE", "SDO_LRS.PROJECT_PT",
    "SDO_LRS.REDEFINE_GEOM_SEGMENT", "SDO_LRS.RESET_MEASURE",
    "SDO_LRS.REVERSE_GEOMETRY", "SDO_LRS.REVERSE_MEASURE",
    "SDO_LRS.SET_PT_MEASURE", "SDO_LRS.SPLIT_GEOM_SEGMENT",
    "SDO_LRS.TRANSLATE_MEASURE", "SDO_LRS.VALID_GEOM_SEGMENT",
    "SDO_LRS.VALID_LRS_PT", "SDO_LRS.VALID_MEASURE",
    "SDO_LRS.VALIDATE_LRS_GEOMETRY",
    # SDO_MIGRATE
    "SDO_MIGRATE.TO_CURRENT",
    # SPATIAL ANALYSIS AND MINING (SDO_SAM)
    "SDO_SAM.AGGREGATES_FOR_GEOMETRY", "SDO_SAM.AGGREGATES_FOR_LAYER",
    "SDO_SAM.BIN_GEOMETRY", "SDO_SAM.BIN_LAYER",
    "SDO_SAM.COLOCATED_REFERENCE_FEATURES",
    "SDO_SAM.SIMPLIFY_GEOMETRY", "SDO_SAM.SIMPLIFY_LAYER",
    "SDO_SAM.SPATIAL_CLUSTERS", "SDO_SAM.TILED_AGGREGATES",
    "SDO_SAM.TILED_BINS",
    # TUNING (SDO_TUNE)
    "SDO_TUNE.AVERAGE_MBR", "SDO_TUNE.ESTIMATE_RTREE_INDEX_SIZE",
    "SDO_TUNE.EXTENT_OF", "SDO_TUNE.MIX_INFO",
    "SDO_TUNE.QUALITY_DEGRADATION",
    # UTILITY (SDO_UTIL)
    "SDO_UTIL.APPEND", "SDO_UTIL.CIRCLE_POLYGON",
    "SDO_UTIL.CONCAT_LINES", "SDO_UTIL.CONVERT_UNIT",
    "SDO_UTIL.ELLIPSE_POLYGON", "SDO_UTIL.EXTRACT",
    "SDO_UTIL.FROM_WKBGEOMETRY", "SDO_UTIL.FROM_WKTGEOMETRY",
    "SDO_UTIL.GETNUMELEM", "SDO_UTIL.GETNUMVERTICES",
    "SDO_UTIL.GETVERTICES", "SDO_UTIL.INITIALIZE_INDEXES_FOR_TTS",
    "SDO_UTIL.POINT_AT_BEARING", "SDO_UTIL.POLYGONTOLINE",
    "SDO_UTIL.PREPARE_FOR_TTS", "SDO_UTIL.RECTIFY_GEOMETRY",
    "SDO_UTIL.REMOVE_DUPLICATE_VERTICES",
    "SDO_UTIL.REVERSE_LINESTRING", "SDO_UTIL.SIMPLIFY",
    "SDO_UTIL.TO_GMLGEOMETRY", "SDO_UTIL.TO_WKBGEOMETRY",
    "SDO_UTIL.TO_WKTGEOMETRY", "SDO_UTIL.VALIDATE_WKBGEOMETRY",
    "SDO_UTIL.VALIDATE_WKTGEOMETRY"
]

# Oracle Operators
operators = [
    ' AND ', ' OR ', '||', ' < ', ' <= ', ' > ', ' >= ', ' = ',
    ' <> ', '!=', '^=', ' IS ', ' IS NOT ', ' IN ', ' ANY ', ' SOME ',
    ' NOT IN ', ' LIKE ', ' GLOB ', ' MATCH ', ' REGEXP ',
    ' BETWEEN x AND y ', ' NOT BETWEEN x AND y ', ' EXISTS ',
    ' IS NULL ', ' IS NOT NULL', ' ALL ', ' NOT ',
    ' CASE {column} WHEN {value} THEN {value} '
]


# constants
constants = ["null", "false", "true"]
oracle_spatial_constants = []


def getSqlDictionary(spatial=True):
    k, c, f = list(keywords), list(constants), list(functions)

    if spatial:
        k += oracle_spatial_keywords
        f += oracle_spatial_functions
        c += oracle_spatial_constants

    return {'keyword': k, 'constant': c, 'function': f}


def getQueryBuilderDictionary():
    # concat functions
    def ff(l):
        return [s for s in l if s[0] != '*']

    def add_paren(l):
        return [s + "(" for s in l]

    foo = sorted(
        add_paren(
            ff(
                list(
                    set.union(set(functions),
                              set(oracle_spatial_functions))))))
    m = sorted(add_paren(ff(math_functions)))
    agg = sorted(add_paren(ff(aggregate_functions)))
    op = ff(operators)
    s = sorted(add_paren(ff(string_functions)))
    return {'function': foo, 'math': m, 'aggregate': agg,
            'operator': op, 'string': s}
