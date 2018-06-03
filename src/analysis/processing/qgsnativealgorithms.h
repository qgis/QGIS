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
#include "qgis.h"
#include "qgsprocessingprovider.h"

/**
 * \ingroup analysis
 * \class QgsNativeAlgorithms
 * \brief Native c++ processing algorithm provider.
 * \since QGIS 3.0
 */
class ANALYSIS_EXPORT QgsNativeAlgorithms: public QgsProcessingProvider
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsNativeAlgorithms.
     */
    QgsNativeAlgorithms( QObject *parent = nullptr );

    QIcon icon() const override;
    QString svgIconPath() const override;
    QString id() const override;
    QString helpId() const override;
    QString name() const override;
    bool supportsNonFileBasedOutput() const override;

  protected:

    void loadAlgorithms() override;

};

#endif // QGSNATIVEALGORITHMS_H


