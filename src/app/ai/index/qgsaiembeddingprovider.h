/***************************************************************************
    qgsaiembeddingprovider.h
    ------------------------
    begin                : June 2026
    copyright            : (C) 2026 by Francesco Mazzi
    email                : francemazzi at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAIEMBEDDINGPROVIDER_H
#define QGSAIEMBEDDINGPROVIDER_H

#include "qgis_app.h"

#include <QList>
#include <QString>
#include <QStringList>
#include <QVector>

using namespace Qt::StringLiterals;

/**
 * Local embedding provider interface used by the workspace index.
 *
 * Implementations must not call external APIs unless the user has explicitly
 * selected a remote provider. The default product path is local/on-device.
 */
class APP_EXPORT QgsAiEmbeddingProvider
{
  public:
    virtual ~QgsAiEmbeddingProvider() = default;

    virtual QString providerId() const = 0;
    virtual QString displayName() const = 0;
    virtual bool isAvailable( QString *errorMessage = nullptr ) const = 0;
    virtual bool embed( const QStringList &texts, QList<QVector<float>> &out, QString *errorMessage = nullptr, int maxBatch = 64 ) = 0;
};

/**
 * Placeholder provider used until a local embedding model is packaged.
 */
class APP_EXPORT QgsAiUnavailableLocalEmbeddingProvider final : public QgsAiEmbeddingProvider
{
  public:
    QString providerId() const override { return u"local"_s; }
    QString displayName() const override { return u"Local embedding model"_s; }

    bool isAvailable( QString *errorMessage = nullptr ) const override
    {
      if ( errorMessage )
        *errorMessage = u"Local embedding model is not installed."_s;
      return false;
    }

    bool embed( const QStringList &texts, QList<QVector<float>> &out, QString *errorMessage = nullptr, int maxBatch = 64 ) override
    {
      Q_UNUSED( texts )
      Q_UNUSED( maxBatch )
      out.clear();
      if ( errorMessage )
        *errorMessage = u"Local embedding model is not installed."_s;
      return false;
    }
};

#endif // QGSAIEMBEDDINGPROVIDER_H
