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
    explicit QgsAuthConfigSelect( QWidget *parent = nullptr, const QString &dataprovider = QString() );
    ~QgsAuthConfigSelect();

    /** Set the authentication config id for the resource */
    void setConfigId( const QString& authcfg );

    /** Get the authentication config id for the resource */
    const QString configId() const { return mAuthCfg; }

    /** Set key of layer provider, if applicable */
    void setDataProviderKey( const QString &key );

  signals:
    /** Emitted when authentication config is changed or missing */
    void selectedConfigIdChanged( const QString& authcfg );

    /** Emitted when authentication config is removed */
    void selectedConfigIdRemoved( const QString& authcfg );

  public slots:
    /** Show a small message bar with a close button */
    void showMessage( const QString &msg );

    /** Clear and hide small message bar */
    void clearMessage();

  private slots:
    void loadConfig();
    void clearConfig();
    void validateConfig();
    void populateConfigSelector();

    void on_cmbConfigSelect_currentIndexChanged( int index );

    void on_btnConfigAdd_clicked();

    void on_btnConfigEdit_clicked();

    void on_btnConfigRemove_clicked();

    void on_btnConfigMsgClear_clicked();

  private:
    void loadAvailableConfigs();

    QString mAuthCfg;
    QString mDataProvider;
    QgsAuthMethodConfigsMap mConfigs;

    bool mDisabled;
    QVBoxLayout *mAuthNotifyLayout;
    QLabel *mAuthNotify;
};


//////////////// Embed in dialog ///////////////////

#include "ui_qgsauthconfiguriedit.h"

class QPushButton;

/** \ingroup gui
 * Dialog wrapper of select widget to edit an authcfg in a data source URI
 */
class GUI_EXPORT QgsAuthConfigUriEdit : public QDialog, private Ui::QgsAuthConfigUriEdit
{
    Q_OBJECT

  public:
    /**
     * Construct wrapper dialog for select widget to edit an authcfg in a data source URI
     * @param parent Parent widget
     * @param datauri URI QString with of without an authcfg=ID string
     * @param dataprovider The key of the calling layer provider, if applicable
     */
    explicit QgsAuthConfigUriEdit( QWidget *parent = nullptr,
                                   const QString &datauri = QString(),
                                   const QString &dataprovider = QString() );
    ~QgsAuthConfigUriEdit();

    /** Set the data source URI to parse */
    void setDataSourceUri( const QString &datauri );

    /** The returned, possibly edited data source URI */
    QString dataSourceUri();

    /** Whether a string contains an authcfg ID */
    static bool hasConfigID( const QString &txt );

  private slots:
    void saveChanges();

    void resetChanges();

    void authCfgUpdated( const QString &authcfg );

    void authCfgRemoved( const QString &authcfg );

  private:
    int authCfgIndex();

    QString authCfgFromUri();

    void selectAuthCfgInUri();

    void updateUriWithAuthCfg();

    void removeAuthCfgFromUri();

    QString mAuthCfg;
    QString mDataUri;
    QString mDataUriOrig;

    bool mDisabled;
    QVBoxLayout *mAuthNotifyLayout;
    QLabel *mAuthNotify;
};

#endif // QGSAUTHCONFIGSELECT_H
