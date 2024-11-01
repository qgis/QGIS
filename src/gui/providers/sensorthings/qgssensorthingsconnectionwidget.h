/***************************************************************************
    qgssensorthingsconnectionwidget.h
     --------------------------------------
    Date                 : December 2023
    Copyright            : (C) 2023 by Nyall Dawson
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
#ifndef QGSSENSORTHINGSCONNECTIONWIDGET_H
#define QGSSENSORTHINGSCONNECTIONWIDGET_H

#include "ui_qgssensorthingsconnectionwidgetbase.h"
#include <QVariantMap>

#define SIP_NO_FILE
///@cond PRIVATE

class QgsSensorThingsConnectionWidget : public QWidget, private Ui::QgsSensorThingsConnectionWidgetBase
{
    Q_OBJECT

  public:
    QgsSensorThingsConnectionWidget( QWidget *parent = nullptr );

    void setSourceUri( const QString &uri );
    QString sourceUri() const;

    void setUrl( const QString &url );
    QString url() const;

    void setUsername( const QString &username );
    void setPassword( const QString &password );
    void setAuthCfg( const QString &id );

    QString username() const;
    QString password() const;
    QString authcfg() const;

    void setReferer( const QString &referer );
    QString referer() const;

    bool isValid() const { return mIsValid; }

  signals:

    void changed();
    void validChanged( bool valid );

  private slots:

    void validate();

  private:
    QVariantMap mSourceParts;
    bool mIsValid = false;
};

///@endcond PRIVATE
#endif // QGSSENSORTHINGSCONNECTIONWIDGET_H
