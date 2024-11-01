/***************************************************************************
    qgsauthconfigidedit.h
    ---------------------
    begin                : September, 2015
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

#ifndef QGSAUTHCONFIGIDEDIT_H
#define QGSAUTHCONFIGIDEDIT_H

#include "ui_qgsauthconfigidedit.h"

#include <QWidget>
#include "qgis_gui.h"

#define SIP_NO_FILE

/**
 * \ingroup gui
 * \brief Custom widget for editing an authentication configuration ID
 * \note Validates the input against the database and for ID's 7-character alphanumeric syntax
 * \note not available in Python bindings
 */
class GUI_EXPORT QgsAuthConfigIdEdit : public QWidget, private Ui::QgsAuthConfigIdEdit
{
    Q_OBJECT

  public:
    /**
     * Widget to unlock and edit an authentication configuration ID
     * \param parent Parent widget
     * \param authcfg Authentication configuration ID
     * \param allowEmpty Whether to allow no ID to be set, even when editing, e.g. Add config functions
     */
    explicit QgsAuthConfigIdEdit( QWidget *parent = nullptr, const QString &authcfg = QString(), bool allowEmpty = true );

    //! The authentication configuration ID, if valid, otherwise null QString
    QString const configId();

    //! Whether to allow no ID to be set
    bool allowEmptyId() { return mAllowEmpty; }

    //! Validate the widget state and ID
    bool validate();

  signals:
    //! Validity of the ID has changed
    void validityChanged( bool valid );

  public slots:
    //! Sets the authentication configuration ID, storing it, and validating the passed value
    void setAuthConfigId( const QString &authcfg );

    //! Sets whether to allow no ID to be set
    void setAllowEmptyId( bool allowed );

    //! Clear all of the widget's editing state and contents
    void clear();

  private slots:
    void updateValidityStyle( bool valid );

    void btnLock_toggled( bool checked );

    void leAuthCfg_textChanged( const QString &txt );

  private:
    bool isAlphaNumeric( const QString &authcfg );

    QString mAuthCfgOrig;
    bool mValid = false;
    bool mAllowEmpty;
};

#endif // QGSAUTHCONFIGIDEDIT_H
