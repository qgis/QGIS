/***************************************************************************
                          qgsmeshcalculator.h
                          -------------------
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

#ifndef QGSMESHCALCULATOR_H
#define QGSMESHCALCULATOR_H

#include <QString>
#include <QVector>
#include <QStringList>

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsrectangle.h"
#include "qgsgeometry.h"
#include "qgsmeshdataset.h"
#include "qgsprovidermetadata.h"

class QgsMeshLayer;
class QgsFeedback;

/**
 * \ingroup core
 * \class QgsMeshCalculator
 * \brief Performs mesh layer calculations.
 *
 * Mesh calculator can do various mathematical operations
 * between dataset groups from a single mesh layer.
 * Resulting dataset group is added to the mesh layer.
 * Result can be filtered by extent or a vector layer mask
 * spatially and by selection of times.
 *
 * Resulting dataset is always scalar
 *
 * \since QGIS 3.6
*/
class CORE_EXPORT QgsMeshCalculator
{
  public:

    //! Result of the calculation
    enum Result
    {
      Success = 0, //!< Calculation successful
      Canceled, //!< Calculation canceled
      CreateOutputError, //!< Error creating output data file
      InputLayerError, //!< Error reading input layer
      ParserError, //!< Error parsing formula
      InvalidDatasets, //!< Datasets with different time outputs or not part of the mesh
      EvaluateError, //!< Error during evaluation
      MemoryError, //!< Error allocating memory for result
    };

    /**
     * Creates calculator with bounding box (rectangular) mask
     * \param formulaString formula/expression to evaluate. Consists of dataset group names, operators and numbers
     * \param outputFile file to store the resulting dataset group data
     * \param outputExtent spatial filter defined by rectangle
     * \param startTime time filter defining the starting dataset
     * \param endTime time filter defining the ending dataset
     * \param layer mesh layer with dataset groups references in formulaString
     *
     * \deprecated QGIS 3.12
     */
    Q_DECL_DEPRECATED QgsMeshCalculator( const QString &formulaString,
                                         const QString &outputFile,
                                         const QgsRectangle &outputExtent,
                                         double startTime,
                                         double endTime,
                                         QgsMeshLayer *layer ) SIP_DEPRECATED;

    /**
     * Creates calculator with geometry mask
     * \param formulaString formula/expression to evaluate. Consists of dataset group names, operators and numbers
     * \param outputFile file to store the resulting dataset group data
     * \param outputMask spatial filter defined by geometry
     * \param startTime time filter defining the starting dataset
     * \param endTime time filter defining the ending dataset
     * \param layer mesh layer with dataset groups references in formulaString
     *
     * \deprecated QGIS 3.12
     */
    Q_DECL_DEPRECATED QgsMeshCalculator( const QString &formulaString,
                                         const QString &outputFile,
                                         const QgsGeometry &outputMask,
                                         double startTime,
                                         double endTime,
                                         QgsMeshLayer *layer ) SIP_DEPRECATED;

    /**
     * Creates calculator with bounding box (rectangular) mask
     * \param formulaString formula/expression to evaluate. Consists of dataset group names, operators and numbers
     * \param outputDriver output driver name
     * \param outputGroupName output group name
     * \param outputFile file to store the resulting dataset group data
     * \param outputExtent spatial filter defined by rectangle
     * \param startTime time filter defining the starting dataset
     * \param endTime time filter defining the ending dataset
     * \param layer mesh layer with dataset groups references in formulaString
     *
     * \since QGIS 3.12
     */
    QgsMeshCalculator( const QString &formulaString,
                       const QString &outputDriver,
                       const QString &outputGroupName,
                       const QString &outputFile,
                       const QgsRectangle &outputExtent,
                       double startTime,
                       double endTime,
                       QgsMeshLayer *layer );

