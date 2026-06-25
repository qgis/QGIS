/***************************************************************************
    qgsarcgisimageserversourcewidget.h
     --------------------------------------
    Date                 : April 2026
    Copyright            : (C) 2026 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSARCGISIMAGESERVERSOURCEWIDGET_H
#define QGSARCGISIMAGESERVERSOURCEWIDGET_H

#include "ui_qgsarcgisimageserversourcewidgetbase.h"

#include "qgsprovidersourcewidget.h"

#include <QVariantMap>

class QgsMapLayer;

class QgsArcGisImageServerSourceWidget : public QgsProviderSourceWidget, private Ui::QgsArcGisImageServerSourceWidgetBase
{
    Q_OBJECT

  public:
    QgsArcGisImageServerSourceWidget( QgsMapLayer *layer, QWidget *parent = nullptr );

    void setSourceUri( const QString &uri ) override;
    QString sourceUri() const override;

    void setUsername( const QString &username );
    void setPassword( const QString &password );
    void setAuthCfg( const QString &id );

    QString username() const;
    QString password() const;
    QString authcfg() const;

    void setReferer( const QString &referer );
    QString referer() const;

  private:
    const QString mProviderKey;
    QVariantMap mSourceParts;
};

#endif // QGSARCGISIMAGESERVERSOURCEWIDGET_H
