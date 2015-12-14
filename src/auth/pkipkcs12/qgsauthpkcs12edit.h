/***************************************************************************
    qgsauthpkcs12edit.h
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

#ifndef QGSAUTHPKCS12EDIT_H
#define QGSAUTHPKCS12EDIT_H

#include <QWidget>
#include "qgsauthmethodedit.h"
#include "ui_qgsauthpkcs12edit.h"

#include "qgsauthconfig.h"


class QgsAuthPkcs12Edit : public QgsAuthMethodEdit, private Ui::QgsAuthPkcs12Edit
{
    Q_OBJECT

  public:
    enum Validity
    {
      Valid,
      Invalid,
      Unknown
    };

    explicit QgsAuthPkcs12Edit( QWidget *parent = nullptr );
    virtual ~QgsAuthPkcs12Edit();

    bool validateConfig() override;

    QgsStringMap configMap() const override;

  public slots:
    void loadConfig( const QgsStringMap &configmap ) override;

    void resetConfig() override;

    void clearConfig() override;

  private slots:
    void clearPkiMessage( QLineEdit *lineedit );
    void writePkiMessage( QLineEdit *lineedit, const QString& msg, Validity valid = Unknown );

    void clearPkcs12BundlePath();
    void clearPkcs12BundlePass();

    void on_lePkcs12KeyPass_textChanged( const QString &pass );
    void on_chkPkcs12PassShow_stateChanged( int state );

    void on_btnPkcs12Bundle_clicked();

  private:
    bool validityChange( bool curvalid );

    QgsStringMap mConfigMap;
    bool mValid;
};

#endif // QGSAUTHPKCS12EDIT_H
