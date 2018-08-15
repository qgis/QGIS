# -*- coding: utf-8 -*-
"""
/***************************************************************************
    KNearestConcaveHull.py
    ----------------------
    Date                 : November 2014
    Copyright            : (C) 2014 by Detlev Neumann
                           Dr. Neumann Consulting - Geospatial Services
    Email                : dneumann@geospatial-services.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
"""

__author__ = 'Detlev Neumann'
__date__ = 'November 2014'
__copyright__ = '(C) 2014, Detlev Neumann'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os.path
import math

from qgis.PyQt.QtGui import QIcon
from qgis.PyQt.QtCore import QVariant

from qgis.core import (QgsApplication,
                       QgsExpression,
                       QgsFeature,
                       QgsFeatureRequest,
                       QgsFeatureSink,
                       QgsField,
                       QgsFields,
                       QgsGeometry,
                       QgsProcessing,
                       QgsProcessingException,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterField,
                       QgsProcessingParameterNumber,
                       QgsPoint,
                       QgsPointXY,
                       QgsWkbTypes)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm

class KNearestConcaveHull(QgisAlgorithm):
    KNEIGHBORS = 'KNEIGHBORS'
    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    FIELD = 'FIELD'

    def name(self):
        return 'knearestconcavehull'

    def displayName(self):
        return self.tr('Concave hull (using k-nearest neighbour algorithm)')

    def icon(self):
        return QgsApplication.getThemeIcon("/algorithms/mAlgorithmKNearestConcaveHull.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("/algorithms/mAlgorithmKNearestConcaveHull.svg")

    def group(self):
        return self.tr('Vector geometry')

    def groupId(self):
        return 'vectorgeometry'

    def __init__(self):
        super().__init__()


    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterNumber(self.KNEIGHBORS,
                                                       self.tr('Number of neighbors'),
                                                       QgsProcessingParameterNumber.Integer,
                                                       defaultValue=3, minValue=3))
        self.addParameter(QgsProcessingParameterField(self.FIELD,
                                              self.tr('Field (optional, set if creating concave hulls by class)'),
                                              parentLayerParameterName=self.INPUT, optional=True))
        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Bounding geometry'),
                                                            QgsProcessing.TypeVectorPolygon))

    def processAlgorithm(self, parameters, context, feedback):
        # get variables from dialog
        source = self.parameterAsSource(parameters, self.INPUT, context)
        if source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.INPUT))

        field_name = self.parameterAsString(parameters, self.FIELD, context)
        kneighbors = self.parameterAsInt(parameters, self.KNEIGHBORS, context)
        use_field = bool(field_name)

        field_index = -1

        fields = QgsFields()
        fields.append(QgsField('id', QVariant.Int, '', 20))

        # get properties of the field the grouping is based on
        if use_field:
            field_index = source.fields().lookupField(field_name)
            if field_index >= 0:
                fields.append(source.fields()[field_index])

        # initialize writer
        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               fields, QgsWkbTypes.Polygon, source.sourceCrs())

        current = 0
        fid = 0

        if field_index >= 0:
            # get unique values of field denoted by index as filter conditions
            unique_values = source.uniqueValues(field_index)
            total = 100.0 / float(source.featureCount() * len(unique_values))

            for unique in unique_values:
                points = []
                filter = QgsExpression.createFieldEqualityExpression(field_name,unique)
                request = QgsFeatureRequest().setFilterExpression(filter)
                request.setSubsetOfAttributes([])
                features = source.getFeatures(request)
                for in_feature in features:
                    points.extend(extract_points(in_feature.geometry()))
                    current += 1
                    feedback.setProgress(int(current * total))

                # A minimum of 3 points is necessary to proceed
                if len(points) >= 3:
                    out_feature = QgsFeature()
                    try:
                        the_hull = concave_hull(points, kneighbors)
                        if the_hull:
                            vertex = [QgsPointXY(point[0], point[1]) for point in the_hull]
                            poly = QgsGeometry().fromPolygonXY([vertex])

                            out_feature.setGeometry(poly)
                            out_feature.setAttributes([fid, unique])
                            sink.addFeature(out_feature, QgsFeatureSink.FastInsert)
                    except:
                        feedback.reportError('Exception while computing concave hull.')
                        raise QgsProcessingException('Exception while computing concave hull.')
                fid += 1

        else:
            points = []
            request = QgsFeatureRequest()
            request.setSubsetOfAttributes([])
            features = source.getFeatures(request)
            total = 100.0 / source.featureCount() if source.featureCount() else 0
            for in_feature in features:
                points.extend(extract_points(in_feature.geometry()))
                current += 1
                feedback.setProgress(int(current * total))

            out_feature = QgsFeature()
            try:
                the_hull = concave_hull(points, kneighbors)
                if the_hull:
                    vertex = [QgsPointXY(point[0], point[1]) for point in the_hull]
                    poly = QgsGeometry().fromPolygonXY([vertex])

                    out_feature.setGeometry(poly)
                    out_feature.setAttributes([0])
                    sink.addFeature(out_feature, QgsFeatureSink.FastInsert)
            except:
                feedback.reportError('Exception while computing concave hull.')
                raise QgsProcessingException('Exception while computing concave hull.')

        return {self.OUTPUT: dest_id}


