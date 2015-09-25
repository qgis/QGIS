/***************************************************************************
    qgsauthconfigselect.h
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

#ifndef QGSAUTHCONFIGSELECT_H
#define QGSAUTHCONFIGSELECT_H

#include <QWidget>

#include "ui_qgsauthconfigselect.h"
#include "qgsauthconfig.h"


/** \ingroup gui
 * Selector widget for authentication configs
 */
class GUI_EXPORT QgsAuthConfigSelect : public QWidget, private Ui::QgsAuthConfigSelect
{
    Q_OBJECT

  public:
    /**
     * Create a dialog for setting an associated authentication config, either
     * from existing configs, or creating/removing them from auth database
     * @param parent Parent widget
     * @param dataprovider The key of the calling layer provider, if applicable
     */
    explicit QgsAuthConfigSelect( QWidget *parent = 0, const QString &dataprovider = QString() );
    ~QgsAuthConfigSelect();

    /** Set the authentication config id for the resource */
    void setConfigId( const QString& authcfg );

    /** Get the authentication config id for the resource */
    const QString configId() const { return mAuthCfg; }

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

    QString mAuthCfg;
    QString mDataProvider;
    QgsAuthMethodConfigsMap mConfigs;

    QVBoxLayout *mAuthNotifyLayout;
    QLabel *mAuthNotify;
};

#endif // QGSAUTHCONFIGSELECT_H
