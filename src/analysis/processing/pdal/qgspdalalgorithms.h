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

    [[nodiscard]] QIcon icon() const override;
    [[nodiscard]] QString svgIconPath() const override;
    [[nodiscard]] QString id() const override;
    [[nodiscard]] QString helpId() const override;
    [[nodiscard]] QString name() const override;
    [[nodiscard]] bool supportsNonFileBasedOutput() const override;

    [[nodiscard]] QStringList supportedOutputVectorLayerExtensions() const override;
    [[nodiscard]] QStringList supportedOutputRasterLayerExtensions() const override;
    [[nodiscard]] QStringList supportedOutputPointCloudLayerExtensions() const override;

  protected:
    void loadAlgorithms() override;
};

#endif // QGSPDALALGORITHMS_H
