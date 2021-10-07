/***************************************************************************
    qgsauthapiheaderedit.h
    ---------------------
    begin                : October 2021
    copyright            : (C) 2021 by Nyall Dawson
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

#ifndef QGSAUTHAPIHEADEREDIT_H
#define QGSAUTHAPIHEADEREDIT_H

#include <QWidget>
#include "qgsauthmethodedit.h"
#include "ui_qgsauthapiheaderedit.h"

#include "qgsauthconfig.h"


class QgsAuthApiHeaderEdit : public QgsAuthMethodEdit, private Ui::QgsAuthApiHeaderEdit
{
    Q_OBJECT

  public:
    explicit QgsAuthApiHeaderEdit( QWidget *parent = nullptr );

    bool validateConfig() override;

    QgsStringMap configMap() const override;

  public slots:
    void loadConfig( const QgsStringMap &configmap ) override;

    void resetConfig() override;

    void clearConfig() override;

  private slots:
    void textChanged( const QString &txt );

  private:
    QgsStringMap mConfigMap;
    bool mValid = false;
};

#endif // QGSAUTHAPIHEADEREDIT_H
