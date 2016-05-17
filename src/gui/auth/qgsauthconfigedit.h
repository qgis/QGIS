/***************************************************************************
    qgsauthconfigedit.h
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


#ifndef QGSAUTHCONFIGEDIT_H
#define QGSAUTHCONFIGEDIT_H

#include <QDialog>

#include "ui_qgsauthconfigedit.h"
#include "qgsauthconfig.h"

class QgsAuthMethodEdit;


/** \ingroup gui
 * Widget for editing an authentication configuration
 * \note not available in Python bindings
 */
class GUI_EXPORT QgsAuthConfigEdit : public QDialog, private Ui::QgsAuthConfigEdit
{
    Q_OBJECT

  public:
    /** Type of configuration validity */
    enum Validity
    {
      Valid,
      Invalid,
      Unknown
    };

    /**
     * Create a dialog for editing an authentication configuration
     * @param parent Parent widget
     * @param authcfg Authentication config id for a existing config in auth database
     * @param dataprovider The provider origin of the edit, to allow for customized code and filtering
     */
    explicit QgsAuthConfigEdit( QWidget *parent = nullptr, const QString& authcfg = QString(),
                                const QString &dataprovider = QString() );
    ~QgsAuthConfigEdit();

    /** Authentication config id, updated with generated id when a new config is saved to auth database */
    const QString configId() const { return mAuthCfg; }

  signals:
    /** Emit generated id when a new config is saved to auth database */
    void authenticationConfigStored( const QString& authcfg );

    /** Emit current id when an existing config is updated in auth database */
    void authenticationConfigUpdated( const QString& authcfg );

  private slots:
    void populateAuthMethods();

    void loadConfig();
    void resetConfig();
    void saveConfig();

    void on_btnClear_clicked();
    void clearAll();

    void validateAuth();

    void on_leName_textChanged( const QString& txt );

  private:
    int authMethodIndex( const QString &authMethodKey );

    QgsAuthMethodEdit *currentEditWidget();

    QString mAuthCfg;
    QString mDataProvider;
    QVBoxLayout *mAuthNotifyLayout;
    QLabel *mAuthNotify;
};

#endif // QGSAUTHCONFIGEDIT_H
