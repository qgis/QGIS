/***************************************************************************
  qgsdatadefinedsizelegendwidget.h
  --------------------------------------
  Date                 : June 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDATADEFINEDSIZELEGENDWIDGET_H
#define QGSDATADEFINEDSIZELEGENDWIDGET_H

#include "qgis.h"
#include "qgis_gui.h"

#include <memory>
#include "ui_qgsdatadefinedsizelegendwidget.h"

#include "qgspanelwidget.h"
#include "qgsproperty.h"

class QStandardItemModel;

class QgsDataDefinedSizeLegend;
class QgsLayerTree;
class QgsLayerTreeLayer;
class QgsLayerTreeModel;
class QgsMapCanvas;
class QgsMarkerSymbol;
class QgsProperty;
class QgsVectorLayer;

/** \ingroup gui
 * Widget for configuration of appearance of legend for marker symbols with data-defined size.
 *
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsDataDefinedSizeLegendWidget : public QgsPanelWidget, private Ui::QgsDataDefinedSizeLegendWidget
{
    Q_OBJECT
  public:
    //! Creates the dialog and initializes the content to what is passed in the legend configuration (may be null).
    //! The ddSize argument determines scaling of the marker symbol - it should have a size scale transformer assigned
    //! to know the range of sizes. The overrideSymbol argument may override the source symbol: this is useful in case
    //! when the symbol is given from outside rather than being set inside QgsDataDefinedSizeLegend.
    explicit QgsDataDefinedSizeLegendWidget( const QgsDataDefinedSizeLegend *ddsLegend, const QgsProperty &ddSize, QgsMarkerSymbol *overrideSymbol SIP_TRANSFER, QgsMapCanvas *canvas = nullptr, QWidget *parent SIP_TRANSFERTHIS = nullptr );
    ~QgsDataDefinedSizeLegendWidget();

    //! Returns configuration as set up in the dialog (may be null). Ownership is passed to the caller.
    QgsDataDefinedSizeLegend *dataDefinedSizeLegend() const SIP_FACTORY;

  signals:

  private slots:
    void updatePreview();
    void changeSymbol();
    void addSizeClass();
    void removeSizeClass();
    void onSizeClassesChanged();

  private:
    std::unique_ptr<QgsMarkerSymbol> mSourceSymbol;   //!< Source symbol (without data-defined size set)
    bool mOverrideSymbol = false;  //!< If true, symbol should not be editable because it will be overridden
    QgsProperty mSizeProperty;    //!< Definition of data-defined size of symbol (should have a size scale transformer associated)
    QgsLayerTreeModel *mPreviewModel;
    QgsLayerTree *mPreviewTree;
    QgsLayerTreeLayer *mPreviewLayerNode;
    QgsVectorLayer *mPreviewLayer;
    QgsMapCanvas *mMapCanvas = nullptr;
    QStandardItemModel *mSizeClassesModel;
};

#endif // QGSDATADEFINEDSIZELEGENDWIDGET_H
