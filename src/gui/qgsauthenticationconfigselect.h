/***************************************************************************
    qgsauthenticationconfigselect.h
    ---------------------
    begin                : October 5, 2014
    copyright            : (C) 2014 by Boundless Spatial, Inc. USA
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

#ifndef QGSAUTHENTICATIONCONFIGSELECT_H
#define QGSAUTHENTICATIONCONFIGSELECT_H

#include <QWidget>

#include "ui_qgsauthenticationconfigselect.h"
#include "qgsauthenticationconfig.h"

/** \ingroup gui
 * Selector widget for authentication configs
 * \since 2.8
 */
class GUI_EXPORT QgsAuthConfigSelect : public QWidget, private Ui::QgsAuthConfigSelect
{
    Q_OBJECT

  public:
    /**
     * Create a dialog for setting an associated authentication config, either
     * from existing configs, or creating/removing them from auth database
     *
     * @param keypasssupported  Whether the auth connection is capable of supporting PKI private key passphrases
     */
    explicit QgsAuthConfigSelect( QWidget *parent = 0, bool keypasssupported = true );
    ~QgsAuthConfigSelect();

    /** Set whether PKI private key passphrases are supported */
    void setKeyPassSupported( bool supported );

    /** Get whether PKI private key passphrases are supported */
    bool keyPassSupported() const { return mKeyPassSupported; }

    /** Set the authentication config id for the resource */
    void setConfigId( const QString& authid );

    /** Get the authentication config id for the resource */
    const QString configId() const { return mConfigId; }

  private slots:
    void loadConfig();
    void clearConfig();
    void validateConfig();
    void populateConfigSelector();

    void on_cmbConfigSelect_currentIndexChanged( int index );

    void on_btnConfigAdd_clicked();

    void on_btnConfigEdit_clicked();

    void on_btnConfigRemove_clicked();

  private:
    void loadAvailableConfigs();

    bool mKeyPassSupported;
    QString mConfigId;
    QHash<QString, QgsAuthConfigBase> mConfigs;

    QVBoxLayout *mAuthNotifyLayout;
    QLabel *mAuthNotify;
};

#endif // QGSAUTHENTICATIONCONFIGSELECT_H
