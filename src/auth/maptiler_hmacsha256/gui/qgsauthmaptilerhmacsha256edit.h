/***************************************************************************
    qgsauthmaptilerhmacsha256edit.h
    ---------------------
    begin                : January 2022
    copyright            : (C) 2022 by Vincent Cloarec
    author               : Vincent Cloarec
    email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAUTHQGSAUTHHMACSHA256EDIT_H
#define QGSAUTHQGSAUTHHMACSHA256EDIT_H

#include <QWidget>
#include "qgsauthmethodedit.h"
#include "ui_qgsauthmaptilerhmacsha256edit.h"

#include "qgsauthconfig.h"


class QgsAuthMapTilerHmacSha256Edit : public QgsAuthMethodEdit, private Ui::QgsAuthEsriTokenEdit
{
    Q_OBJECT

  public:
    explicit QgsAuthMapTilerHmacSha256Edit( QWidget *parent = nullptr );

    bool validateConfig() override;

    QgsStringMap configMap() const override;

  public slots:
    void loadConfig( const QgsStringMap &configmap ) override;

    void resetConfig() override;

    void clearConfig() override;

  private slots:
    void configChanged();

  private:
    QgsStringMap mConfigMap;
    bool mValid = false;
};

#endif // QGSAUTHQGSAUTHHMACSHA256EDIT_H
