/***************************************************************************
    qgsalignrasterdialog.h
    ---------------------
    begin                : June 2015
    copyright            : (C) 2015 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSALIGNRASTERDIALOG_H
#define QGSALIGNRASTERDIALOG_H

#include <QDialog>
#include "qgsalignraster.h"
#include "ui_qgsalignrasterdialog.h"

class QgsAlignRaster;

/** Dialog providing user interface for QgsAlignRaster */
class QgsAlignRasterDialog : public QDialog, private Ui::QgsAlignRasterDialog
{
    Q_OBJECT
  public:
    explicit QgsAlignRasterDialog( QWidget *parent = nullptr );
    ~QgsAlignRasterDialog();

  signals:

  protected slots:
    void addLayer();
    void removeLayer();
    void editLayer();

    void referenceLayerChanged();

    void runAlign();

    void destinationCrsChanged();

    void clipExtentChanged();

    void updateCustomCRS();
    void updateCustomCellSize();
    void updateCustomGridOffset();

    void updateParametersFromReferenceLayer();

  protected:
    void populateLayersView();
    void updateAlignedRasterInfo();

  protected:
    QgsAlignRaster* mAlign;
};


class QgsMapLayerComboBox;
class QCheckBox;

/** Simple dialog to display details of one layer's configuration */
class QgsAlignRasterLayerConfigDialog : public QDialog
{
    Q_OBJECT
  public:
    QgsAlignRasterLayerConfigDialog();

    QString inputFilename() const;
    QString outputFilename() const;
    QgsAlignRaster::ResampleAlg resampleMethod() const;
    bool rescaleValues() const;

    void setItem( const QString& inputFilename, const QString& outputFilename, QgsAlignRaster::ResampleAlg resampleMethod, bool rescaleValues );

  protected slots:
    void browseOutputFilename();

  protected:
    QgsMapLayerComboBox* cboLayers;
    QLineEdit* editOutput;
    QPushButton* btnBrowse;
    QComboBox* cboResample;
    QCheckBox* chkRescale;
    QDialogButtonBox* btnBox;
};


#endif // QGSALIGNRASTERDIALOG_H
