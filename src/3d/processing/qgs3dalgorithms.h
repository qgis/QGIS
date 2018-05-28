/***************************************************************************
                         qgs3dalgorithms.h
                         ---------------------
    begin                : November 2017
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

#ifndef QGS3DALGORITHMS_H
#define QGS3DALGORITHMS_H

#include "qgis_3d.h"
#include "qgis.h"
#include "processing/qgsprocessingprovider.h"

/**
 * \ingroup analysis
 * \class Qgs3DAlgorithms
 * \brief QGIS 3D processing algorithm provider.
 * \since QGIS 3.0
 */
class _3D_EXPORT Qgs3DAlgorithms: public QgsProcessingProvider
{
    Q_OBJECT

  public:

    /**
     * Constructor for Qgs3DAlgorithms.
     */
    Qgs3DAlgorithms( QObject *parent = nullptr );

    QIcon icon() const override;
    QString svgIconPath() const override;
    QString id() const override;
    QString helpId() const override;
    QString name() const override;
    bool supportsNonFileBasedOutput() const override;

  protected:

    void loadAlgorithms() override;

};

#endif // QGS3DALGORITHMS_H


