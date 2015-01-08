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
    explicit QgsProjectionSelectionWidget( QWidget *parent = 0 );

    /* Returns a pointer to the projection selector dialog used by the widget
     * @returns projection selector dialog
     */
    QgsGenericProjectionSelector* dialog() { return mDialog; }

    /* Returns a pointer to the line edit used by the widget
     * @returns CRS line edit
     */
    QLineEdit* lineEdit() { return mCrsLineEdit; }

    /* Returns the currently selected CRS for the widget
     * @returns current CRS
     */
    QgsCoordinateReferenceSystem crs() const { return mCrs; }

  signals:

    /* Emitted when the selected CRS is changed
     */
    void crsChanged( QgsCoordinateReferenceSystem );

  public slots:

    /* Sets the current CRS for the widget
     * @param crs new CRS
     */
    void setCrs( const QgsCoordinateReferenceSystem& crs );

    /* Opens the dialog for selecting a new CRS
     */
    void selectCrs();

  private:
    QgsCoordinateReferenceSystem mCrs;
    QLineEdit* mCrsLineEdit;
    QToolButton* mButton;
    QgsGenericProjectionSelector* mDialog;
};

#endif // QGSPROJECTIONSELECTIONWIDGET_H
