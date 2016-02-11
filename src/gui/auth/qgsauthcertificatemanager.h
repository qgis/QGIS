/***************************************************************************
    qgsauthcertificatemanager.h
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

#ifndef QGSAUTHCERTIFICATEMANAGER_H
#define QGSAUTHCERTIFICATEMANAGER_H

#include "ui_qgsauthcertificatemanager.h"

#include <QWidget>
#include <QDialog>


/** \ingroup gui
 * Wrapper widget to manage available certificate editors
 */
class GUI_EXPORT QgsAuthCertEditors : public QWidget, private Ui::QgsAuthCertManager
{
    Q_OBJECT

  public:
    /**
     * Construct a widget to contain various certificate editors
     * @param parent Parent widget
     */
    explicit QgsAuthCertEditors( QWidget *parent = nullptr );

    ~QgsAuthCertEditors();
};


//////////////// Embed in dialog ///////////////////

/** \ingroup gui
 * Dialog wrapper for widget to manage available certificate editors
 */
class GUI_EXPORT QgsAuthCertManager : public QDialog
{
    Q_OBJECT

  public:
    /**
     * Construct a dialog wrapper for widget to manage available certificate editors
     * @param parent Parent widget
     */
    explicit QgsAuthCertManager( QWidget *parent = nullptr );

    ~QgsAuthCertManager();

    /** Get access to embedded editors widget */
    QgsAuthCertEditors *certEditorsWidget() { return mCertEditors; }

  private:
    QgsAuthCertEditors *mCertEditors;
};

#endif // QGSAUTHCERTIFICATEMANAGER_H
