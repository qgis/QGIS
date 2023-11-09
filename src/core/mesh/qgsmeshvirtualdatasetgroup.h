/***************************************************************************
                         qgsmeshvirtualdatasetgroup.h
                         ---------------------
    begin                : June 2020
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

#ifndef QGSMESHVIRTUALDATASETGROUP_H
#define QGSMESHVIRTUALDATASETGROUP_H

#include "qgsmeshdataset.h"
#include "qgsmeshcalcnode.h"

#include "qgsmeshlayertemporalproperties.h"

#define SIP_NO_FILE

/**
 * \ingroup core
 * \class QgsMeshVirtualDatasetGroup
 * \brief Represents a dataset group calculated from a formula string.
 *
 * The calculation is done by the QgsMeshCalcUtils class from a QgsMeshCalcNode created from the formula
 * Each dataset is calculted when needed, so there is only one dataset store in this class all the time.
 *
 * \since QGIS 3.16
 */
class CORE_EXPORT QgsMeshVirtualDatasetGroup: public QgsMeshDatasetGroup
{
  public:

    /**
     * Constructor
     * \param name name of the dataset group
     * \param formulaString formula use to define the dataset group
     * \param layer mesh layer that contains dataset group
     * \param relativeStartTime relative time start, in mimliseconds, from the mesh layer provider reference time
     * \param relativeEndTime relative time end, in mimliseconds, from the mesh layer provider reference time
     */
    QgsMeshVirtualDatasetGroup( const QString &name,
                                const QString &formulaString,
                                QgsMeshLayer *layer,
                                qint64 relativeStartTime,
                                qint64 relativeEndTime );

    void initialize() override;
    int datasetCount() const override;
    QgsMeshDataset *dataset( int index ) const override;
    QgsMeshDatasetMetadata datasetMetadata( int datasetIndex ) const override;
    QStringList datasetGroupNamesDependentOn() const override;
    QDomElement writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const override;
    QString description() const override;
    QgsMeshDatasetGroup::Type type() const override {return QgsMeshDatasetGroup::Virtual;}

  private:
    QString mFormula;
    std::unique_ptr<QgsMeshCalcNode> mCalcNode;
    QgsMeshLayer *mLayer = nullptr;
    qint64 mStartTime = 0;
    qint64 mEndTime = 0;
    QStringList mDatasetGroupNameUsed;
    QStringList mDatasetGroupNameUsedForAggregate;
    QList<qint64> mDatasetTimes;

    mutable std::shared_ptr<QgsMeshMemoryDataset> mCacheDataset;
    mutable QVector<QgsMeshDatasetMetadata> mDatasetMetaData;
    mutable int mCurrentDatasetIndex = -1;

    bool calculateDataset() const;
};

#endif // QGSMESHVIRTUALDATASETGROUP_H
