/***************************************************************************
    qgsauthplanetarycomputeredit.h
    ------------------------
    begin                : August 2025
    copyright            : (C) 2025 by Stefanos Natsis
    author               : Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAUTHPLANETARYCOMPUTEREDIT_H
#define QGSAUTHPLANETARYCOMPUTEREDIT_H

#include "ui_qgsauthplanetarycomputeredit.h"

#include "qgsauthconfig.h"
#include "qgsauthmethodedit.h"

#include <QWidget>

class QgsAuthPlanetaryComputerEdit : public QgsAuthMethodEdit, private Ui::QgsAuthPlanetaryComputerEdit
{
    Q_OBJECT

  public:
    explicit QgsAuthPlanetaryComputerEdit( QWidget *parent = nullptr );

    bool validateConfig() override;

    [[nodiscard]] QgsStringMap configMap() const override;

  public slots:
    void loadConfig( const QgsStringMap &configmap ) override;

    void resetConfig() override;

    void clearConfig() override;

  private:
    void updateServerType( int indx );

    QgsStringMap mConfigMap;
    bool mValid = false;

    static const QString REQUEST_URL_TEMPLATE;
    static const QString TOKEN_URL_TEMPLATE;
    static const QString SCOPE;
};

#endif // QGSAUTHPLANETARYCOMPUTEREDIT_H
