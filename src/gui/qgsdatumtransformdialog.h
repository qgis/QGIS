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

    /**
     * Constructor for QgsDatumTransformDialog.
     */
    QgsDatumTransformDialog( QgsCoordinateReferenceSystem sourceCrs = QgsCoordinateReferenceSystem(),
                             QgsCoordinateReferenceSystem destinationCrs = QgsCoordinateReferenceSystem(),
                             QWidget *parent = nullptr, Qt::WindowFlags f = nullptr );
    ~QgsDatumTransformDialog();

    /**
     * Returns the number of possible datum transformation for currently selected source and destination CRS
     * \since 3.0
     */
    int availableTransformationCount();

    /**
     * Returns the source and destination transforms, each being a pair of QgsCoordinateReferenceSystems and datum transform code
     * \since 3.0
     */
    QPair< QPair<QgsCoordinateReferenceSystem, int>, QPair<QgsCoordinateReferenceSystem, int > > selectedDatumTransforms();

  private slots:
    void mHideDeprecatedCheckBox_stateChanged( int state );
    void mDatumTransformTreeWidget_currentItemChanged( QTreeWidgetItem *, QTreeWidgetItem * );
    void setSourceCrs();
    void setDestinationCrs();

  private:
    void updateTitle();
    bool gridShiftTransformation( const QString &itemText ) const;
    //! Returns false if the location of the grid shift files is known (PROJ_LIB) and the shift file is not there
    bool testGridShiftFileAvailability( QTreeWidgetItem *item, int col ) const;
    void load();

    QList< QList< int > > mDatumTransforms;
    QgsCoordinateReferenceSystem mSourceCrs;
    QgsCoordinateReferenceSystem mDestinationCrs;
};

#endif // QGSDATUMTRANSFORMDIALOG_H
