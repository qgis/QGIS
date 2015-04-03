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
#include <QLineEdit>
#include <QToolButton>
#include <QComboBox>

#include "qgscoordinatereferencesystem.h"

class QgsGenericProjectionSelector;

/**
 * \class QgsProjectionSelectionWidget
 * \ingroup gui
 * \brief A widget for selecting a projection.
 * \note added in QGIS 2.7
 */
class GUI_EXPORT QgsProjectionSelectionWidget : public QWidget
{
    Q_OBJECT
  public:

    /** Predefined CRS options shown in widget
     */
    enum CrsOption
    {
      LayerCrs, /*< optional layer CRS */
      ProjectCrs, /*< current project CRS (if OTF reprojection enabled) */
      CurrentCrs, /*< current user selected CRS */
      DefaultCrs, /*< global default QGIS CRS */
      RecentCrs /*< recently used CRS */
    };

    explicit QgsProjectionSelectionWidget( QWidget *parent = 0 );

    /* Returns a pointer to the projection selector dialog used by the widget.
     * Can be used to modify how the projection selector dialog behaves.
     * @returns projection selector dialog
     */
    QgsGenericProjectionSelector* dialog() { return mDialog; }

    /* Returns the currently selected CRS for the widget
     * @returns current CRS
     */
    QgsCoordinateReferenceSystem crs() const;

    /* Sets whether a predefined CRS option should be shown in the widget.
     * @param option CRS option to show/hide
     * @param visible whether the option should be shown
     */
    void setOptionVisible( const CrsOption option, const bool visible );

  signals:

    /* Emitted when the selected CRS is changed
     */
    void crsChanged( QgsCoordinateReferenceSystem );

  public slots:

    /* Sets the current CRS for the widget
     * @param crs new CRS
     */
    void setCrs( const QgsCoordinateReferenceSystem& crs );

    /* Sets the layer CRS for the widget. If set, this will be added as an option
     * to the preset CRSes shown in the widget.
     * @param crs layer CRS
     */
    void setLayerCrs( const QgsCoordinateReferenceSystem& crs );

    /* Opens the dialog for selecting a new CRS
     */
    void selectCrs();

  private:

    QgsCoordinateReferenceSystem mCrs;
    QgsCoordinateReferenceSystem mLayerCrs;
    QgsCoordinateReferenceSystem mProjectCrs;
    QgsCoordinateReferenceSystem mDefaultCrs;
    QComboBox* mCrsComboBox;
    QToolButton* mButton;
    QgsGenericProjectionSelector* mDialog;

    void addProjectCrsOption();
    void addDefaultCrsOption();
    void addRecentCrs();
    bool crsIsShown( const long srsid ) const;

    int firstRecentCrsIndex() const;

  private slots:

    void comboIndexChanged( int idx );

};

#endif // QGSPROJECTIONSELECTIONWIDGET_H
