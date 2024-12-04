/***************************************************************************
                         qgspdalalgorithmbase.h
                         ---------------------
    begin                : February 2023
    copyright            : (C) 2023 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPDALALGORITHMBASE_H
#define QGSPDALALGORITHMBASE_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Base class for PDAL algorithms.
 */
class QgsPdalAlgorithmBase : public QgsProcessingAlgorithm
{
  public:
    /**
     * Builds a command line string to run required pdal_wrench tool.
     */
    virtual QStringList createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback );

  protected:
    void setOutputValue( const QString &name, const QVariant &value );

    void enableElevationPropertiesPostProcessor( bool enable );

    /**
     * Creates common advanced parameters, such as expression and rectangle filters
     */
    void createCommonParameters();

    /**
     * Evaluates common advanced parameters and adds them to the pdal_wrench
     * command line.
     */
    void applyCommonParameters( QStringList &arguments, QgsCoordinateReferenceSystem crs, const QVariantMap &parameters, QgsProcessingContext &context );

    /**
     * Adds "--threads"parameter to the pdal_wrench command line.
     */
    void applyThreadsParameter( QStringList &arguments, QgsProcessingContext &context );

    /**
     * "Fixes" output file name by changing suffix to .vpc if input file
     * is a VPC and output is a temporary output.
     *
     * This is necessary as pdal_wrench at the moment can create only VPC
     * output if input file is a VPC. We automatically adjust output file
     * extension for temporary outputs to provide better UX. For normal outputs
     * user will see a error if output file is not a VPC.
     */
    QString fixOutputFileName( const QString &inputFileName, const QString &outputFileName, QgsProcessingContext &context );

    /**
     * Checks that output file has correct format (LAS/LAZ) throws an exception
     * if this is not the case.
     *
     * This check is needed because most of the pdal_wrench tools don't support
     * writing COPC as they are being run in the streaming mode.
     *
     * Also checks that output file is not a VPC file when input is not a VPC.
     */
    void checkOutputFormat( const QString &inputFileName, const QString &outputFileName );

    /**
     * Returns path to the pdal_wrench executable binary.
     */
    QString wrenchExecutableBinary() const;

    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    QMap<QString, QVariant> mOutputValues;
    bool mEnableElevationProperties = false;
};

///@endcond PRIVATE

#endif // QGSPDALALGORITHMBASE_H
