/***************************************************************************
                         qgsinbuiltlocatorfilters.h
                         --------------------------
    begin                : May 2017
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

#ifndef QGSINBUILTLOCATORFILTERS_H
#define QGSINBUILTLOCATORFILTERS_H

#include "qgslocatorfilter.h"

class QgsLayerTreeLocatorFilter : public QgsLocatorFilter
{
    Q_OBJECT

  public:

    QgsLayerTreeLocatorFilter( QObject *parent = nullptr );
    void fetchResults( const QString &string, const QgsLocatorContext &context, QgsFeedback *feedback ) override;
    void triggerResult( const QgsLocatorResult &result ) override;

};

class QgsLayoutLocatorFilter : public QgsLocatorFilter
{
    Q_OBJECT

  public:

    QgsLayoutLocatorFilter( QObject *parent = nullptr );
    void fetchResults( const QString &string, const QgsLocatorContext &context, QgsFeedback *feedback ) override;
    void triggerResult( const QgsLocatorResult &result ) override;

};

#endif // QGSINBUILTLOCATORFILTERS_H


