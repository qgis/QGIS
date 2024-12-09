/***************************************************************************
    qgsarcgisrestsourcewidget.h
     --------------------------------------
    Date                 : January 2021
    Copyright            : (C) 2021 by Nyall Dawson
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
#ifndef QGSARCGISRESTSOURCEWIDGET_H
#define QGSARCGISRESTSOURCEWIDGET_H

#include "qgsprovidersourcewidget.h"
#include "ui_qgsarcgisrestsourcewidgetbase.h"
#include <QVariantMap>

class QgsArcGisRestSourceWidget : public QgsProviderSourceWidget, private Ui::QgsArcGisRestSourceWidgetBase
{
    Q_OBJECT

  public:
    QgsArcGisRestSourceWidget( const QString &providerKey, QWidget *parent = nullptr );

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

#endif // QGSARCGISRESTSOURCEWIDGET_H
