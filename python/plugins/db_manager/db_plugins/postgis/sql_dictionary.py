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
# TODO get them from a reference page
"abs", "changes", "coalesce", "glob", "ifnull", "hex", "last_insert_rowid", 
"length", "like", "lower", "ltrim", "max", "min", "nullif", "quote", "random", 
"randomblob", "replace", "round", "rtrim", "soundex", "total_change", "trim", 
"typeof", "upper", "zeroblob", "date", "datetime", "julianday", "strftime", 
"avg", "count", "group_concat", "sum", "total"
]
postgis_functions = [	# from http://www.postgis.org/docs/reference.html
# 7.1. PostgreSQL PostGIS Types
"box2d", "box3d", "box3d_extent", "geometry", "geometry_dump", "geography",
# 7.2. Management Functions
"addgeometrycolumn", "dropgeometrycolumn", "dropgeometrytable", "postgis_full_version", "postgis_geos_version", "postgis_libxml_version", "postgis_lib_build_date", "postgis_lib_version", "postgis_proj_version", "postgis_scripts_build_date", "postgis_scripts_installed", "postgis_scripts_released", "postgis_uses_stats", "postgis_version", "populate_geometry_columns", "probe_geometry_columns", "updategeometrysrid",
# 7.3. Geometry Constructors
"st_bdpolyfromtext", "st_bdmpolyfromtext", "st_geogfromtext", "st_geographyfromtext", "st_geogfromwkb", "st_geomcollfromtext", "st_geomfromewkb", "st_geomfromewkt", "st_geometryfromtext", "st_geomfromgml", "st_geomfromkml", "st_gmltosql", "st_geomfromtext", "st_geomfromwkb", "st_linefrommultipoint", "st_linefromtext", "st_linefromwkb", "st_linestringfromwkb", "st_makebox2d", "st_makebox3d", "st_makeline", "st_makeenvelope", "st_makepolygon", "st_makepoint", "st_makepointm", "st_mlinefromtext", "st_mpointfromtext", "st_mpolyfromtext", "st_point", "st_pointfromtext", "st_pointfromwkb", "st_polygon", "st_polygonfromtext", "st_wkbtosql", "st_wkttosql",
# 7.4. Geometry Accessors
"geometrytype", "st_boundary", "st_coorddim", "st_dimension", "st_endpoint", "st_envelope", "st_exteriorring", "st_geometryn", "st_geometrytype", "st_interiorringn", "st_isclosed", "st_isempty", "st_isring", "st_issimple", "st_isvalid", "st_isvalidreason", "st_m", "st_ndims", "st_npoints", "st_nrings", "st_numgeometries", "st_numinteriorrings", "st_numinteriorring", "st_numpoints", "st_pointn", "st_srid", "st_startpoint", "st_summary", "st_x", "st_y", "st_z", "st_zmflag",
# 7.5. Geometry Editors
"st_addpoint", "st_affine", "st_force_2d", "st_force_3d", "st_force_3dz", "st_force_3dm", "st_force_4d", "st_force_collection", "st_forcerhr", "st_linemerge", "st_collectionextract", "st_multi", "st_removepoint", "st_reverse", "st_rotate", "st_rotatex", "st_rotatey", "st_rotatez", "st_scale", "st_segmentize", "st_setpoint", "st_setsrid", "st_snaptogrid", "st_transform", "st_translate", "st_transscale",
# 7.6. Geometry Outputs
"st_asbinary", "st_asewkb", "st_asewkt", "st_asgeojson", "st_asgml", "st_ashexewkb", "st_askml", "st_assvg", "st_geohash", "st_astext",
# 7.7. Operators
# 7.8. Spatial Relationships and Measurements
"st_area", "st_azimuth", "st_centroid", "st_closestpoint", "st_contains", "st_containsproperly", "st_covers", "st_coveredby", "st_crosses", "st_linecrossingdirection", "st_disjoint", "st_distance", "st_hausdorffdistance", "st_maxdistance", "st_distance_sphere", "st_distance_spheroid", "st_dfullywithin", "st_dwithin", "st_equals", "st_hasarc", "st_intersects", "st_length", "st_length2d", "st_length3d", "st_length_spheroid", "st_length2d_spheroid", "st_length3d_spheroid", "st_longestline", "st_orderingequals", "st_overlaps", "st_perimeter", "st_perimeter2d", "st_perimeter3d", "st_pointonsurface", "st_relate", "st_shortestline", "st_touches", "st_within",
# 7.9. Geometry Processing Functions
"st_buffer", "st_buildarea", "st_collect", "st_convexhull", "st_curvetoline", "st_difference", "st_dump", "st_dumppoints", "st_dumprings", "st_intersection", "st_linetocurve", "st_memunion", "st_minimumboundingcircle", "st_polygonize", "st_shift_longitude", "st_simplify", "st_simplifypreservetopology", "st_symdifference", "st_union",
# 7.10. Linear Referencing
"st_line_interpolate_point", "st_line_locate_point", "st_line_substring", "st_locate_along_measure", "st_locate_between_measures", "st_locatebetweenelevations", "st_addmeasure",
# 7.11. Long Transactions Support
"addauth", "checkauth", "disablelongtransactions", "enablelongtransactions", "lockrow", "unlockrows",
# 7.12. Miscellaneous Functions
"st_accum", "box2d", "box3d", "st_estimated_extent", "st_expand", "st_extent", "st_extent3d", "find_srid", "st_mem_size", "st_point_inside_circle", "st_xmax", "st_xmin", "st_ymax", "st_ymin", "st_zmax", "st_zmin",
# 7.13. Exceptional Functions
"postgis_addbbox", "postgis_dropbbox", "postgis_hasbbox"
]

# constants
constants = [ "null", "false", "true" ]
postgis_constants = []

def getSqlDictionary(spatial=True):
	k, c, f = list(keywords), list(constants), list(functions)

	if spatial:
		k += postgis_keywords
		f += postgis_functions
		c += postgis_constants

	return { 'keyword' : k, 'constant' : c, 'function' : f }

