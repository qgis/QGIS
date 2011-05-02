#!/usr/bin/env python

############################################################################
#
# MODULE:       qgis.v.out.ogr.pg.py
# AUTHOR(S):    Radim Blazek
#
# PURPOSE:      Export a vectore to PostGIS (PostgreSQL) database table
# COPYRIGHT:    (C) 2009 by Radim Blazek
#
#               This program is free software under the GNU General Public
#               License (>=v2). Read the file COPYING that comes with GRASS
#               for details.
#
#############################################################################

#%Module
#% description: Export vector to PostGIS (PostgreSQL) database table.
#% keywords: vector, export, database
#%End

#%option
#% key: input
#% type: string
#% gisprompt: old,vector,vector
#% key_desc : name
#% description: Name of input vector map
#% required : yes
#%end

#%option
#% key: layer
#% type: integer
#% description: Number of input layer
#% answer: 1
#% required : yes
#%end

#%option
#% key: type
#% type: string
#% description: Feature type(s)
#% options: point,kernel,centroid,line,boundary,area,face
#% multiple: yes
#% required : yes
#%end

#%option
#% key: olayer
#% type: string
#% description: Name of output database table
#% required : yes
#%end

#%option
#% key: host
#% type: string
#% label: Host
#% description: Host name of the machine on which the server is running. 
#% required : no
#%end

#%option
#% key: port
#% type: integer
#% label: Port
#% description: TCP port on which the server is listening, usually 5432.
#% required : no
#%end

#%option
#% key: database
#% type: string
#% key_desc : name
#% gisprompt: old_dbname,dbname,dbname
#% label: Database
#% description: Database name
#% required : yes
#%end

# AFAIK scheme is not supported well by OGR
##%option
##% key: schema
##% type: string
##% label: Schema
##% description: Database schema.
##% required : no
##%end

#%option
#% key: user
#% type: string
#% label: User
#% description: Connect to the database as the user username instead of the  default. 
#% required : no
#%end

#%option
#% key: password
#% type: string
#% label: Password
#% description: Password will be stored in file!
#% required : no
#%end

#%flag
#% key: c
#% description: to export features with category (labeled) only. Otherwise all features are exported
#%end

import sys
import os
import string
try:
    from grass.script import core as grass
except ImportError:
    import grass
except:
    raise Exception ("Cannot find 'grass' Python module. Python is supported by GRASS from version >= 6.4" )

def main():
    input = options['input']
    layer = options['layer']
    type = options['type']
    olayer = options['olayer']
    host = options['host']
    port = options['port']
    database = options['database']
    #schema = options['schema']
    user = options['user']
    password = options['password']

    # Construct dsn string
    dsn = "PG:dbname=" + database
    if host: dsn += " host=" + host
    if port: dsn += " port=" + port
    if user: dsn += " user=" + user
    if password: dsn += " password=" + password

    if grass.run_command('v.out.ogr', flags=flags_string, input=input, layer=layer, type=type, format="PostgreSQL", dsn=dsn, olayer=olayer ) != 0:
         grass.fatal("Cannot export vector to database.")
	
if __name__ == "__main__":
    options, flags = grass.parser()
    flags_string = "".join([k for k in flags.keys() if flags[k] and k != 'r'])
    main()
