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
#include "qgis_core.h"

/**
 * \ingroup core
 * \class QgsMeshCalcUtils
 * \brief Mathematical operations on QgsMeshMemoryDatasetGroup
 *
 * QgsMeshMemoryDatasetGroups must be compatible (same mesh structure, same number of datasets, ...)
 * Any operation with NODATA is NODATA (e.g. NODATA + 1 = NODATA)
 *
 * \since QGIS 3.6
 */
class CORE_EXPORT QgsMeshCalcUtils
{
  public:

    /**
     * Creates the utils and validates the input
     *
     * The constructor fetches all dataset values and creates memory datasets from them.
     *
     * \param layer mesh layer
     * \param usedGroupNames dataset group's names that are used in the expression
     * \param startTime start time
     * \param endTime end time
     */
    QgsMeshCalcUtils( QgsMeshLayer *layer,
                      const QStringList &usedGroupNames,
                      double startTime,
                      double endTime );

    /**
     * Creates the utils and validates the input for a calculation only for a dataset at time \a relativeTime
     *
     * The constructor fetches dataset values from selected dataset corresponding to the relative time \a relativeTime
     * (see QgsMeshLayer::datasetIndexAtRelativeTime()) and creates memory datasets from them.
     * There are only one dataset per group, selected with the matching method defined for the layer.
     *
     * \param layer mesh layer
     * \param usedGroupNames dataset group's names that are used in the expression
     * \param relativeTime time of the calculation
     *
     * \note this instance do not support aggregate functions
     *
     * \deprecated QGIS 3.22 because the constructor does not specify any time interval to calculate aggregate functions
     */
    Q_DECL_DEPRECATED QgsMeshCalcUtils( QgsMeshLayer *layer,
                                        const QStringList &usedGroupNames,
                                        const QgsInterval &relativeTime );

    /**
     * Creates the utils and validates the input for a calculation only for a dataset at time \a relativeTime
     *
     * The constructor fetches dataset values from selected dataset corresponding to the relative time \a relativeTime
     * (see QgsMeshLayer::datasetIndexAtRelativeTime()) and creates memory datasets from them.
     * There are only one dataset per group, selected with the matching method defined for the layer,
     * excepted when an aggregate function is involved upstream of a node. In this case,
     * all the dataset that are between start time and end time are selected.
     *
     * \param layer mesh layer
     * \param usedGroupNames dataset group's names that are used in the expression and not involved in a aggregate function
     * \param usedGroupNamesForAggregate dataset group's names that are used in the expression and involved in a aggregate function
     * \param relativeTime time of the calculation
     * \param startTime retlative start time for aggregate functions
     * \param endTime relative end time for aggregate functions
     *
     * \since QGIS 3.22
     */
    QgsMeshCalcUtils( QgsMeshLayer *layer,
                      const QStringList &usedGroupNames,
                      const QStringList &usedGroupNamesForAggregate,
                      const QgsInterval &relativeTime,
                      const QgsInterval &startTime,
                      const QgsInterval &endTime );


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
    void copy( QgsMeshMemoryDatasetGroup &group1, const QString &groupName, bool forAggregate = false ) const;

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

    //! Calculates the data type of result dataset group
    static QgsMeshDatasetGroupMetadata::DataType determineResultDataType( QgsMeshLayer *layer, const QStringList &usedGroupNames );

    //! Returns the data type of result dataset group
    QgsMeshDatasetGroupMetadata::DataType outputType() const;

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
     * Clones the dataset data to memory
     *
     * Finds dataset group in provider with the name and copies all values to
     * memory dataset group. Returns NULLPTR if no such dataset group
     * exists. Resulting datasets are guaranteed to have the same mOutputType type
     */
    std::shared_ptr<QgsMeshMemoryDatasetGroup> createMemoryDatasetGroup( const QString &datasetGroupName,
        const QgsInterval &relativeTime = QgsInterval(),
        const QgsInterval &startTime = QgsInterval(),
        const QgsInterval &endTime = QgsInterval() ) const;

    /**
     *  Returns dataset group based on name, the dataset count contained in the group will depend on \a isAggregate.
     *  If isAggregate is TRUE, the dataset group will contained all dataset needed to calculate aggregate function.
     *  If isAggregate isFALSE, the datasetgroup will only contain dataset group needed to calculate non aggregate function.
     *  For example, if \a this instance is created with a relative time, a start time and an end time, that is a calculation that
     *  will be operated only for this relative time, dataset group involved in a aggregate function will have all the dataset between
     *  start time and end time, other dataset will only have the dataset corresponding to relative time.
     */
    std::shared_ptr<const QgsMeshMemoryDatasetGroup> group( const QString &groupName, bool isAggregate ) const;

    /**
     *  Creates dataset based on group. Initializes values and active based on group type.
     */
    std::shared_ptr<QgsMeshMemoryDataset> createMemoryDataset( const QgsMeshMemoryDatasetGroup &grp ) const;

    /**
     *  Creates dataset based on group. Fill with values of corresponding dataset
     */
    std::shared_ptr<QgsMeshMemoryDataset> createMemoryDataset( const QgsMeshDatasetIndex &datasetIndex ) const;

    /**
     *  Creates dataset with given type. Initializes values and active based on type.
     */
    std::shared_ptr<QgsMeshMemoryDataset> createMemoryDataset( const QgsMeshDatasetGroupMetadata::DataType type ) const;

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
    QMap < QString, std::shared_ptr<QgsMeshMemoryDatasetGroup> > mDatasetGroupMapForAggregate; //!< Groups that are referenced in the expression and used for aggregate function

    bool mIgnoreTime = false; // with virtual datasetgroup, we only consider the current time step, except for aggregate function where we don't care about time value
};

///@endcond

#endif // QGSMESHCALCUTILS_H
