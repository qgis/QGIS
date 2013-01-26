# -*- coding: utf-8 -*-

"""
***************************************************************************
    ogrds.py
    ---------------------
    Date                 : November 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Victor Olaya'
__date__ = 'November 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from osgeo import ogr

class OgrDs:

    def __init__(self, format, ds):
        self.open(format, ds)

    def open(self, format, ds):
        driver = ogr.GetDriverByName(format)
        self.ds = driver.CreateDataSource(ds)
        return self.ds

    def close(self):
        if self.ds is not None:
            self.ds.Destroy()

    def execute_sql(self,  sql_statement):
        poResultSet = self.ds.ExecuteSQL( sql_statement, None, None )
        if poResultSet is not None:
          self.ds.ReleaseResultSet( poResultSet )

    def select_values(self,  sql_statement):
        """Returns an array of the values of the first column in a select:
            select_values(ds, "SELECT id FROM companies LIMIT 3") => [1,2,3]
        """
        values = []
        poResultSet = self.ds.ExecuteSQL( sql_statement, None, None )
        if poResultSet is not None:
            poDefn = poResultSet.GetLayerDefn()
            poFeature = poResultSet.GetNextFeature()
            while poFeature is not None:
                for iField in range(poDefn.GetFieldCount()):
                    values.append( poFeature.GetFieldAsString( iField ) )
                poFeature = poResultSet.GetNextFeature()
            self.ds.ReleaseResultSet( poResultSet )
        return values

    def table_exists(self, table):
        exists = True
        try:
          self.ds.ExecuteSQL( "SELECT 1 FROM %s" % table, None, None )
        except:
          exists = False
        return exists
