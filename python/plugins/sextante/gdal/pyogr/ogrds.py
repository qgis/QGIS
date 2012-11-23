import ogr

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
