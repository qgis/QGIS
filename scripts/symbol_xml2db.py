#!/usr/bin/env python3
"""
/***************************************************************************
                               symbol_xml2db.py
                              -------------------
    begin                : 26-5-2012
    copyright            : (C) 2012 by Arunmozhi
    email                : aruntheguy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

    The script creates a sqlite3 Db for storing the symbols which will be
    shipped with QGIS by default. It then converts symbols and colorramps
    in the symbology_ng_style.xml to the database table entries.
"""
import sqlite3

from xml.dom.minidom import parse, parseString

xmlfile = "../resources/symbology-style.xml"
dbfile = "../resources/symbology-style.db"

_symbol = "CREATE TABLE symbol("\
          "id INTEGER PRIMARY KEY,"\
          "name TEXT UNIQUE,"\
          "xml TEXT,"\
          "favorite INTEGER)"

_colorramp = "CREATE TABLE colorramp("\
             "id INTEGER PRIMARY KEY,"\
             "name TEXT UNIQUE,"\
             "xml TEXT,"\
             "favorite INTEGER)"

_tag = "CREATE TABLE tag("\
       "id INTEGER PRIMARY KEY,"\
       "name TEXT)"

_tagmap = "CREATE TABLE tagmap("\
          "tag_id INTEGER NOT NULL,"\
          "symbol_id INTEGER)"

_ctagmap = "CREATE TABLE ctagmap("\
           "tag_id INTEGER NOT NULL,"\
           "colorramp_id INTEGER)"

_smartgroup = "CREATE TABLE smartgroup("\
              "id INTEGER PRIMARY KEY,"\
              "name TEXT,"\
              "xml TEXT)"

create_tables = [_symbol, _colorramp, _tag, _tagmap, _ctagmap, _smartgroup]

# Create the DB with required Schema
conn = sqlite3.connect(dbfile)
c = conn.cursor()
print "Creating tables in the Database\n"
for table in create_tables:
    try:
        c.execute(table)
        print table
    except sqlite3.OperationalError as e:
        pass
    conn.commit()

# parse the XML file &  write symbol into DB
dom = parse(xmlfile)
symbols = dom.getElementsByTagName("symbol")
for symbol in symbols:
    symbol_name = symbol.getAttribute("name")
    symbol_favorite = symbol.getAttribute("favorite")
    if not symbol_favorite:
        symbol_favorite = 0

    if '@' in symbol_name:
        parts = symbol_name.split('@')
        parent_name = parts[1]
        layerno = int(parts[2])
        c.execute("SELECT xml FROM symbol WHERE name=(?)", (parent_name,))
        symdom = parseString(c.fetchone()[0]).getElementsByTagName('symbol')[0]
        symdom.getElementsByTagName("layer")[layerno].appendChild(symbol)
        c.execute("UPDATE symbol SET xml=? WHERE name=?", (symdom.toxml(), parent_name))
    else:
        c.execute("INSERT INTO symbol VALUES (?,?,?,?)", (None, symbol_name, symbol.toxml(), symbol_favorite))
conn.commit()


# ColorRamps
colorramps = dom.getElementsByTagName("colorramp")
for ramp in colorramps:
    ramp_name = ramp.getAttribute("name")
    symbol_favorite = symbol.getAttribute("favorite")
    if not symbol_favorite:
        symbol_favorite = 0

    c.execute("INSERT INTO colorramp VALUES (?,?,?,?)", (None, ramp_name, ramp.toxml(), symbol_favorite))
conn.commit()

# Finally close the sqlite cursor
c.close()
