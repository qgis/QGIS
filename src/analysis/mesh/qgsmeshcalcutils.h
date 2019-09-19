/***************************************************************************
                          qgsmeshcalcutils.h
                          ------------------
    begin                : December 18th, 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMESHCALCUTILS_H
#define QGSMESHCALCUTILS_H

#define SIP_NO_FILE

///@cond PRIVATE

#include <QStringList>
#include <QMap>
#include <QVector>

#include <algorithm>
#include <functional>
#include <math.h>
#include <numeric>

#include "qgsrectangle.h"
#include "qgsmeshlayer.h"
#include "qgsmeshdataprovider.h"
#include "qgis_analysis.h"

struct QgsMeshMemoryDatasetGroup;
struct QgsMeshMemoryDataset;

/**
 * \ingroup analysis
 * \class QgsMeshCalcUtils
 * Mathematical operations on QgsMeshMemoryDatasetGroup
 * QgsMeshMemoryDatasetGroups must be compatible (same mesh structure, same number of datasets, ...)
 * Any operation with NODATA is NODATA (e.g. NODATA + 1 = NODATA)
 *
 * \since QGIS 3.6
 */
class ANALYSIS_EXPORT QgsMeshCalcUtils
{
  public:

    /**
     * Creates the utils and validates the input
     * \param layer mesh layer
     * \param usedGroupNames dataset group's names that are used in the expression
     * \param startTime start time
     * \param endTime end time
     */
    QgsMeshCalcUtils( QgsMeshLayer *layer,
                      const QStringList &usedGroupNames,
                      double startTime,
                      double endTime );

    //! Returns whether the input parameters are consistent and valid for given mesh layer
    bool isValid() const;

    //! Returns associated mesh layer
    const QgsMeshLayer *layer() const;

    //! Returns dataset group based on name
    std::shared_ptr<const QgsMeshMemoryDatasetGroup> group( const QString &groupName ) const;

    //! Creates a single dataset with all values set to 1
    void ones( QgsMeshMemoryDatasetGroup &group1 ) const;

    //! Creates a single dataset with all values set to NODATA
    void nodata( QgsMeshMemoryDatasetGroup &group1 ) const;

    //! Returns a single dataset with all values set to val
    std::shared_ptr<QgsMeshMemoryDataset> number( double val, double time ) const;

    //! Creates a deepcopy of group with groupName to group1. Does not copy datasets for filtered out times.
    void copy( QgsMeshMemoryDatasetGroup &group1, const QString &groupName ) const;

    //! Creates a deepcopy of dataset0
    std::shared_ptr<QgsMeshMemoryDataset> copy( std::shared_ptr<const QgsMeshMemoryDataset> dataset0 ) const;

    //! Changes ownership of all datasets from group2 to group1
    void transferDatasets( QgsMeshMemoryDatasetGroup &group1, QgsMeshMemoryDatasetGroup &group2 ) const;

    //! If group2 has more datasets than group1, duplicates datasets in group1 so it has the same number of datasets as group2
    void expand( QgsMeshMemoryDatasetGroup &group1,
                 const QgsMeshMemoryDatasetGroup &group2 ) const;

    //! Creates a single dataset with custom number
    void number( QgsMeshMemoryDatasetGroup &group1, double val ) const;

    //! If condition is TRUE (for given time&point), takes val from trueGroup else from falseGroup
    void addIf( QgsMeshMemoryDatasetGroup &trueGroup,
                const QgsMeshMemoryDatasetGroup &falseGroup,
                const QgsMeshMemoryDatasetGroup &condition ) const;

    //! Creates a spatial filter from extent
    void filter( QgsMeshMemoryDatasetGroup &group1, const QgsRectangle &extent ) const;

    //! Creates a spatial filter from geometry
    void filter( QgsMeshMemoryDatasetGroup &group1, const QgsGeometry &mask ) const;

    //! Operator NOT
    void logicalNot( QgsMeshMemoryDatasetGroup &group1 ) const;

