/***************************************************************************
  qgsauthawss3edit.h
  --------------------------------------
  Date                 : December 2022
  Copyright            : (C) 2022 by Jacky Volpes
  Email                : jacky dot volpes at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAUTHAWSS3EDIT_H
#define QGSAUTHAWSS3EDIT_H

#include <QWidget>

#include "qgsauthmethodedit.h"
#include "ui_qgsauthawss3edit.h"

#include "qgsauthconfig.h"


class QgsAuthAwsS3Edit : public QgsAuthMethodEdit, private Ui::QgsAuthAwsS3Edit
{
    Q_OBJECT

  public:
    explicit QgsAuthAwsS3Edit( QWidget *parent = nullptr );

    bool validateConfig() override;

    QgsStringMap configMap() const override;

  public slots:
    void loadConfig( const QgsStringMap &configmap ) override;

    void resetConfig() override;

    void clearConfig() override;

  private slots:
    void leUsername_textChanged( const QString &txt );

    void lePassword_textChanged( const QString &txt );

    void leRegion_textChanged( const QString &txt );

    void chkPasswordShow_stateChanged( int state );

  private:
    QgsStringMap mConfigMap;
    bool mValid = false;
};

#endif // QGSAUTHAWSS3EDIT_H
