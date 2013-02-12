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

# GENERIC SQL DICTIONARY

# keywords
keywords = [
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
"view", "when", "where"
]

# functions
functions = [
"abs", "changes", "coalesce", "glob", "ifnull", "hex", "last_insert_rowid",
"length", "like", "lower", "ltrim", "max", "min", "nullif", "quote", "random",
"randomblob", "replace", "round", "rtrim", "soundex", "total_change", "trim",
"typeof", "upper", "zeroblob", "date", "datetime", "julianday", "strftime",
"avg", "count", "group_concat", "sum", "total"
]

# constants
constants = [ "null", "false", "true" ]

def getSqlDictionary():
	return { 'keyword' : list(keywords), 'constant' : list(constants), 'function' : list(functions) }

