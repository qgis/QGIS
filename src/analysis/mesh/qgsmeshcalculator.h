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

#include "qgis_analysis.h"
#include "qgsrectangle.h"
#include "qgsmeshlayer.h"
#include "qgsmeshdataprovider.h"
#include "qgsfeedback.h"

struct QgsMeshMemoryDatasetGroup;
struct QgsMeshMemoryDataset;

/**
 * \ingroup analysis
 * \class QgsMeshCalculator
 * Performs mesh layer calculations.
 *
 * Mesh calculator can do various mathematical operations
 * between dataset groups from a single mesh layer.
 * Resulting dataset group is added to the mesh layer.
 * Result can be filtered by extent or a vector layer mask
 * spatially and by selection of times.
 *
 * Note: only dataset groups defined on vertices are
 * implemented and supported
 *
 * \since QGIS 3.6
*/
class ANALYSIS_EXPORT QgsMeshCalculator
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
     */
    QgsMeshCalculator( const QString &formulaString,
                       const QString &outputFile,
                       const QgsRectangle &outputExtent,
                       double startTime,
                       double endTime,
                       QgsMeshLayer *layer );

    /**
     * Creates calculator with geometry mask
     * \param formulaString formula/expression to evaluate. Consists of dataset group names, operators and numbers
     * \param outputFile file to store the resulting dataset group data
     * \param outputMask spatial filter defined by geometry
     * \param startTime time filter defining the starting dataset
     * \param endTime time filter defining the ending dataset
     * \param layer mesh layer with dataset groups references in formulaString
     */
    QgsMeshCalculator( const QString &formulaString,
                       const QString &outputFile,
                       const QgsGeometry &outputMask,
                       double startTime,
                       double endTime,
                       QgsMeshLayer *layer );

    /**
     * Starts the calculation, writes new dataset group to file and adds it to the mesh layer
     * \param feedback The optional feedback argument for progress reporting and cancellation support
     * \returns QgsMeshCalculator::Success in case of success
     */
    Result processCalculation( QgsFeedback *feedback = nullptr );

    /**
     * Returns whether formula is valid for particular mesh layer
     * \param formulaString formula/expression to evaluate. Consists of dataset group names, operators and numbers
     * \param layer mesh layer with dataset groups references in formulaString
     * \returns QgsMeshCalculator::Success in case of success
     */
    static Result expression_valid( const QString &formulaString, QgsMeshLayer *layer );

  private:
    QgsMeshCalculator();

    QString mFormulaString;
    QString mOutputFile;
    QgsRectangle mOutputExtent;
    QgsGeometry mOutputMask;
    bool mUseMask = false;
    double mStartTime = 0.0;
    double mEndTime = 0.0;
    QgsMeshLayer *mMeshLayer = nullptr;
};

#endif // QGSMESHCALCULATOR_H
