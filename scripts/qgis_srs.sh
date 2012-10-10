#!/bin/sh
###########################################################################
#    qgis_srs.sh
#    ---------------------
#    Date                 : August 2009
#    Copyright            : (C) 2009 by Magnus Homann
#    Email                : magnus at homann dot se
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################


# AUTHOR:       Maciej Sieczka, msieczka@sieczka.org, http://www.sieczka.org
#
# PURPOSE:      Create a QGIS srs.db-compliant SQL script with SRS, ellipsoid
#               and projections defs based on the output of installed PROJ.4 and
#               GDAL.
#
# VERSION:      1.1.0, 2009.03.19
#
# COPYRIGHT:    (c) 2008,2009 Maciej Sieczka
#
# LICENSE:      This program is free software under the GNU General Public
#               License (>=v2).

# CHANGELOG:
#
# 1.1.0:
# - Reorganize the code into functions.
# - Support qgis.db convenient update too.
# - Workaround the issue http://trac.osgeo.org/gdal/ticket/2900.
#
# 1.0.3: Minor cosmetics in comments.
#
# 1.0.2: Replace 'latlon' and 'lonlat' in the `proj -le` output, so that QGIS
#        can parse the 'tbl_projection' table to provide the GCSs list in the
#        'Projection' dialog (BTW, the dialog should be called 'Coordinate
#        system' actually, as a projection is only a component of a cs).
#        More comments.
#
# 1.0.1: Typos in comments fixed.
#
# 1.0:   First public release.

# USAGE:  1. qgis_srs.sh --full/--tmpl > output.sql
#         2. import output.sql into SQLite Database Browser
#         3. save as a new dbase, name it srs.db (--full) or qgis.db (--tmpl),
#            use with QGIS

# DETAILS:
#
# The script creates an SQL plain text dump that can be imported into
# SQlite db eg. using SQLite Database Browser's "Import > Database from
# SQL file" tool.
#
# The ellipsoids (tbl_ellipsoid) and projections (tbl_projection) tables
# are created parsing the proj command output. I chose this approach,
# because looking at the original tables content it seems the original
# tables were created the same way.
#
# The tbl_srs table is created parsing the epsg_tr.py output. EPSG codes
# to process are taken from the installed GDAL's pcs.csv and gcs.csv files.
#
# Tables structure and final SQL statements creating the view and indices
# were copied from the original srs.db shipped with QGIS trunk r8544,
# after dumping it to a plain text format with SQLite Database Browser.

### FUNCTIONS ###

mk_tbl_bookmarks ()
{
# Create bookmarks table:

echo "CREATE TABLE tbl_bookmarks (
  bookmark_id integer PRIMARY KEY,
  name varchar(255) NOT NULL,
  project_name varchar(32),
  xmin double,
  ymin double,
  xmax double,
  ymax double,
  projection_srid integer
);"
}

mk_tbl_ellps ()
{
# Create ellipsoids table:

echo "CREATE TABLE tbl_ellipsoid (
  acronym char(20) NOT NULL default '',
  name char(255) NOT NULL default '',
  radius char(255) NOT NULL default '',
  parameter2 char(255) NOT NULL default '',
  PRIMARY KEY (acronym)
);"
}

pop_tbl_ellps ()
{
# Populate ellipsoids table. Care about (possible) apostrophes in strings, which
# would brake the SQL syntax, as the "'" is also a string separator:

proj -le | sed 's/^ *//g' | tr -d "\t" | sed "s/  */ /g" | sed "s/'/''/g" | awk 'BEGIN {sep="'\'','\''"} NF>4 {printf $1 sep $4; for (i=5;i<NF+1;i++) {printf " "$i} print sep $2 sep $3} NF<5 {print $1 sep $4 sep $2 sep $3}' | while read i; do
 echo "INSERT INTO tbl_ellipsoid VALUES('"${i}"');"
done
}

mk_tbl_projs ()
{
# Create projections table:

echo "CREATE TABLE tbl_projection (
  acronym varchar(20) NOT NULL PRIMARY KEY,
  name varchar(255) NOT NULL default '',
  notes varchar(255) NOT NULL default '',
  parameters varchar(255) NOT NULL default ''
);"
}

pop_tbl_projs ()
{
# Populate projections table:

# Process each proj4 projection acronym...

for i in `proj -l | cut -d" " -f1 | sed -e 's/lonlat/longlat/' -e 's/latlon/latlong/'` ; do

 #...to extract it's parameters, making sure not more than 4 fields are created...

 proj=`proj -l=$i | tr -d "\t" | sed 's/^ *//g' | sed 's/ : /\n/' | sed "s/'/''/g" | awk '{print "'\''"$0"'\''"}' | tr "\n" "," | sed 's/,$/\n/' | sed "s/','/ /4g"`

 #...count the number of parameters...

 proj_nf=`echo $proj | awk -F"','" '{print NF}'`

 #...if only 3 (3 or 4 are possible) add an empty 4th one.

 if [ $proj_nf -eq 3 ] ; then
   proj=${proj}",''"
 fi

 # Create an SQL command for each proj:

 echo "INSERT INTO tbl_projection VALUES("${proj}");"

done
}

