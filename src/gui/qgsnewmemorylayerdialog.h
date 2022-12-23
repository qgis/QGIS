/***************************************************************************
                         qgsnewmemorylayerdialog.h
                             -------------------
    begin                : September 2014
    copyright            : (C) 2014 by Nyall Dawson, Marco Hugentobler
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSNEWMEMORYLAYERDIALOG_H
#define QGSNEWMEMORYLAYERDIALOG_H

#include "ui_qgsnewmemorylayerdialogbase.h"
#include "qgsguiutils.h"
#include "qgswkbtypes.h"
#include "qgshelp.h"
#include "qgis_gui.h"

class QgsFields;
class QgsVectorLayer;

/**
 * \ingroup gui
 * \class QgsNewMemoryLayerDialog
 */
class GUI_EXPORT QgsNewMemoryLayerDialog: public QDialog, private Ui::QgsNewMemoryLayerDialogBase
{
    Q_OBJECT

  public:

    /**
     * Runs the dialog and creates a new memory layer
     * \param parent parent widget
     * \param defaultCrs default layer CRS to show in dialog
     * \returns new memory layer
     */
    static QgsVectorLayer *runAndCreateLayer( QWidget *parent = nullptr, const QgsCoordinateReferenceSystem &defaultCrs = QgsCoordinateReferenceSystem() );

    /**
     * New dialog constructor.
     */
    QgsNewMemoryLayerDialog( QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );

    //! Returns the selected geometry type
    QgsWkbTypes::Type selectedType() const;

    /**
     * Sets the \a crs value for the new layer in the dialog.
     * \see crs()
     * \since QGIS 3.0
     */
    void setCrs( const QgsCoordinateReferenceSystem &crs );

    /**
     * Returns the selected CRS for the new layer.
     * \see setCrs()
     */
    QgsCoordinateReferenceSystem crs() const;

    //! Returns the layer name
    QString layerName() const;

    /**
     * Returns attributes for the new layer.
     * \since QGIS 3.14
     */
    QgsFields fields() const;

    void accept() override;

  private:

    QString mCrsId;
    QPushButton *mOkButton = nullptr;

  private slots:

    void geometryTypeChanged( int index );
    void fieldNameChanged( const QString & );
    void mTypeBox_currentIndexChanged( int index );
    void mAddAttributeButton_clicked();
    void mRemoveAttributeButton_clicked();
    void selectionChanged();
    void showHelp();
};

#endif //QGSNEWMEMORYLAYERDIALOG_H