    /**
     * Creates calculator with geometry mask
     * \param formulaString formula/expression to evaluate. Consists of dataset group names, operators and numbers
     * \param outputDriver output driver name
     * \param outputGroupName output group name
     * \param outputFile file to store the resulting dataset group data
     * \param outputMask spatial filter defined by geometry
     * \param startTime time filter defining the starting dataset
     * \param endTime time filter defining the ending dataset
     * \param layer mesh layer with dataset groups references in formulaString
     *
     * \since QGIS 3.12
     */
    QgsMeshCalculator( const QString &formulaString,
                       const QString &outputDriver,
                       const QString &outputGroupName,
                       const QString &outputFile,
                       const QgsGeometry &outputMask,
                       double startTime,
                       double endTime,
                       QgsMeshLayer *layer );

    /**
     * Creates calculator with bounding box (rectangular) mask, store the result in \a destination (must be on memory or virtual),
     * see QgsMeshCalculator::Destination
     * \param formulaString formula/expression to evaluate. Consists of dataset group names, operators and numbers
     * \param outputGroupName output group name
     * \param outputExtent spatial filter defined by rectangle
     * \param destination destination of the calculation (memory or virtual)
     * \param startTime time filter defining the starting dataset
     * \param endTime time filter defining the ending dataset
     * \param layer mesh layer with dataset groups references in formulaString
     *
     * \since QGIS 3.16
     */
    QgsMeshCalculator( const QString &formulaString,
                       const QString &outputGroupName,
                       const QgsRectangle &outputExtent,
                       const QgsMeshDatasetGroup::Type &destination,
                       QgsMeshLayer *layer,
                       double startTime,
                       double endTime );

    /**
     * Creates calculator with with geometry mask, store the result in \a destination (must be on memory or virtual),
     *  see QgsMeshCalculator::Destination
     * \param formulaString formula/expression to evaluate. Consists of dataset group names, operators and numbers
     * \param outputGroupName output group name
     * \param outputMask spatial filter defined by geometry
     * \param destination destination of the calculation (memory or virtual)
     * \param startTime time filter defining the starting dataset
     * \param endTime time filter defining the ending dataset
     * \param layer mesh layer with dataset groups references in formulaString
     *
     * \since QGIS 3.16
     */
    QgsMeshCalculator( const QString &formulaString,
                       const QString &outputGroupName,
                       const QgsGeometry &outputMask,
                       const QgsMeshDatasetGroup::Type &destination,
                       QgsMeshLayer *layer,
                       double startTime,
                       double endTime );

    /**
     * Starts the calculation, creates new dataset group and adds it to the mesh layer
     * \param feedback The optional feedback argument for progress reporting and cancellation support
     * \returns QgsMeshCalculator::Success in case of success
     */
    Result processCalculation( QgsFeedback *feedback = nullptr );

    /**
     * Returns whether formula is valid for particular mesh layer
     * \param formulaString formula/expression to evaluate. Consists of dataset group names, operators and numbers
     * \param layer mesh layer with dataset groups references in formulaString
     * \returns QgsMeshCalculator::Success in case of success
     *
     * \deprecated QGIS 3.12 - use expressionIsValid
     */
    Q_DECL_DEPRECATED static Result expression_valid( const QString &formulaString,
        QgsMeshLayer *layer ) SIP_DEPRECATED;

    /**
     * Returns whether formula is valid for particular mesh layer
     * \param formulaString formula/expression to evaluate. Consists of dataset group names, operators and numbers
     * \param layer mesh layer with dataset groups references in formulaString
     * \param requiredCapability returns required capability of driver to store results of the calculation
     * \returns QgsMeshCalculator::Success in case of success
     *
     * \since QGIS 3.12
     */
    static Result expressionIsValid( const QString &formulaString,
                                     QgsMeshLayer *layer,
                                     QgsMeshDriverMetadata::MeshDriverCapability &requiredCapability );

  private:
    QgsMeshCalculator();

    QString mFormulaString;
    QString mOutputDriver;
    QString mOutputGroupName;
    QString mOutputFile;
    QgsRectangle mOutputExtent;
    QgsGeometry mOutputMask;
    bool mUseMask = false;
    QgsMeshDatasetGroup::Type mDestination = QgsMeshDatasetGroup::Persistent;
    double mStartTime = 0.0;
    double mEndTime = 0.0;
    QgsMeshLayer *mMeshLayer = nullptr;
};

#endif // QGSMESHCALCULATOR_H
