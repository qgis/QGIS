#!/usr/bin/env python

############################################################################
#
# MODULE:       qgis.db.connect-login.py
# AUTHOR(S):    Radim Blazek
#
# PURPOSE:      Connect to Postgresql
# COPYRIGHT:    (C) 2009 by Radim Blazek
#
#               This program is free software under the GNU General Public
#               License (>=v2). Read the file COPYING that comes with GRASS
#               for details.
#
#############################################################################

#%Module
#% description: Make connection to PostgreSQL database and login.
#% keywords: database
#%End

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

#%option
#% key: schema
#% type: string
#% label: Schema
#% description: Database schema.
#% required : no
#%end

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
    host = options['host']
    port = options['port']
    database = options['database']
    schema = options['schema']
    user = options['user']
    password = options['password']

    # Test connection
    conn = "dbname=" + database
    if host: conn += ",host=" + host
    if port: conn += ",port=" + port

    # Unfortunately we cannot test untill user/password is set 
    if user or password:
        print "Setting login (db.login) ... "
        sys.stdout.flush()
        if grass.run_command('db.login', driver = "pg", database = conn, user = user, password = password) != 0:
	    grass.fatal("Cannot login")

    # Try to connect
    print "Testing connection ..."
    sys.stdout.flush()
    if grass.run_command('db.select', quiet = True, flags='c', driver= "pg", database=conn, sql="select version()" ) != 0:
	if user or password:
	    print "Deleting login (db.login) ..."
	    sys.stdout.flush()
	    if grass.run_command('db.login', quiet = True, driver = "pg", database = conn, user = "", password = "") != 0:
		print "Cannot delete login."
		sys.stdout.flush()
        grass.fatal("Cannot connect to database.")
  
    if grass.run_command('db.connect', driver = "pg", database = conn, schema = schema) != 0:
        grass.fatal("Cannot connect to database.")
	
if __name__ == "__main__":
    options, flags = grass.parser()
    main()
