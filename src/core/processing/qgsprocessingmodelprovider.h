/***************************************************************************
                         qgsprocessingmodelprovider.h
                         ------------------------
    begin                : September 2026
    copyright            : (C) 2026 by Nyall Dawson
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

#ifndef QGSPROCESSINGMODELPROVIDER_H
#define QGSPROCESSINGMODELPROVIDER_H

#include "qgis_core.h"
#include "qgsprocessingprovider.h"

#include <QDomDocument>
#include <QPointer>

class QgsProcessingModelAlgorithm;

/**
 * \brief A Processing provider for Processing models.
 * \ingroup core
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsProcessingModelProvider : public QgsProcessingProvider
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsProcessingModelProvider.
     */
    QgsProcessingModelProvider( QObject *parent SIP_TRANSFERTHIS = nullptr );

    QString name() const override;
    QString id() const override;
    QIcon icon() const override;
    QString svgIconPath() const override;
    bool supportsNonFileBasedOutput() const override;
    bool load() override;

  protected:
    void loadAlgorithms() override;

  private slots:
    void onProviderAdded( const QString &providerId );

  private:
    void loadFromFolder( const QString &folder );

    bool mIsLoading = false;
};

#endif // QGSPROCESSINGMODELPROVIDER_H
