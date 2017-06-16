/***************************************************************************
  qgsdatadefinedsizelegenddialog.h
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

#ifndef QGSDATADEFINEDSIZELEGENDDIALOG_H
#define QGSDATADEFINEDSIZELEGENDDIALOG_H

#include "qgis.h"
#include "qgis_gui.h"

#include <memory>
#include <QDialog>
#include <ui_qgsdatadefinedsizelegenddialog.h>

class QgsDataDefinedSizeLegend;
class QgsLayerTree;
class QgsLayerTreeLayer;
class QgsLayerTreeModel;
class QgsMarkerSymbol;
class QgsVectorLayer;

/** \ingroup gui
 * Dialog for configuration of appearance of legend for marker symbols with data-defined size.
 *
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsDataDefinedSizeLegendDialog : public QDialog, private Ui::QgsDataDefinedSizeLegendDialog
{
    Q_OBJECT
  public:
    //! Creates the dialog and initializes the content to what is passed in the legend configuration (may be null)
    explicit QgsDataDefinedSizeLegendDialog( const QgsDataDefinedSizeLegend *ddsLegend, QWidget *parent SIP_TRANSFERTHIS = nullptr );
    ~QgsDataDefinedSizeLegendDialog();

    //! Use given symbol for preview. Takes ownership of the symbol. It should have data-defined size enabled + size scale transformer attached.
    void setSourceSymbol( QgsMarkerSymbol *symbol SIP_TRANSFER );

    //! Setup map view details to make preview match the expected output
    void setLegendMapViewData( double mapUnitsPerPixel, int dpi, double scale );

    //! Returns configuration as set up in the dialog (may be null). Ownership is passed to the caller.
    QgsDataDefinedSizeLegend *dataDefinedSizeLegend() const SIP_FACTORY;

  signals:

  private slots:
    void updatePreview();

  private:
    std::unique_ptr<QgsMarkerSymbol> mSourceSymbol;
    QgsLayerTreeModel *mPreviewModel;
    QgsLayerTree *mPreviewTree;
    QgsLayerTreeLayer *mPreviewLayerNode;
    QgsVectorLayer *mPreviewLayer;
};

#endif // QGSDATADEFINEDSIZELEGENDDIALOG_H