def clean_list(list_of_points):
    """
    Deletes duplicate points in list_of_points
    """
    return list(set(list_of_points))


def length(vector):
    """
    Returns the number of elements in vector
    """
    return len(vector)


def find_min_y_point(list_of_points):
    """
    Returns that point of *list_of_points* having minimal y-coordinate

    :param list_of_points: list of tuples
    :return: tuple (x, y)
    """
    min_y_pt = list_of_points[0]
    for point in list_of_points[1:]:
        if point[1] < min_y_pt[1] or (point[1] == min_y_pt[1] and point[0] < min_y_pt[0]):
            min_y_pt = point
    return min_y_pt


def add_point(vector, element):
    """
    Returns vector with the given element append to the right
    """
    vector.append(element)
    return vector


def remove_point(vector, element):
    """
    Returns a copy of vector without the given element
    """
    vector.pop(vector.index(element))
    return vector


def euclidian_distance(point1, point2):
    """
    Returns the euclidian distance of the 2 given points.

    :param point1: tuple (x, y)
    :param point2: tuple (x, y)
    :return: float
    """
    return math.sqrt(math.pow(point1[0] - point2[0], 2) + math.pow(point1[1] - point2[1], 2))


def nearest_points(list_of_points, point, k):
    """
    Returns a list of the indices of the k closest neighbors from list_of_points to the specified point.
    The measure of proximity is the Euclidean distance. Internally, k becomes the minimum between the given value
    for k and the number of points in list_of_points

    :param list_of_points: list of tuples
    :param point: tuple (x, y)
    :param k: integer
    :return: list of k tuples
    """
    # build a list of tuples of distances between point *point* and every point in *list_of_points*, and
    # their respective index of list *list_of_distances*
    list_of_distances = []
    for index in range(len(list_of_points)):
        list_of_distances.append(( euclidian_distance(list_of_points[index], point), index))

    # sort distances in ascending order
    list_of_distances.sort()

    # get the k nearest neighbors of point
    nearest_list = []
    for index in range(min(k, len(list_of_points))):
        nearest_list.append((list_of_points[list_of_distances[index][1]]))
    return nearest_list


def angle(from_point, to_point):
    """
    Returns the angle of the directed line segment, going from *from_point* to *to_point*, in radians. The angle is
    positive for segments with upward direction (north), otherwise negative (south). Values ranges from 0 at the
    right (east) to pi at the left side (west).

    :param from_point: tuple (x, y)
    :param to_point: tuple (x, y)
    :return: float
    """
    return math.atan2(to_point[1] - from_point[1], to_point[0] - from_point[0])


def angle_difference(angle1, angle2):
    """
    Calculates the difference between the given angles in clockwise direction as radians.

    :param angle1: float
    :param angle2: float
    :return: float; between 0 and 2*Pi
    """
    if (angle1 > 0 and angle2 >= 0) and angle1 > angle2:
        return abs(angle1 - angle2)
    elif (angle1 >= 0 and angle2 > 0) and angle1 < angle2:
        return 2 * math.pi + angle1 - angle2
    elif (angle1 < 0 and angle2 <= 0) and angle1 < angle2:
        return 2 * math.pi + angle1 + abs(angle2)
    elif (angle1 <= 0 and angle2 < 0) and angle1 > angle2:
        return abs(angle1 - angle2)
    elif angle1 <= 0 < angle2:
        return 2 * math.pi + angle1 - angle2
    elif angle1 >= 0 >= angle2:
        return angle1 + abs(angle2)
    else:
        return 0