    //! Operator SIGN
    void changeSign( QgsMeshMemoryDatasetGroup &group1 ) const;

    //! Operator ABS
    void abs( QgsMeshMemoryDatasetGroup &group1 ) const;

    //! Operator aggregated sum (over all datasets)
    void sumAggregated( QgsMeshMemoryDatasetGroup &group1 ) const;

    //! Operator aggregated min (over all datasets)
    void minimumAggregated( QgsMeshMemoryDatasetGroup &group1 ) const;

    //! Operator aggregated max (over all datasets)
    void maximumAggregated( QgsMeshMemoryDatasetGroup &group1 ) const;

    //! Operator aggregated average (over all datasets)
    void averageAggregated( QgsMeshMemoryDatasetGroup &group1 ) const;

    //! Operator +
    void add( QgsMeshMemoryDatasetGroup &group1, const QgsMeshMemoryDatasetGroup &group2 ) const;

    //! Operator -
    void subtract( QgsMeshMemoryDatasetGroup &group1, const QgsMeshMemoryDatasetGroup &group2 ) const;

    //! Operator *
    void multiply( QgsMeshMemoryDatasetGroup &group1, const QgsMeshMemoryDatasetGroup &group2 ) const;

    //! Operator /
    void divide( QgsMeshMemoryDatasetGroup &group1, const QgsMeshMemoryDatasetGroup &group2 ) const;

    //! Operator ^
    void power( QgsMeshMemoryDatasetGroup &group1, const QgsMeshMemoryDatasetGroup &group2 ) const;

    //! Operator ==
    void equal( QgsMeshMemoryDatasetGroup &group1, const QgsMeshMemoryDatasetGroup &group2 ) const;

    //! Operator !=
    void notEqual( QgsMeshMemoryDatasetGroup &group1, const QgsMeshMemoryDatasetGroup &group2 ) const;

    //! Operator >
    void greaterThan( QgsMeshMemoryDatasetGroup &group1, const QgsMeshMemoryDatasetGroup &group2 ) const;

    //! Operator <
    void lesserThan( QgsMeshMemoryDatasetGroup &group1, const QgsMeshMemoryDatasetGroup &group2 ) const;

    //! Operator <=
    void lesserEqual( QgsMeshMemoryDatasetGroup &group1, const QgsMeshMemoryDatasetGroup &group2 ) const;

    //! Operator >=
    void greaterEqual( QgsMeshMemoryDatasetGroup &group1, const QgsMeshMemoryDatasetGroup &group2 ) const;

    //! Operator AND
    void logicalAnd( QgsMeshMemoryDatasetGroup &group1, const QgsMeshMemoryDatasetGroup &group2 ) const;

    //! Operator OR
    void logicalOr( QgsMeshMemoryDatasetGroup &group1, const QgsMeshMemoryDatasetGroup &group2 ) const;

    //! Operator minimum
    void minimum( QgsMeshMemoryDatasetGroup &group1, const QgsMeshMemoryDatasetGroup &group2 ) const;

    //! Operator maximum
    void maximum( QgsMeshMemoryDatasetGroup &group1, const QgsMeshMemoryDatasetGroup &group2 ) const;

  private:
    double ffilter( double val1, double filter ) const;
    double fadd( double val1, double val2 ) const;
    double fsubtract( double val1, double val2 ) const;
    double fmultiply( double val1, double val2 ) const;
    double fdivide( double val1, double val2 ) const;
    double fpower( double val1, double val2 ) const;
    double fequal( double val1, double val2 ) const;
    double fnotEqual( double val1, double val2 ) const;
    double fgreaterThan( double val1, double val2 ) const;
    double flesserThan( double val1, double val2 ) const;
    double flesserEqual( double val1, double val2 ) const;
    double fgreaterEqual( double val1, double val2 ) const;
    double flogicalAnd( double val1, double val2 ) const;
    double flogicalOr( double val1, double val2 ) const;
    double flogicalNot( double val1 ) const;
    double fchangeSign( double val1 ) const;
    double fmin( double val1, double val2 ) const;
    double fmax( double val1, double val2 ) const;
    double fabs( double val1 ) const;
    double fsumAggregated( QVector<double> &vals ) const;
    double fminimumAggregated( QVector<double> &vals ) const;
    double fmaximumAggregated( QVector<double> &vals ) const;
    double faverageAggregated( QVector<double> &vals ) const;

