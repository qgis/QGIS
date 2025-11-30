/***************************************************************************
                         qgsnativealgorithms.h
                         ---------------------
    begin                : April 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSNATIVEALGORITHMS_H
#define QGSNATIVEALGORITHMS_H

#include "qgis_analysis.h"
#include "qgis_sip.h"
#include "qgsprocessingprovider.h"

/**
 * \ingroup analysis
 * \class QgsNativeAlgorithms
 * \brief Native c++ processing algorithm provider.
 */
class ANALYSIS_EXPORT QgsNativeAlgorithms : public QgsProcessingProvider
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsNativeAlgorithms.
     */
    QgsNativeAlgorithms( QObject *parent = nullptr );

    [[nodiscard]] QIcon icon() const override;
    [[nodiscard]] QString svgIconPath() const override;
    [[nodiscard]] QString id() const override;
    [[nodiscard]] QString helpId() const override;
    [[nodiscard]] QString name() const override;
    [[nodiscard]] bool supportsNonFileBasedOutput() const override;
    [[nodiscard]] Qgis::ProcessingProviderFlags flags() const override;

  protected:
    void loadAlgorithms() override;
};

#endif // QGSNATIVEALGORITHMS_H
