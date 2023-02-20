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
    virtual QStringList createArgumentLists( const QVariantMap &parameters,
        QgsProcessingContext &context, QgsProcessingFeedback *feedback );

  protected:
    void setOutputValue( const QString &name, const QVariant &value );

    /**
     * Adds max number of concurrent threads for parallel runs parameter
     * if user has MAX_THREADS option set in Processing options.
     */
    void addThreadsParameter( QStringList &arguments );

    /**
     * Returns path to the pdal_wrench executable binary.
     */
    QString wrenchExecutableBinary() const;

    QVariantMap processAlgorithm( const QVariantMap &parameters,
                                  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    QMap<QString, QVariant> mOutputValues;
};

///@endcond PRIVATE

#endif // QGSPDALALGORITHMBASE_H
