/***************************************************************************
    qgsauthpkipathsedit.h
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

#ifndef QGSAUTHPKIPATHSEDIT_H
#define QGSAUTHPKIPATHSEDIT_H

#include <QWidget>
#include "qgsauthmethodedit.h"
#include "ui_qgsauthpkipathsedit.h"

#include "qgsauthconfig.h"


class QgsAuthPkiPathsEdit : public QgsAuthMethodEdit, private Ui::QgsAuthPkiPathsEdit
{
    Q_OBJECT

  public:
    enum Validity
    {
      Valid,
      Invalid,
      Unknown
    };

    explicit QgsAuthPkiPathsEdit( QWidget *parent = nullptr );
    virtual ~QgsAuthPkiPathsEdit();

    bool validateConfig() override;

    QgsStringMap configMap() const override;

  public slots:
    void loadConfig( const QgsStringMap &configmap ) override;

    void resetConfig() override;

    void clearConfig() override;

  private slots:
    void clearPkiMessage( QLineEdit *lineedit );
    void writePkiMessage( QLineEdit *lineedit, const QString& msg, Validity valid = Unknown );

    void clearPkiPathsCertPath();
    void clearPkiPathsKeyPath();
    void clearPkiPathsKeyPass();

    void on_chkPkiPathsPassShow_stateChanged( int state );

    void on_btnPkiPathsCert_clicked();

    void on_btnPkiPathsKey_clicked();

  private:
    bool validityChange( bool curvalid );

    QgsStringMap mConfigMap;
    bool mValid;
};

#endif // QGSAUTHPKIPATHSEDIT_H
