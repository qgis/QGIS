# -*- coding: utf-8 -*-

"""
***************************************************************************
    SagaGroupNameDecorator.py
    ---------------------
    Date                 : August 2012
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
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

groups = {'grid_analysis': 'Raster analysis',
          'grid_calculus': 'Raster calculus',
          'grid_calculus_bsl': 'Raster calculus',
          'grid_discretisation': 'Raster analysis',
          'grid_filter': 'Raster filter',
          'grid_gridding': 'Raster creation tools',
          'grid_spline': 'Raster creation tools',
          'grid_tools': 'Raster tools',
          'grid_visualisation': 'Raster visualization',
          'imagery_classification': 'Image analysis',
          'imagery_rga': 'Image analysis',
          'imagery_segmentation': 'Image analysis',
          'imagery_tools': 'Image analysis',
          'io_esri_e00': 'I/O',
          'io_gdal': 'I/O',
          'io_gps': 'I/O',
          'io_grid': 'I/O',
          'io_grid_grib2': 'I/O',
          'io_grid_image': 'I/O',
          'io_odbc': 'I/O',
          'io_shapes': 'I/O',
          'io_shapes_dxf': 'I/O',
          'io_shapes_las': 'I/O',
          'io_table': 'I/O',
          'pj_georeference': 'Georeferencing',
          'pj_geotrans': 'Projections and Transformations',
          'pj_proj4': 'Projections and Transformations',
          'pointcloud_tools': 'Point clouds',
          'shapes_grid': 'Vector to raster',
          'shapes_lines': 'Vector line tools',
          'shapes_points': 'Vector point tools',
          'shapes_polygons': 'Vector polygon tools',
          'shapes_tools': 'Vector general tools',
          'shapes_transect': 'Vector general tools',
          'sim_cellular_automata': 'Simulation',
          'sim_ecosystems_hugget': 'Simulation',
          'sim_fire_spreading': 'Simulation',
          'sim_hydrology': 'Simulation',
          'statistics_grid': 'Geostatistics',
          'statistics_kriging': 'Raster creation tools',
          'statistics_points': 'Geostatistics',
          'statistics_regression': 'Geostatistics',
          'ta_channels': 'Terrain Analysis - Channels',
          'ta_compound': 'Terrain Analysis - Morphometry',
          'ta_hydrology': 'Terrain Analysis - Hydrology',
          'ta_lighting': 'Terrain Analysis - Lighting',
          'ta_morphometry': 'Terrain Analysis - Morphometry',
          'ta_preprocessor': 'Terrain Analysis - Hydrology',
          'ta_profiles': 'Terrain Analysis - Profiles',
          'table_calculus': 'Table tools',
          'table_tools': 'Table tools',
          'tin_tools': 'TIN'}


def decoratedGroupName(name):
    return groups.get(name, name)

algorithms = {'Add Grid Values to Points': 'Add raster values to points',
              'Add Grid Values to Shapes': 'Add raster values to features',
              'Change Grid Values': 'Reclassify values (simple)',
              'Clip Grid with Polygon': 'Clip raster with polygon',
              'Cluster Analysis for Grids': 'Cluster Analysis',
              'Contour Lines from Grid': 'Contour Lines',
              'Cubic Spline Approximation': 'Interpolate (Cubic spline)',
              'Cut Shapes Layer': 'Cut vector Layer',
              'Directional Statistics for Single Grid': 'Directional Statistics for raster layer',
              'Filter Clumps': 'Remove small pixel clumps (to no-data)',
              'Fire Risk Analysis': 'Fire Risk Analysis',
              'Fit N Points to shape': 'Fit n points in polygon',
              'Flat Detection': 'Flat Detection',
              'Flow Accumulation (Flow Tracing)': 'Catchment area (Flow Tracing)',
              'Flow Accumulation (Recursive)': 'Catchment area (Recursive)',
              'Flow Accumulation (Top-Down)': 'Catchment area',
              'GWR for Multiple Predictor Grids': 'GWR for Multiple Predictor layers',
              'GWR for Single Predictor Grid': 'GWR for Single Predictor layer',
              'Geographically Weighted Multiple Regression (Points/Grids)': 'Geographically Weighted Multiple Regression (Points/Raster)',
              'Geographically Weighted Regression (Points/Grid)': 'Geographically Weighted Regression (Points/Raster)',
              'Geometric Figures': 'Geometric Figures',
              'Get Shapes Extents': 'Feature extents',
              "Global Moran's I for Grids": "Global Moran's I for raster layer",
              'Grid Buffer': 'Raster Buffer',
              'Grid Cell Index': 'Raster Cell Index',
              'Grid Difference': 'Raster Difference',
              'Grid Division': 'Raster Division',
              'Grid Masking': 'Raster Masking',
              'Grid Normalisation': 'Raster Normalisation',
              'Grid Orientation': 'Raster Orientation',
              'Grid Proximity Buffer': 'Raster Proximity Buffer',
              'Grid Skeletonization': 'Raster Skeletonization',
              'Grid Standardisation': 'Raster Standardisation',
              'Grid Statistics for Polygons': 'Raster Statistics for Polygons',
              'Grid Values to Points': 'Raster Values to Points',
              'Grid Values to Points (randomly)': 'Raster Values to Points (randomly)',
              'Grid Volume': 'Raster Volume',
              'Grids Product': 'Raster Product',
              'Grids Sum': 'Rasters Sum',
              'Inverse Distance Weighted': 'Inverse Distance Weighted Interpolation',
              'Identity': 'Polygon identity',
              'Merge Layers': 'Merge vector layers',
              'Modified Quadratic Shepard': 'Modified Quadratic Shepard interpolation',
              'Mosaick raster layers': 'Mosaic raster layers',
              'Multilevel B-Spline Interpolation': 'Multilevel B-Spline Interpolation',
              'Multilevel B-Spline Interpolation (from Grid)': 'Multilevel B-Spline Interpolation (from Raster)',
              'Multiple Regression Analysis (Grid/Grids)': 'Multiple Regression Analysis (Raster/Raster)',
              'Multiple Regression Analysis (Points/Grids)': 'Multiple Regression Analysis (Points/Raster)',
              'Proximity Grid': 'Proximity Raster',
              'QuadTree Structure to Shapes': 'QuadTree Structure to polygons',
              'Radius of Variance (Grid)': 'Radius of Variance (Raster)',
              'Reclassify Grid Values': 'Reclassify values',
              'Shapes Buffer (Attribute distance)': 'Variable distance buffer',
              'Shapes Buffer (Fixed distance)': 'fixed distance buffer',
              'Shapes to Grid': 'Rasterize',
              'Statistics for Grids': 'Statistics for Rasters',
              'Terrain Ruggedness Index (TRI)': 'Terrain Ruggedness Index (TRI)',
              'Thin Plate Spline (Global)': 'Thin Plate Spline (Global)',
              'Thin Plate Spline (Local)': 'Thin Plate Spline (Local)',
              'Thin Plate Spline (TIN)': 'Thin Plate Spline (TIN)',
              'Threshold Buffer': 'Threshold raster buffer',
              'Transform Shapes': 'Transform vector layer',
              'Transpose Grids': 'Transpose Raster layers',
              'Union': 'Polygon uUnion',
              'Update': 'Polygon update',
              'Upslope Area': 'Upslope Area',
              'Zonal Grid Statistics': 'Zonal raster statistics'}


def decoratedAlgorithmName(name):
    decorated = algorithms.get(name, name)
    return decorated[0].upper() + decorated[1:].lower()
