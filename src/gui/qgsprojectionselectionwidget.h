/***************************************************************************
    qgsprojectionselectionwidget.h
     --------------------------------------
    Date                 : 05.01.2015
    Copyright            : (C) 2015 Denis Rouzaud
    Email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSPROJECTIONSELECTIONWIDGET_H
#define QGSPROJECTIONSELECTIONWIDGET_H

#include <QWidget>
#include "qgis_sip.h"
#include <QLineEdit>
#include <QToolButton>
#include <QComboBox>

#include "qgscoordinatereferencesystem.h"
#include "qgis_gui.h"

class QgsProjectionSelectionDialog;

/**
 * \class QgsProjectionSelectionWidget
 * \ingroup gui
 * \brief A widget for selecting a projection.
 * \since QGIS 2.7
 */
class GUI_EXPORT QgsProjectionSelectionWidget : public QWidget
{
    Q_OBJECT
  public:

    /**
     * Predefined CRS options shown in widget
     */
    enum CrsOption
    {
      LayerCrs, //!< Optional layer CRS
      ProjectCrs, //!< Current project CRS (if OTF reprojection enabled)
      CurrentCrs, //!< Current user selected CRS
      DefaultCrs, //!< Global default QGIS CRS
      RecentCrs, //!< Recently used CRS
      CrsNotSet, //!< Not set (hidden by default)
    };

    //! Constructor for QgsProjectionSelectionWidget
    explicit QgsProjectionSelectionWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the currently selected CRS for the widget
     * \returns current CRS
     */
    QgsCoordinateReferenceSystem crs() const;

    /**
     * Sets whether a predefined CRS option should be shown in the widget.
     * \param option CRS option to show/hide
     * \param visible whether the option should be shown
     * \see optionVisible()
     */
    void setOptionVisible( CrsOption option, bool visible );

    /**
     * Returns whether the specified CRS option is visible in the widget.
     * \see setOptionVisible()
     * \since QGIS 3.0
     */
    bool optionVisible( CrsOption option ) const;

    /**
     * Sets the text to show for the not set option. Note that this option is not shown
     * by default and must be set visible by calling setOptionVisible().
     * \since QGIS 3.0
     */
    void setNotSetText( const QString &text );

    /**
     * Sets a \a message to show in the dialog. If an empty string is
     * passed, the message will be a generic
     * 'define the CRS for this layer'.
     * \since QGIS 3.0
     */
    void setMessage( const QString &text );

  signals:

    /**
     * Emitted when the selected CRS is changed
     */
    void crsChanged( const QgsCoordinateReferenceSystem & );

    /**
     * Emitted when the not set option is selected.
     * \since QGIS 3.0
     */
    void cleared();

  public slots:

    /**
     * Sets the current CRS for the widget
     * \param crs new CRS
     */
    void setCrs( const QgsCoordinateReferenceSystem &crs );

    /**
     * Sets the layer CRS for the widget. If set, this will be added as an option
     * to the preset CRSes shown in the widget.
     * \param crs layer CRS
     */
    void setLayerCrs( const QgsCoordinateReferenceSystem &crs );

    /**
     * Opens the dialog for selecting a new CRS
     */
    void selectCrs();

  private:

    QgsCoordinateReferenceSystem mCrs;
    QgsCoordinateReferenceSystem mLayerCrs;
    QgsCoordinateReferenceSystem mProjectCrs;
    QgsCoordinateReferenceSystem mDefaultCrs;
    QComboBox *mCrsComboBox = nullptr;
    QToolButton *mButton = nullptr;
    QgsProjectionSelectionDialog *mDialog = nullptr;
    QString mNotSetText;
    QString mMessage;

    void addNotSetOption();
    void addProjectCrsOption();
    void addDefaultCrsOption();
    void addCurrentCrsOption();
    QString currentCrsOptionText( const QgsCoordinateReferenceSystem &crs ) const;
    void addRecentCrs();
    bool crsIsShown( long srsid ) const;

    int firstRecentCrsIndex() const;

  private slots:

    void comboIndexChanged( int idx );

};

#endif // QGSPROJECTIONSELECTIONWIDGET_H
