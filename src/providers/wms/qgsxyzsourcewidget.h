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

class QgsWmsInterpretationComboBox;

class QgsXyzSourceWidget : public QgsProviderSourceWidget, private Ui::QgsXyzSourceWidgetBase
{
    Q_OBJECT

  public:
    QgsXyzSourceWidget( QWidget *parent = nullptr );

    void setSourceUri( const QString &uri ) override;
    QString sourceUri() const override;

    void setUrl( const QString &url );
    QString url() const;

    void setZMin( int zMin );
    int zMin() const;

    void setZMax( int zMax );
    int zMax() const;

    void setUsername( const QString &username );
    void setPassword( const QString &password );
    void setAuthCfg( const QString &id );

    QString username() const;
    QString password() const;
    QString authcfg() const;

    void setReferer( const QString &referer );
    QString referer() const;

    void setTilePixelRatio( int ratio );
    int tilePixelRatio() const;

    void setInterpretation( const QString &interpretation );
    QString interpretation() const;

  private slots:

    void validate();

  private:

    QVariantMap mSourceParts;
    bool mIsValid = false;

    QgsWmsInterpretationComboBox *mInterpretationCombo = nullptr;
};

#endif // QGGXYZSOURCEWIDGET_H
