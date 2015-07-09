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


class SagaGroupNameDecorator:

    groups = {}
    groups['contrib_a_perego'] = 'Contributions'
    groups['statistics_grid'] = 'Geostatistics'
    groups['statistics_kriging'] = 'Kriging'
    groups['statistics_points'] = 'Geostatistics'
    groups['statistics_regression'] = 'Geostatistics'
    groups['grid_analysis'] = 'Grid - Analysis'
    groups['grid_calculus'] = 'Grid - Calculus'
    groups['grid_calculus_bsl'] = 'Grid - Calculus'
    groups['grid_discretisation'] = 'Grid - Discretisation'
    groups['grid_filter'] = 'Grid - Filter'
    groups['grid_gridding'] = 'Grid - Gridding'
    groups['grid_spline'] = 'Grid - Spline'
    groups['grid_tools'] = 'Grid - Tools'
    groups['grid_visualisation'] = 'Grid - Visualization'
    groups['hacres'] = 'Hacres'
    groups['imagery_segmentation'] = 'Imagery - Segmentation'
    groups['imagery_classification'] = 'Imagery - Classification'
    groups['imagery_rga'] = 'Imagery - RGA'
    groups['imagery_tools'] = 'Imagery - Tools'
    groups['io_esri_e00'] = 'I/O'
    groups['io_gdal'] = 'I/O'
    groups['io_gps'] = 'I/O'
    groups['io_grid'] = 'I/O'
    groups['io_grid_grib2'] = 'I/O'
    groups['io_grid_image'] = 'I/O'
    groups['io_odbc'] = 'I/O'
    groups['io_shapes'] = 'I/O'
    groups['io_shapes_dxf'] = 'I/O'
    groups['io_shapes_las'] = 'I/O'
    groups['io_table'] = 'I/O'
    groups['lectures_introduction'] = 'Lectures'
    groups['pj_georeference'] = 'Georeferencing'
    groups['pj_geotrans'] = 'Projections and Transformations'
    groups['pj_proj4'] = 'Projections and Transformations'
    groups['pointcloud_tools'] = 'Point clouds'
    groups['recreations_fractals'] = 'Recreations'
    groups['recreations_games'] = 'Diversiones'
    groups['shapes_grid'] = 'Shapes - Grid'
    groups['shapes_lines'] = 'Shapes - Lines'
    groups['shapes_points'] = 'Shapes - Points'
    groups['shapes_polygons'] = 'Shapes - Polygons'
    groups['shapes_tools'] = 'Shapes - Tools'
    groups['shapes_transect'] = 'Shapes - Transect'
    groups['sim_cellular_automata'] = 'Simulation - CA'
    groups['sim_ecosystems_hugget'] = 'Simulation - Ecosystems'
    groups['sim_fire_spreading'] = 'Simulation - Fire Spreading'
    groups['sim_hydrology'] = 'Simulation - Hydrology'
    groups['table_calculus'] = 'Table - Calculus'
    groups['table_tools'] = 'Table - Tools'
    groups['ta_channels'] = 'Terrain Analysis - Channels'
    groups['ta_compound'] = 'Terrain Analysis - Morphometry'
    groups['ta_hydrology'] = 'Terrain Analysis - Hydrology'
    groups['ta_lighting'] = 'Terrain Analysis - Lighting'
    groups['ta_morphometry'] = 'Terrain Analysis - Morphometry'
    groups['ta_preprocessor'] = 'Terrain Analysis - Hydrology'
    groups['ta_profiles'] = 'Terrain Analysis - Profiles'
    groups['tin_tools'] = 'TIN'
    groups['vigra'] = 'Vigra'

    @staticmethod
    def getDecoratedName(groupName):
        return SagaGroupNameDecorator.groups.get(groupName, groupName)
