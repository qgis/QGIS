# shapefile_and_friends = None
# shapefile_plus_sidecars = shapefile_and_friends("test/data/states")

def shapefile_and_friends(path):
    return dict((ext, path + "." + ext) for ext in ['shx', 'shp', 'dbf', 'prj'])