def intersect(line1, line2):
    """
    Returns True if the two given line segments intersect each other, and False otherwise.

    :param line1: 2-tuple of tuple (x, y)
    :param line2: 2-tuple of tuple (x, y)
    :return: boolean
    """
    a1 = line1[1][1] - line1[0][1]
    b1 = line1[0][0] - line1[1][0]
    c1 = a1 * line1[0][0] + b1 * line1[0][1]
    a2 = line2[1][1] - line2[0][1]
    b2 = line2[0][0] - line2[1][0]
    c2 = a2 * line2[0][0] + b2 * line2[0][1]
    tmp = (a1 * b2 - a2 * b1)
    if tmp == 0:
        return False
    sx = (c1 * b2 - c2 * b1) / tmp
    if (sx > line1[0][0] and sx > line1[1][0]) or (sx > line2[0][0] and sx > line2[1][0]) or\
            (sx < line1[0][0] and sx < line1[1][0]) or (sx < line2[0][0] and sx < line2[1][0]):
        return False
    sy = (a1 * c2 - a2 * c1) / tmp
    if (sy > line1[0][1] and sy > line1[1][1]) or (sy > line2[0][1] and sy > line2[1][1]) or\
            (sy < line1[0][1] and sy < line1[1][1]) or (sy < line2[0][1] and sy < line2[1][1]):
        return False
    return True


def point_in_polygon_q(point, list_of_points):
    """
    Return True if given point *point* is laying in the polygon described by the vertices *list_of_points*,
    otherwise False

    Based on the "Ray Casting Method" described by Joel Lawhead in this blog article:
    http://geospatialpython.com/2011/01/point-in-polygon.html

    """
    x = point[0]
    y = point[1]
    poly = [(pt[0], pt[1]) for pt in list_of_points]
    n = len(poly)
    inside = False

    p1x, p1y = poly[0]
    for i in range(n + 1):
        p2x, p2y = poly[i % n]
        if y > min(p1y, p2y):
            if y <= max(p1y, p2y):
                if x <= max(p1x, p2x):
                    if p1y != p2y:
                        xints = (y - p1y) * (p2x - p1x) / (p2y - p1y) + p1x
                    if p1x == p2x or x <= xints:
                        inside = not inside
        p1x, p1y = p2x, p2y

    return inside


def as_polygon(point_list):
    """
    Returns the geometry described by *point_list* in as QgsGeometry

    :param point_list: list of tuples (x, y)
    :return: QgsGeometry
    """
    # create a list of QgsPoint() from list of point coordinate strings in *point_list*
    points = [QgsPoint(point[0], point[1]) for point in point_list]
    # create the polygon geometry from list of point geometries
    poly = QgsGeometry.fromPolygon([points])
    return poly


def extract_points(geom):
    """
    Generate list of QgsPoints from QgsGeometry *geom* ( can be point, line, or polygon )
    Code taken from fTools plugin

    :param geom: an arbitrary geometry feature
    :return: list of points
    """
    multi_geom = QgsGeometry()
    temp_geom = []
    # point geometry
    if geom.type() == 0:
        if geom.isMultipart():
            temp_geom = geom.asMultiPoint()
        else:
            temp_geom.append(geom.asPoint())
    # line geometry
    if geom.type() == 1:
        # if multipart feature explode to single part
        if geom.isMultipart():
            multi_geom = geom.asMultiPolyline()
            for i in multi_geom:
                temp_geom.extend( i )
        else:
            temp_geom = geom.asPolyline()
    # polygon geometry
    elif geom.type() == 2:
        # if multipart feature explode to single part
        if geom.isMultipart():
            multi_geom = geom.asMultiPolygon()
            # now single part polygons
            for i in multi_geom:
                # explode to line segments
                for j in i:
                    temp_geom.extend( j )
        else:
            multi_geom = geom.asPolygon()
            # explode to line segments
            for i in multi_geom:
                temp_geom.extend( i )
    return temp_geom


