/***************************************************************************
    qgslayoutlegendlayersdialog.h
    -----------------------------
    begin                : October 2017
    copyright            : (C) 2020 by Wang Peng
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLAYOUTLEGENDLAYERSDIALOG_H
#define QGSLAYOUTLEGENDLAYERSDIALOG_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include "qgis_gui.h"
#include "ui_qgslayoutlegendlayersdialogbase.h"

class QgsMapLayer;
class QgsMapLayerProxyModel;

/**
 * \ingroup gui
 * A dialog to add new layers to the legend.
 *
 * \note This class is not a part of public API
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsLayoutLegendLayersDialog: public QDialog, private Ui::QgsLayoutLegendLayersDialogBase
{
    Q_OBJECT

  public:
    //! constructor
    QgsLayoutLegendLayersDialog( QWidget *parent = nullptr );

    /**
     * Sets a list of visible \a layers, to use for filtering within the dialog.
     */
    void setVisibleLayers( const QList<QgsMapLayer *> &layers );

    //! Returns the list of selected layers
    QList< QgsMapLayer * > selectedLayers() const;

  private slots:

    void filterVisible( bool enabled );
    void showHelp();

  private:

    QgsMapLayerProxyModel *mModel = nullptr;
    QList< QgsMapLayer * > mVisibleLayers;
};

#endif //QGSLAYOUTLEGENDLAYERSDIALOG_H
