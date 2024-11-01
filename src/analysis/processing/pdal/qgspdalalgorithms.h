/***************************************************************************
                         qgspdalalgorithms.h
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

#ifndef QGSPDALALGORITHMS_H
#define QGSPDALALGORITHMS_H

#include "qgis_analysis.h"
#include "qgis_sip.h"
#include "qgsprocessingprovider.h"

SIP_IF_MODULE( HAVE_PDAL_PROVIDER )

/**
 * \ingroup analysis
 * \class QgsPdalAlgorithms
 * \brief PDAL Processing algorithm provider.
 * \since QGIS 3.32
 */

class ANALYSIS_EXPORT QgsPdalAlgorithms : public QgsProcessingProvider
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsPdalAlgorithms.
     */
    QgsPdalAlgorithms( QObject *parent = nullptr );

    QIcon icon() const override;
    QString svgIconPath() const override;
    QString id() const override;
    QString helpId() const override;
    QString name() const override;
    bool supportsNonFileBasedOutput() const override;

    QStringList supportedOutputVectorLayerExtensions() const override;
    QStringList supportedOutputRasterLayerExtensions() const override;
    QStringList supportedOutputPointCloudLayerExtensions() const override;

  protected:
    void loadAlgorithms() override;
};

#endif // QGSPDALALGORITHMS_H
