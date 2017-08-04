/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsvectorlayereditutils.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsVectorLayerEditUtils
{

%TypeHeaderCode
#include "qgsvectorlayereditutils.h"
%End
  public:
    QgsVectorLayerEditUtils( QgsVectorLayer *layer );

    bool insertVertex( double x, double y, QgsFeatureId atFeatureId, int beforeVertex );
%Docstring
 Insert a new vertex before the given vertex number,
  in the given ring, item (first number is index 0), and feature
  Not meaningful for Point geometries
 :rtype: bool
%End

    bool insertVertex( const QgsPoint &point, QgsFeatureId atFeatureId, int beforeVertex );
%Docstring
 Insert a new vertex before the given vertex number,
  in the given ring, item (first number is index 0), and feature
  Not meaningful for Point geometries
 :rtype: bool
%End

    bool moveVertex( double x, double y, QgsFeatureId atFeatureId, int atVertex );
%Docstring
 Moves the vertex at the given position number,
  ring and item (first number is index 0), and feature
  to the given coordinates
 :rtype: bool
%End

    bool moveVertex( const QgsPoint &p, QgsFeatureId atFeatureId, int atVertex ) /PyName=moveVertexV2/;
%Docstring
 Moves the vertex at the given position number,
  ring and item (first number is index 0), and feature
  to the given coordinates
.. note::

   available in Python bindings as moveVertexV2
 :rtype: bool
%End

    QgsVectorLayer::EditResult deleteVertex( QgsFeatureId featureId, int vertex );
%Docstring
 Deletes a vertex from a feature.
 \param featureId ID of feature to remove vertex from
 \param vertex index of vertex to delete
.. versionadded:: 2.14
 :rtype: QgsVectorLayer.EditResult
%End

    int addRing( const QList<QgsPointXY> &ring, const QgsFeatureIds &targetFeatureIds = QgsFeatureIds(), QgsFeatureId *modifiedFeatureId = 0 );
%Docstring
 :rtype: int
%End

    int addRing( QgsCurve *ring, const QgsFeatureIds &targetFeatureIds = QgsFeatureIds(), QgsFeatureId *modifiedFeatureId = 0 ) /PyName=addCurvedRing/;
%Docstring
 :rtype: int
%End

    int addPart( const QList<QgsPointXY> &ring, QgsFeatureId featureId );
%Docstring
 :rtype: int
%End

    int addPart( const QgsPointSequence &ring, QgsFeatureId featureId );
%Docstring
 :rtype: int
%End

    int addPart( QgsCurve *ring, QgsFeatureId featureId ) /PyName=addCurvedPart/;
%Docstring
 :rtype: int
%End

    int translateFeature( QgsFeatureId featureId, double dx, double dy );
%Docstring
 Translates feature by dx, dy
 \param featureId id of the feature to translate
 \param dx translation of x-coordinate
 \param dy translation of y-coordinate
 :return: 0 in case of success
 :rtype: int
%End

    int splitParts( const QList<QgsPointXY> &splitLine, bool topologicalEditing = false );
%Docstring
 :rtype: int
%End

    int splitFeatures( const QList<QgsPointXY> &splitLine, bool topologicalEditing = false );
%Docstring
 :rtype: int
%End

    int addTopologicalPoints( const QgsGeometry &geom );
%Docstring
 Adds topological points for every vertex of the geometry.
 \param geom the geometry where each vertex is added to segments of other features
.. note::

   geom is not going to be modified by the function
 :return: 0 in case of success
 :rtype: int
%End

    int addTopologicalPoints( const QgsPointXY &p );
%Docstring
 Adds a vertex to segments which intersect point p but don't
 already have a vertex there. If a feature already has a vertex at position p,
 no additional vertex is inserted. This method is useful for topological
 editing.
 \param p position of the vertex
 :return: 0 in case of success
 :rtype: int
%End

  protected:

    int boundingBoxFromPointList( const QList<QgsPointXY> &list, double &xmin, double &ymin, double &xmax, double &ymax ) const;
%Docstring
 Little helper function that gives bounding box from a list of points.
:return: 0 in case of success *
 :rtype: int
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsvectorlayereditutils.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
