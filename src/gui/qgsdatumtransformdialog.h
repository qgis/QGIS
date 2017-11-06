/***************************************************************************
                         qgsdatumtransformdialog.h
                         -------------------------
    begin                : November 2013
    copyright            : (C) 2013 by Marco Hugentobler
    email                : marco.hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDATUMTRANSFORMDIALOG_H
#define QGSDATUMTRANSFORMDIALOG_H

#include "ui_qgsdatumtransformdialogbase.h"
#include "qgscoordinatereferencesystem.h"
#include "qgis_gui.h"

#define SIP_NO_FILE

/**
 * \ingroup gui
 * \class QgsDatumTransformDialog
 * \note not available in Python bindings
 */
class GUI_EXPORT QgsDatumTransformDialog : public QDialog, private Ui::QgsDatumTransformDialogBase
{
    Q_OBJECT
  public:
    QgsDatumTransformDialog( const QList< QList< int > > &dt, QWidget *parent = nullptr, Qt::WindowFlags f = nullptr );
    ~QgsDatumTransformDialog();

    /**
     * Sets the \a source and \a destination coordinate reference systems.
     * \since QGIS 3.0
     */
    void setCrs( const QgsCoordinateReferenceSystem &source, const QgsCoordinateReferenceSystem &destination );

    //! getter for selected datum transformations
    QList< int > selectedDatumTransform();

    //! dialog shall remember the selection
    bool rememberSelection() const;

  private slots:
    void mHideDeprecatedCheckBox_stateChanged( int state );
    void mDatumTransformTreeWidget_currentItemChanged( QTreeWidgetItem *, QTreeWidgetItem * );
    void accepted();

  private:
    QgsDatumTransformDialog();
    void updateTitle();
    bool gridShiftTransformation( const QString &itemText ) const;
    //! Returns false if the location of the grid shift files is known (PROJ_LIB) and the shift file is not there
    bool testGridShiftFileAvailability( QTreeWidgetItem *item, int col ) const;
    void load();

    QList< QList< int > > mDt;
    QgsCoordinateReferenceSystem mSrcCrs;
    QgsCoordinateReferenceSystem mDestCrs;
};

#endif // QGSDATUMTRANSFORMDIALOG_H
