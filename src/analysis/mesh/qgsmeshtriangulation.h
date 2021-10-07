/***************************************************************************
                          qgsmeshtriangulation.h
                          -----------------
    begin                : August 9th, 2020
    copyright            : (C) 2020 by Vincent Cloarec
    email                : vcloarec at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSMESHTRIANGULATION_H
#define QGSMESHTRIANGULATION_H

#include "qgscoordinatereferencesystem.h"
#include "qgsmeshdataprovider.h"
#include "qgsmeshadvancedediting.h"

#include "qgis_analysis.h"


class QgsVectorLayer;
class QgsCoordinateTransformContext;
class QgsFeature;
class QgsFeatureIterator;
class QgsTriangulation;
class QgsFeedback;

/**
 * \ingroup analysis
 * \class QgsMeshTriangulation
 *
 * \brief Class that handles mesh creation with Delaunay constrained triangulation
 *
 * \since QGIS 3.16
 */
class ANALYSIS_EXPORT QgsMeshTriangulation : public QObject
{
    Q_OBJECT
  public:

    //! Constructor
    QgsMeshTriangulation();

    //! Destructor
    ~QgsMeshTriangulation();

    /**
     * Adds vertices to the triangulation from a feature iterator, return TRUE if successful.
     *
     * \param vertexFeatureIterator the feature iterator of vertices to insert
     * \param valueAttribute the index of the attribute that represents the value of vertices, if -1 uses Z coordinate of vertices
     * \param transform the coordinates transform used to transform coordinates
     * \param feedback feedback argument may be specified to allow cancellation and progress reports
     * \param featureCount the count of feature to allow progress report of the feedback
     */
    bool addVertices( QgsFeatureIterator &vertexFeatureIterator, int valueAttribute, const QgsCoordinateTransform &transform, QgsFeedback *feedback = nullptr, long featureCount = 1 );

    /**
     * Adds break lines from a vector layer, return TRUE if successful.
     * \param lineFeatureIterator the feature iterator of break lines to insert
     * \param valueAttribute the index of the attribute that represents the value of vertices, if -1 uses Z coordinate of vertices
     * \param transformContext the coordinates transform context used to transform coordinates
     * \param feedback feedback argument may be specified to allow cancellation and progress reports
     * \param featureCount the count of feature to allow progress report of the feedback
     *
     * \warning if the feature iterator contains only point geometries, the vertices will be added only without treating them as breaklines
     */
    bool addBreakLines( QgsFeatureIterator &lineFeatureIterator, int valueAttribute, const QgsCoordinateTransform &transformContext, QgsFeedback *feedback = nullptr, long featureCount = 1 );

    /**
     * Adds a new vertex in the triangulation and returns the index of the new vertex
     *
     * \since QGIS 3.22
     */
    int addVertex( const QgsPoint &vertex );

    //! Returns the triangulated mesh
    QgsMesh triangulatedMesh( QgsFeedback *feedback = nullptr ) const;

    //! Sets the coordinate reference system used for the triangulation
    void setCrs( const QgsCoordinateReferenceSystem &crs );

  private:
#ifdef SIP_RUN
    QgsMeshTriangulation( const QgsMeshTriangulation &rhs );
#endif

    QgsCoordinateReferenceSystem mCrs;
    std::unique_ptr<QgsTriangulation> mTriangulation;

    void addVerticesFromFeature( const QgsFeature &feature, int valueAttribute, const QgsCoordinateTransform &transform, QgsFeedback *feedback = nullptr );
    void addBreakLinesFromFeature( const QgsFeature &feature, int valueAttribute, const QgsCoordinateTransform &transform, QgsFeedback *feedback = nullptr );
};

#ifndef SIP_RUN

/**
 * \ingroup analysis
 * \class QgsMeshZValueDataset
 *
 * \brief Convenient class that can be used to obtain a dataset that represents the Z values of mesh vertices
 *
 * \since QGIS 3.16
 */
class QgsMeshZValueDataset: public QgsMeshDataset
{
  public:

    //! Constructor with the mesh
    QgsMeshZValueDataset( const QgsMesh &mesh );

    QgsMeshDatasetValue datasetValue( int valueIndex ) const override;
    QgsMeshDataBlock datasetValues( bool isScalar, int valueIndex, int count ) const override;
    QgsMeshDataBlock areFacesActive( int faceIndex, int count ) const override;
    bool isActive( int faceIndex ) const override;
    QgsMeshDatasetMetadata metadata() const override;
    int valuesCount() const override;

  private:
    QgsMesh mMesh;
    double mZMinimum = std::numeric_limits<double>::max();
    double mZMaximum = -std::numeric_limits<double>::max();
};

#endif //SIP_RUN

/**
 * \ingroup analysis
 * \class QgsMeshZValueDatasetGroup
 *
 * \brief Convenient class that can be used to obtain a datasetgroup on vertices that represents the Z value of the mesh vertices
 *
 * \since QGIS 3.16
 */
class ANALYSIS_EXPORT QgsMeshZValueDatasetGroup: public QgsMeshDatasetGroup
{
  public:

    /**
     * Constructor
     *
     * \param datasetGroupName the name of the dataset group
     * \param mesh the mesh used to create the Z value dataset
    */
    QgsMeshZValueDatasetGroup( const QString &datasetGroupName, const QgsMesh &mesh );

    void initialize() override;
    QgsMeshDatasetMetadata datasetMetadata( int datasetIndex ) const override;
    int datasetCount() const override;
    QgsMeshDataset *dataset( int index ) const override;
    QgsMeshDatasetGroup::Type type() const override {return QgsMeshDatasetGroup::Virtual;}
    QDomElement writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const override;

  private:
#ifdef SIP_RUN
    QgsMeshZValueDatasetGroup( const QgsMeshZValueDatasetGroup &rhs );
#endif
    std::unique_ptr<QgsMeshZValueDataset> mDataset;
};


/**
 * \ingroup analysis
 * \class QgsMeshEditingDelaunayTriangulation
 *
 * \brief Class that can be used with QgsMeshEditor::advancedEdit() to add triangle faces to a mesh created by
 * a Delaunay triangulation on provided existing vertex.
 *
 * \since QGIS 3.22
 */
class ANALYSIS_EXPORT QgsMeshEditingDelaunayTriangulation : public QgsMeshAdvancedEditing
{
  public:

    //! Constructor
    QgsMeshEditingDelaunayTriangulation();

    QString text() const override;

  private:
    QgsTopologicalMesh::Changes apply( QgsMeshEditor *meshEditor ) override;
};

#endif // QGSMESHTRIANGULATION_H
