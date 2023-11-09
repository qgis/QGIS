/***************************************************************************
    qgsauthesritokenedit.h
    ---------------------
    begin                : October 2018
    copyright            : (C) 2018 by Nyall Dawson
    author               : Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAUTHESRITOKENEDIT_H
#define QGSAUTHESRITOKENEDIT_H

#include <QWidget>
#include "qgsauthmethodedit.h"
#include "ui_qgsauthesritokenedit.h"

#include "qgsauthconfig.h"


class QgsAuthEsriTokenEdit : public QgsAuthMethodEdit, private Ui::QgsAuthEsriTokenEdit
{
    Q_OBJECT

  public:
    explicit QgsAuthEsriTokenEdit( QWidget *parent = nullptr );

    bool validateConfig() override;

    QgsStringMap configMap() const override;

  public slots:
    void loadConfig( const QgsStringMap &configmap ) override;

    void resetConfig() override;

    void clearConfig() override;

  private slots:
    void tokenChanged();

  private:
    QgsStringMap mConfigMap;
    bool mValid = false;
};

#endif // QGSAUTHESRITOKENEDIT_H
