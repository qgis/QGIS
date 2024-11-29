/***************************************************************************
                         qgsactionlocatorfilter.h
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

#ifndef QGSACTIONLOCATORFILTERS_H
#define QGSACTIONLOCATORFILTERS_H

#include "qgis_app.h"
#include "qgslocatorfilter.h"


class QAction;


class APP_EXPORT QgsActionLocatorFilter : public QgsLocatorFilter
{
    Q_OBJECT

  public:
    QgsActionLocatorFilter( const QList<QWidget *> &parentObjectsForActions, QObject *parent = nullptr );
    QgsActionLocatorFilter *clone() const override;
    QString name() const override { return QStringLiteral( "actions" ); }
    QString displayName() const override { return tr( "Actions" ); }
    Priority priority() const override { return Lowest; }
    QString prefix() const override { return QStringLiteral( "." ); }
    QgsLocatorFilter::Flags flags() const override { return QgsLocatorFilter::FlagFast; }

    void fetchResults( const QString &string, const QgsLocatorContext &context, QgsFeedback *feedback ) override;
    void triggerResult( const QgsLocatorResult &result ) override;

  private:
    QList<QWidget *> mActionParents;

    void searchActions( const QString &string, QWidget *parent, QList<QAction *> &found );
};


#endif // QGSACTIONLOCATORFILTERS_H
