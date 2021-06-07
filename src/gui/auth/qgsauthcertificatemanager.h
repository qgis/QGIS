/***************************************************************************
    qgsauthcertificatemanager.h
    ---------------------
    begin                : September, 2015
    copyright            : (C) 2015 by Boundless Spatial, Inc. USA
    author               : Larry Shaffer
    email                : lshaffer at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAUTHCERTIFICATEMANAGER_H
#define QGSAUTHCERTIFICATEMANAGER_H

#include "ui_qgsauthcertificatemanager.h"
#include "qgis_sip.h"

#include <QWidget>
#include <QDialog>
#include "qgis_gui.h"


/**
 * \ingroup gui
 * Wrapper widget to manage available certificate editors
 */
class GUI_EXPORT QgsAuthCertEditors : public QWidget, private Ui::QgsAuthCertManager
{
    Q_OBJECT

  public:

    /**
     * Construct a widget to contain various certificate editors
     * \param parent Parent widget
     */
    explicit QgsAuthCertEditors( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Destructor: store last selected tab
     */
    ~QgsAuthCertEditors( ) override;

};


//////////////// Embed in dialog ///////////////////

/**
 * \ingroup gui
 * Dialog wrapper for widget to manage available certificate editors
 */
class GUI_EXPORT QgsAuthCertManager : public QDialog
{
    Q_OBJECT

  public:

    /**
     * Construct a dialog wrapper for widget to manage available certificate editors
     * \param parent Parent widget
     */
    explicit QgsAuthCertManager( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    //! Gets access to embedded editors widget
    QgsAuthCertEditors *certEditorsWidget() { return mCertEditors; }

  private:
    QgsAuthCertEditors *mCertEditors = nullptr;
};

#endif // QGSAUTHCERTIFICATEMANAGER_H