def sort_by_angle(list_of_points, last_point, last_angle):
    """
    returns the points in list_of_points in descending order of angle to the last segment of the envelope,
    measured in a clockwise direction. Thus, the rightmost of the neighboring points is always selected. The first
    point of this list will be the next point of the envelope.
    """
    def getkey(item):
        return angle_difference(last_angle, angle(last_point, item))

    vertex_list = sorted(list_of_points, key=getkey, reverse=True)
    return vertex_list


def concave_hull(points_list, k):
    """
    Calculates a valid concave hull polygon containing all given points. The algorithm searches for that
    point in the neighborhood of k nearest neighbors which maximizes the rotation angle in clockwise direction
    without intersecting any previous line segments.

    This is an implementation of the algorithm described by Adriano Moreira and Maribel Yasmina Santos:
    CONCAVE HULL: A neighborhood_k-NEAREST NEIGHBOURS APPROACH FOR THE COMPUTATION OF THE REGION OCCUPIED BY A SET OF POINTS.
    GRAPP 2007 - International Conference on Computer Graphics Theory and Applications; pp 61-68.

    :param points_list: list of tuples (x, y)
    :param k: integer
    :return: list of tuples (x, y)
    """
    # return an empty list if not enough points are given
    if k > len(points_list):
        return None

    # the number of nearest neighbors k must be greater than or equal to 3
    # kk = max(k, 3)
    kk = max(k, 2)

    # delete duplicate points
    point_set = clean_list(points_list)

    # if point_set has less then 3 points no polygon can be created and an empty list will be returned
    if len(point_set) < 3:
        return None

    # if point_set has 3 points then these are already vertices of the hull. Append the first point to
    # close the hull polygon
    if len(point_set) == 3:
        return add_point(point_set, point_set[0])

    # make sure that k neighbours can be found
    kk = min(kk, len(point_set))

    # start with the point having the smallest y-coordinate (most southern point)
    first_point = find_min_y_point(point_set)

    # add this points as the first vertex of the hull
    hull = [first_point]

    # make the first vertex of the hull to the current point
    current_point = first_point

    # remove the point from the point_set, to prevent him being among the nearest points
    point_set = remove_point(point_set, first_point)
    previous_angle = math.pi

    # step counts the number of segments
    step = 2

    # as long as point_set is not empty or search is returning to the starting point
    while (current_point != first_point) or (step == 2) and (len(point_set) > 0):

        # after 3 iterations add the first point to point_set again, otherwise a hull cannot be closed
        if step == 5:
            point_set = add_point(point_set, first_point)

        # search the k nearest neighbors of the current point
        k_nearest_points = nearest_points(point_set, current_point, kk)

        # sort the candidates (neighbors) in descending order of right-hand turn. This way the algorithm progresses
        # in clockwise direction through as many points as possible
        c_points = sort_by_angle(k_nearest_points, current_point, previous_angle)

        its = True
        i = -1

        # search for the nearest point to which the connecting line does not intersect any existing segment
        while its is True and (i < len(c_points)-1):
            i += 1
            if c_points[i] == first_point:
                last_point = 1
            else:
                last_point = 0
            j = 2
            its = False

            while its is False and (j < len(hull) - last_point):
                its = intersect((hull[step-2], c_points[i]), (hull[step-2-j], hull[step-1-j]))
                j += 1

        # there is no candidate to which the connecting line does not intersect any existing segment, so the
        # for the next candidate fails. The algorithm starts again with an increased number of neighbors
        if its is True:
            return concave_hull(points_list, kk + 1)

        # the first point which complies with the requirements is added to the hull and gets the current point
        current_point = c_points[i]
        hull = add_point(hull, current_point)

        # calculate the angle between the last vertex and his precursor, that is the last segment of the hull
        # in reversed direction
        previous_angle = angle(hull[step - 1], hull[step - 2])

        # remove current_point from point_set
        point_set = remove_point(point_set, current_point)

        # increment counter
        step += 1

    all_inside = True
    i = len(point_set)-1

    # check if all points are within the created polygon
    while (all_inside is True) and (i >= 0):
        all_inside = point_in_polygon_q(point_set[i], hull)
        i -= 1

    # since at least one point is out of the computed polygon, try again with a higher number of neighbors
    if all_inside is False:
        return concave_hull(points_list, kk + 1)

    # a valid hull has been constructed
    return hull