    /**
     * Find dataset group in provider with the name and copy all values to
     * memory dataset group. Returns NULLPTR if no such dataset group
     * exists
     */
    std::shared_ptr<QgsMeshMemoryDatasetGroup> create( const QString &datasetGroupName ) const;

    /**
     *  Creates dataset based on group. Initializes values and active based on group type.
     */
    std::shared_ptr<QgsMeshMemoryDataset> create( const QgsMeshMemoryDatasetGroup &grp ) const;

    /**
     *  Creates dataset with given type. Initializes values and active based on type.
     */
    std::shared_ptr<QgsMeshMemoryDataset> create( const QgsMeshDatasetGroupMetadata::DataType type ) const;

    /**
     * Returns dataset based on (time) index. If group has only 1 dataset, returns first one
     */
    std::shared_ptr<QgsMeshMemoryDataset> canditateDataset( QgsMeshMemoryDatasetGroup &group,
        int datasetIndex ) const;

    /**
     * Returns dataset based on on (time) index. If group has only 1 dataset, returns first one
     */
    std::shared_ptr<const QgsMeshMemoryDataset> constCandidateDataset( const QgsMeshMemoryDatasetGroup &group,
        int datasetIndex ) const;

    /**
     * Returns maximum number of datasets in the groups
     */
    int datasetCount( const QgsMeshMemoryDatasetGroup &group1, const QgsMeshMemoryDatasetGroup &group2 ) const;

    /**
     * Set active property for vertices in dataset based on:
     * if given vertex is active in dataset and refDataset
     * if all values in vertices that are referenced by the dataset are not NODATA
     */
    void activate( std::shared_ptr<QgsMeshMemoryDataset> dataset,
                   std::shared_ptr<const QgsMeshMemoryDataset> refDataset = nullptr ) const;

    //! Activates all datasets in group
    void activate( QgsMeshMemoryDatasetGroup &group ) const;

    //! Creates spatial filter group from rectagle
    void populateSpatialFilter( QgsMeshMemoryDatasetGroup &filter, const QgsRectangle &extent ) const; // create a filter from extent

    //! Creates mask filter group from geometry
    void populateMaskFilter( QgsMeshMemoryDatasetGroup &filter, const QgsGeometry &mask ) const; // create a filter from mask

    //! Calculates unary operators
    void func1( QgsMeshMemoryDatasetGroup &group,
                std::function<double( double )> func ) const;

    //! Calculates binary operators
    void func2( QgsMeshMemoryDatasetGroup &group1,
                const QgsMeshMemoryDatasetGroup &group2,
                std::function<double( double, double )> func ) const;

    //! Calculates unary aggregate operator (e.g. sum of values of one vertex for all times)
    void funcAggr( QgsMeshMemoryDatasetGroup &group1,
                   std::function<double( QVector<double>& )> func ) const;

    const QgsTriangularMesh *triangularMesh() const;
    const QgsMesh *nativeMesh() const;
    void updateMesh() const;

    QgsMeshLayer *mMeshLayer; //!< Reference mesh
    bool mIsValid; //!< All used datasets (in datasetMap) do have outputs for same times & all used dataset names are present in mesh
    QgsMeshDatasetGroupMetadata::DataType mOutputType; //!< Mesh can work only with one output types, so you cannot mix
    //!< E.g. one dataset with element outputs and one with node outputs
    QVector<double> mTimes;
    QMap < QString, std::shared_ptr<QgsMeshMemoryDatasetGroup> > mDatasetGroupMap; //!< Groups that are referenced in the expression
};

///@endcond

#endif // QGSMESHCALCUTILS_H
