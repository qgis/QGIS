/***************************************************************************
                          qgsgenericprojectionselector.h
                Set user defined projection using projection selector widget
                             -------------------
    begin                : May 28, 2004
    copyright            : (C) 2004 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSGENERICPROJECTIONSELECTOR_H
#define QGSGENERICPROJECTIONSELECTOR_H
#include <ui_qgsgenericprojectionselectorbase.h>
#include <qgisgui.h>

#include <QSet>

#include "qgscontexthelp.h"
#include "qgis_gui.h"
#include "qgscoordinatereferencesystem.h"

/**
 * \class QgsProjectionSelectionDialog
 * \ingroup gui
 * A generic dialog to prompt the user for a Coordinate Reference System.
 *
 * Typically you will use this when you want to prompt the user for
 * a coordinate system identifier e.g. from a plugin you might do this
 * to get an epsg code:
 * \code
 * QgsProjectionSelectionDialog mySelector( mQGisIface->mainWindow() );
 * mySelector.setCrs( crs );
 * if ( mySelector.exec() )
 * {
 *   mCrs = mySelector.crs();
 * }
 * \endcode
 *
 * If you wish to embed the projection selector into an existing dialog
 * the you probably want to look at QgsProjectionSelectionWidget instead.
 * @note added in QGIS 3.0
 */

class GUI_EXPORT QgsProjectionSelectionDialog : public QDialog, private Ui::QgsGenericProjectionSelectorBase
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsProjectionSelectionDialog.
     */
    QgsProjectionSelectionDialog( QWidget *parent = nullptr,
                                  Qt::WindowFlags fl = QgisGui::ModalDialogFlags );


    ~QgsProjectionSelectionDialog();

    /**
     * Returns the CRS currently selected in the widget.
     * @note added in QGIS 3.0
     * @see setCrs()
     */
    QgsCoordinateReferenceSystem crs() const;

    /**
     * Sets a \a message to show in the dialog. If an empty string is
     * passed, the message will be a generic
     * 'define the CRS for this layer'.
     */
    void setMessage( const QString& message );

    /**
     * Sets whether a "no/invalid" projection option should be shown. If this
     * option is selected, calling crs() will return an invalid QgsCoordinateReferenceSystem.
     * @see showNoProjection()
     * @note added in QGIS 3.0
     */
    void setShowNoProjection( bool show );

    /**
     * Returns whether the "no/invalid" projection option is shown. If this
     * option is selected, calling crs() will return an invalid QgsCoordinateReferenceSystem.
     * @note added in QGIS 3.0
     * @see setShowNoProjection()
     */
    bool showNoProjection() const;

  public slots:

    /**
     * Sets the initial \a crs to show within the dialog.
     * @note added in QGIS 3.0
     * @see crs()
     */
    void setCrs( const QgsCoordinateReferenceSystem& crs );

    /**
     * \brief filters this dialog by the given CRSs
     *
     * Sets this dialog to filter the available projections to those listed
     * by the given Coordinate Reference Systems.
     *
     * \param crsFilter a list of OGC Coordinate Reference Systems to filter the
     *                  list of projections by.  This is useful in (e.g.) WMS situations
     *                  where you just want to offer what the WMS server can support.
     *
     * \warning This function's behavior is undefined if it is called after the dialog is shown.
     */
    void setOgcWmsCrsFilter( const QSet<QString>& crsFilter );

  private slots:

    void on_mButtonBox_helpRequested() { QgsContextHelp::run( metaObject()->className() ); }
};

#endif // #ifndef QGSLAYERCRSSELECTOR_H
