/***************************************************************************
    qgsauthidentcertedit.h
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

#ifndef QGSAUTHIDENTCERTEDIT_H
#define QGSAUTHIDENTCERTEDIT_H

#include <QWidget>
#include "qgsauthmethodedit.h"
#include "ui_qgsauthidentcertedit.h"

#include "qgsauthconfig.h"


class QgsAuthIdentCertEdit : public QgsAuthMethodEdit, private Ui::QgsAuthIdentCertEdit
{
    Q_OBJECT

  public:
    explicit QgsAuthIdentCertEdit( QWidget *parent = nullptr );
    virtual ~QgsAuthIdentCertEdit();

    bool validateConfig() override;

    QgsStringMap configMap() const override;

  public slots:
    void loadConfig( const QgsStringMap &configmap ) override;

    void resetConfig() override;

    void clearConfig() override;

  private slots:
    void populateIdentityComboBox();

    void on_cmbIdentityCert_currentIndexChanged( int indx );

  private:
    QgsStringMap mConfigMap;
    bool mValid;
};

#endif // QGSAUTHIDENTCERTEDIT_H