mk_tbl_srss_srs ()
{
# Create SRSs table for srs.db:

echo "CREATE TABLE tbl_srs (
  srs_id INTEGER PRIMARY KEY,
  description text NOT NULL,
  projection_acronym text NOT NULL,
  ellipsoid_acronym NOT NULL,
  parameters text NOT NULL,
  srid integer NOT NULL,
  epsg integer NOT NULL,
  is_geo integer NOT NULL
);"
}

mk_tbl_srss_qgis ()
{
# Create SRSs table for qgis.db:

echo "CREATE TABLE tbl_srs (
  srs_id INTEGER PRIMARY KEY,
  description text NOT NULL,
  projection_acronym text NOT NULL,
  ellipsoid_acronym NOT NULL,
  parameters text NOT NULL,
  srid integer NULL,
  auth_name varchar NULL,
  auth_id varchar NULL,
  is_geo integer NOT NULL
);"
}

pop_tbl_srss ()
{
# Populate SRSs table:

gdal_share=`gdal-config --datadir`
no=0

# Extract projected SRSs from the installed GDAL pcs.csv file:

#Find valid EPSG numbers parsing GDAL's pcs.csv:
for i in `awk 'NR>1' ${gdal_share}/pcs.csv | cut -d, -f1`; do

  raw=`epsg_tr.py -proj4 $i 2>&1 | tr "\n" " " | sed 's/  <> $//' | grep -v "^ERROR 6: "`

  if [ -n "$raw" ]; then

   no=`expr $no + 1`
   name=`echo $raw | sed 's/^# //' | grep -o "^.\{1,\} <[[:digit:]]\{1,\}>" | sed 's/ <[[:digit:]]\{1,\}>//' | sed "s/'/''/g"`
   proj=`echo $raw | grep -o "+proj=[^[:space:]]\{1,\}" | cut -d"=" -f2`
   ellps=`echo $raw | grep -o "+ellps=[^[:space:]]\{1,\}" | cut -d"=" -f2`
   srs=`echo $raw | grep -o "+proj.\{1,\} +no_defs"`
   epsg=`echo $raw | grep -o ' <[[:digit:]]\{1,\}> ' | sed 's/[^[:digit:]]//g'`
   isgeo=0

   echo "INSERT INTO tbl_srs VALUES(${no},'${name}','${proj}','${ellps}','${srs}',${epsg},${epsg},${isgeo});"

  fi

done

# Extract un-projected SRSs from the installed GDAL gcs.csv file:

#Find valid EPSG numbers parsing GDAL's gcs.csv:
for i in `awk 'NR>1' ${gdal_share}/gcs.csv | cut -d, -f1`; do

  raw=`epsg_tr.py -proj4 $i 2>&1 | tr "\n" " " | sed 's/  <> $//' | grep -v "^ERROR 6: "`

  if [ -n "$raw" ]; then

   no=`expr $no + 1`
   name=`echo $raw | sed 's/^# //' | grep -o "^.\{1,\} <[[:digit:]]\{1,\}>" | sed 's/ <[[:digit:]]\{1,\}>//' | sed "s/'/''/g"`
   proj=`echo $raw | grep -o "+proj=[^[:space:]]\{1,\}" | cut -d"=" -f2`
   ellps=`echo $raw | grep -o "+ellps=[^[:space:]]\{1,\}" | cut -d"=" -f2`
   srs=`echo $raw | grep -o "+proj.\{1,\} +no_defs"`
   epsg=`echo $raw | grep -o ' <[[:digit:]]\{1,\}> ' | sed 's/[^[:digit:]]//g'`
   isgeo=1

   echo "INSERT INTO tbl_srs VALUES(${no},'${name}','${proj}','${ellps}','${srs}',${epsg},${epsg},${isgeo});"

  fi

done
}

mk_view ()
{
# Final SQL statements:

echo "CREATE VIEW vw_srs as
   select a.description as description,
          a.srs_id as srs_id,
          a.is_geo as is_geo,
          b.name as name,
          a.parameters as parameters,
          a.auth_name as auth_name,
          a.auth_id as auth_id
   from tbl_srs a
     inner join tbl_projection b
     on a.projection_acronym=b.acronym
   order by
     b.name, a.description;"
}

usage ()
{
echo "
Usage:

--qgis Create a database to be used as the 'qgis.db' upgraded replacement.
--srs  Create a database to be used as the 'srs.db' upgraded replacement.
"
}

### DO IT ###

if [ "$1" = "--qgis" ]; then
  echo "BEGIN TRANSACTION;"
  mk_tbl_bookmarks
  mk_tbl_ellps; pop_tbl_ellps
  mk_tbl_projs; pop_tbl_projs
  mk_tbl_srss_qgis
  mk_view
  echo "COMMIT;"

elif [ "$1" = "--srs" ]; then
  echo "BEGIN TRANSACTION;"
  mk_tbl_ellps; pop_tbl_ellps
  mk_tbl_projs; pop_tbl_projs
  mk_tbl_srss_srs; pop_tbl_srss
  mk_view
  echo "CREATE UNIQUE INDEX idx_srsauthid on tbl_srs(auth_name,auth_id);
CREATE UNIQUE INDEX idx_srssrid on tbl_srs(srid);
COMMIT;"

else
  usage
fi

