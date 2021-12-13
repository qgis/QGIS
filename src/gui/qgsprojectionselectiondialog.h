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
#include "ui_qgsgenericprojectionselectorbase.h"
#include "qgis_sip.h"
#include "qgsguiutils.h"

#include <QSet>
#include <QDialog>

#include "qgis_gui.h"
#include "qgscoordinatereferencesystem.h"

class QDialogButtonBox;

/**
 * \class QgsCrsSelectionWidget
 * \ingroup gui
 * \brief A generic widget allowing users to pick a Coordinate Reference System (or define their own).
 *
 * \since QGIS 3.24
 */
class GUI_EXPORT QgsCrsSelectionWidget : public QgsPanelWidget, private Ui::QgsGenericProjectionSelectorBase
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsCrsSelectionWidget, with the specified \a parent widget.
     */
    QgsCrsSelectionWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    ~QgsCrsSelectionWidget() override;

    /**
     * Returns the CRS currently selected in the widget.
     * \see setCrs()
     */
    QgsCoordinateReferenceSystem crs() const;

    /**
     * Sets a \a message to show in the dialog.
     */
    void setMessage( const QString &message );

    /**
     * Sets whether a "no/invalid" CRS option should be shown. If this
     * option is selected, calling crs() will return an invalid QgsCoordinateReferenceSystem.
     * \see showNoCrs()
     */
    void setShowNoCrs( bool show );

    /**
     * Returns whether the "no/invalid" CRS option is shown. If this
     * option is selected, calling crs() will return an invalid QgsCoordinateReferenceSystem.
     * \see setShowNoCrs()
     */
    bool showNoCrs() const;

    /**
     * Sets the text to show for the not set option. Note that this option is not shown
     * by default and must be set visible by calling setShowNoCrs().
     *
     * The \a description argument can be used to specify a detailed description which
     * is shown when the option is selected.
     */
    void setNotSetText( const QString &text, const QString &description = QString() );

    /**
     * Returns TRUE if the widget has a valid CRS defined.
     */
    bool hasValidSelection() const;

  public slots:

    /**
     * Sets the \a crs to show within the widget.
     * \see crs()
     */
    void setCrs( const QgsCoordinateReferenceSystem &crs );

    /**
     * \brief filters this dialog by the given CRSs
     *
     * Sets this dialog to filter the available CRSs to those listed
     * by the given Coordinate Reference Systems.
     *
     * \param crsFilter a list of OGC Coordinate Reference Systems to filter the
     *                  list of CRS by. This is useful in (e.g.) WMS situations
     *                  where you just want to offer what the WMS server can support.
     *
     * \warning This function's behavior is undefined if it is called after the dialog is shown.
     */
    void setOgcWmsCrsFilter( const QSet<QString> &crsFilter );

  signals:

    /**
     * Emitted when the CRS defined in the widget is changed.
     */
    void crsChanged();

    /**
     * Emitted when a CRS entry in the widget is double-clicked.
     */
    void crsDoubleClicked( const QgsCoordinateReferenceSystem &crs );

    /**
     * Emitted when the widget has a valid selection or not.
     */
    void hasValidSelectionChanged( bool isValid );

  private:

    /**
     * CRS types
     */
    enum class CrsType
    {
      Predefined, //!< Predefined (from database )
      Custom, //!< Custom CRS
    };

    QString mNotSetText;
    bool mShowNoCrsOption = false;
    bool mDeferredInvalidCrsSet = false;
    int mBlockSignals = 0;

};



/**
 * \class QgsProjectionSelectionDialog
 * \ingroup gui
 * \brief A generic dialog to prompt the user for a Coordinate Reference System.
 *
 * Typically you will use this when you want to prompt the user for
 * a coordinate system identifier e.g. from a plugin you might do this
 * to get an epsg code:
 *
 * \code{.py}
 *     crs = QgsCoordinateReferenceSystem()
 *     mySelector = QgsProjectionSelectionDialog( iface.mainWindow() )
 *     mySelector.setCrs( crs )
 *     if mySelector.exec():
 *       mCrs = mySelector.crs()
 * \endcode
 *
 * If you wish to embed the projection selector into an existing dialog
 * the you probably want to look at QgsProjectionSelectionWidget instead.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsProjectionSelectionDialog : public QDialog
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsProjectionSelectionDialog.
     */
    QgsProjectionSelectionDialog( QWidget *parent SIP_TRANSFERTHIS = nullptr,
                                  Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );

    /**
     * Returns the CRS currently selected in the widget.
     * \see setCrs()
     * \since QGIS 3.0
     */
    QgsCoordinateReferenceSystem crs() const;

    /**
     * Sets a \a message to show in the dialog.
     *
     * \see showNoCrsForLayerMessage()
     */
    void setMessage( const QString &message );

    /**
     * When called, the dialog will show a default "layer has no CRS set" message above the projection selector.
     *
     * \see setMessage()
     * \since QGIS 3.16
     */
    void showNoCrsForLayerMessage();

    /**
     * Sets whether a "no/invalid" projection option should be shown. If this
     * option is selected, calling crs() will return an invalid QgsCoordinateReferenceSystem.
     * \see showNoProjection()
     * \since QGIS 3.0
     */
    void setShowNoProjection( bool show );

    /**
     * Returns whether the "no/invalid" projection option is shown. If this
     * option is selected, calling crs() will return an invalid QgsCoordinateReferenceSystem.
     * \see setShowNoProjection()
     * \since QGIS 3.0
     */
    bool showNoProjection() const;

    /**
     * Sets the text to show for the not set option. Note that this option is not shown
     * by default and must be set visible by calling setShowNoProjection().
     *
     * Since QGIS 3.24, the \a description argument can be used to specify a detailed description which
     * is shown when the option is selected.
     *
     * \since QGIS 3.16
     */
    void setNotSetText( const QString &text, const QString &description = QString() );

    /**
     * Sets the dialog to require a valid selection only, preventing users from accepting the
     * dialog if no selection is present.
     *
     * \since QGIS 3.18
     */
    void setRequireValidSelection();

    /**
     * Returns TRUE if the dialog has a valid CRS defined.
     *
     * \since QGIS 3.24
     */
    bool hasValidSelection() const;

  public slots:

    /**
     * Sets the initial \a crs to show within the dialog.
     * \see crs()
     * \since QGIS 3.0
     */
    void setCrs( const QgsCoordinateReferenceSystem &crs );

    /**
     * \brief filters this dialog by the given CRSs
     *
     * Sets this dialog to filter the available projections to those listed
     * by the given Coordinate Reference Systems.
     *
     * \param crsFilter a list of OGC Coordinate Reference Systems to filter the
     *                  list of projections by. This is useful in (e.g.) WMS situations
     *                  where you just want to offer what the WMS server can support.
     *
     * \warning This function's behavior is undefined if it is called after the dialog is shown.
     */
    void setOgcWmsCrsFilter( const QSet<QString> &crsFilter );

  private slots:

    void showHelp();

  private:

    QDialogButtonBox *mButtonBox = nullptr;
    QgsCrsSelectionWidget *mCrsWidget = nullptr;

    bool mRequireValidSelection = false;
};

#endif // #ifndef QGSLAYERCRSSELECTOR_H
