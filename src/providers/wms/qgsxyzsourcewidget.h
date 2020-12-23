/***************************************************************************
    qgsxyzsourcewidget.h
     --------------------------------------
    Date                 : December 2020
    Copyright            : (C) 2020 by Nyall Dawson
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
#ifndef QGGXYZSOURCEWIDGET_H
#define QGGXYZSOURCEWIDGET_H

#include "qgsprovidersourcewidget.h"
#include "ui_qgsxyzsourcewidgetbase.h"
#include <QVariantMap>

class QgsXyzSourceWidget : public QgsProviderSourceWidget, private Ui::QgsXyzSourceWidgetBase
{
    Q_OBJECT

  public:
    QgsXyzSourceWidget( QWidget *parent = nullptr );

    void setSourceUri( const QString &uri ) override;
    QString sourceUri() const override;

  private slots:

    void validate();

  private:

    QVariantMap mSourceParts;
    bool mIsValid = false;
};

#endif // QGGXYZSOURCEWIDGET_H
