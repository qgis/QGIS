/***************************************************************************
    qgsauthbasicedit.h
    ---------------------
    begin                : September 1, 2015
    copyright            : (C) 2015 by Boundless Spatial, Inc. USA
    author               : Larry Shaffer
    email                : lshaffer at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAUTHBASICEDIT_H
#define QGSAUTHBASICEDIT_H

#include <QWidget>
#include "qgsauthmethodedit.h"
#include "ui_qgsauthbasicedit.h"

#include "qgsauthconfig.h"


class QgsAuthBasicEdit : public QgsAuthMethodEdit, private Ui::QgsAuthBasicEdit
{
    Q_OBJECT

  public:
    explicit QgsAuthBasicEdit( QWidget *parent = nullptr );
    virtual ~QgsAuthBasicEdit();

    bool validateConfig() override;

    QgsStringMap configMap() const override;

  public slots:
    void loadConfig( const QgsStringMap &configmap ) override;

    void resetConfig() override;

    void clearConfig() override;

  private slots:
    void on_leUsername_textChanged( const QString& txt );

    void on_chkPasswordShow_stateChanged( int state );

  private:
    QgsStringMap mConfigMap;
    bool mValid;
};

#endif // QGSAUTHBASICEDIT_H
