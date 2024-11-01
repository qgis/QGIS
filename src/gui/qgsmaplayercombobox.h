/***************************************************************************
   qgsmaplayercombobox.h
    --------------------------------------
   Date                 : 01.04.2014
   Copyright            : (C) 2014 Denis Rouzaud
   Email                : denis.rouzaud@gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef QGSMAPLAYERCOMBOBOX_H
#define QGSMAPLAYERCOMBOBOX_H

#include <QComboBox>

#include "qgsmaplayerproxymodel.h"
#include "qgis_gui.h"

#include "qgis_sip.h"

class QgsMapLayer;
class QgsVectorLayer;

/**
 * \ingroup gui
 * \brief The QgsMapLayerComboBox class is a combo box which displays the list of layers
 */
class GUI_EXPORT QgsMapLayerComboBox : public QComboBox
{
    Q_OBJECT
    Q_PROPERTY( Qgis::LayerFilters filters READ filters WRITE setFilters )
    Q_PROPERTY( bool allowEmptyLayer READ allowEmptyLayer WRITE setAllowEmptyLayer )
    Q_PROPERTY( bool showCrs READ showCrs WRITE setShowCrs )
    Q_PROPERTY( QStringList excludedProviders READ excludedProviders WRITE setExcludedProviders )

  public:
    /**
     * \brief QgsMapLayerComboBox creates a combo box to display the list of layers (currently in the registry).
     * The layers can be filtered and/or ordered.
     */
    explicit QgsMapLayerComboBox( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets \a filters for the layers displayed in the combo box.
     *
     * This method allows filtering layers according to layer type and/or geometry type.
     *
     * \see filters()
     */
    void setFilters( Qgis::LayerFilters filters ) { mProxyModel->setFilters( filters ); }

    /**
     * Filters according to layer type and/or geometry type.
     * \note for API compatibility
     * \since QGIS 3.34
     * \deprecated QGIS 3.34. Use the flag signature instead.
     */
    Q_DECL_DEPRECATED void setFilters( int filters ) SIP_DEPRECATED { setFilters( static_cast<Qgis::LayerFilters>( filters ) ); }

    /**
     * Returns any currently used filters on the listed layers.
     *
     * \see setFilters()
     */
    Qgis::LayerFilters filters() const { return mProxyModel->filters(); }

    /**
     * Sets a list of layers which should be excluded from the combo box.
     *
     * \see exceptedLayerList()
     */
    void setExceptedLayerList( const QList<QgsMapLayer *> &layerList ) { mProxyModel->setExceptedLayerList( layerList ); }

    /**
     * Returns a list of layers which should be excluded from the combo box.
     *
     * \see setExceptedLayerList()
     */
    QList<QgsMapLayer *> exceptedLayerList() const { return mProxyModel->exceptedLayerList(); }

    /**
     * Sets a list of data providers which should be excluded from the combobox.
     * \see excludedProviders()
     */
    void setExcludedProviders( const QStringList &providers );

    /**
     * Returns the list of data providers which are excluded from the combobox.
     * \see setExcludedProviders()
     */
    QStringList excludedProviders() const;

    /**
     * Sets the \a project from which map layers are shown.
     *
     * If \a project is NULLPTR then QgsProject::instance() will be used.
     *
     * \since QGIS 3.24
     */
    void setProject( QgsProject *project );


    /**
     * Sets whether an optional empty layer ("not set") option is shown in the combo box.
     *
     * Since QGIS 3.20, the optional \a text and \a icon arguments allows the text and icon for the empty layer item to be set.
     *
     * \see allowEmptyLayer()
     */
    void setAllowEmptyLayer( bool allowEmpty, const QString &text = QString(), const QIcon &icon = QIcon() );

    /**
     * Returns TRUE if the combo box allows the empty layer ("not set") choice.
     * \see setAllowEmptyLayer()
     */
    bool allowEmptyLayer() const;

    /**
     * Sets whether the CRS of layers is also included in the combo box text.
     * \see showCrs()
     */
    void setShowCrs( bool showCrs );

    /**
     * Returns TRUE if the combo box shows the layer's CRS.
     * \see setShowCrs()
     */
    bool showCrs() const;

    /**
     * Sets a list of additional (non map layer) items to include at the end of the combobox.
     * These may represent additional layers such as layers which are not included in the map
     * layer registry, or paths to layers which have not yet been loaded into QGIS.
     * \see additionalItems()
     */
    void setAdditionalItems( const QStringList &items );

    /**
     * Returns the list of additional (non map layer) items included at the end of the combo box.
     * \see setAdditionalItems()
     */
    QStringList additionalItems() const;

    /**
     * Sets a list of additional \a layers to include in the combobox.
     *
     * This method allows adding additional layers, which are not part of a project's
     * layers, into the combobox.
     *
     * \see additionalLayers()
     * \since QGIS 3.22
     */
    void setAdditionalLayers( const QList<QgsMapLayer *> &layers );

    /**
     * Returns the list of additional layers added to the combobox.
     *
     * \see setAdditionalLayers()
     * \since QGIS 3.22
     */
    QList<QgsMapLayer *> additionalLayers() const;

    /**
     * Returns the current layer selected in the combo box.
     * \see layer
     */
    QgsMapLayer *currentLayer() const;

    /**
     * Returns the layer currently shown at the specified index within the combo box.
     * \param layerIndex position of layer to return
     * \see currentLayer
     */
    QgsMapLayer *layer( int layerIndex ) const;

  public slots:

    /**
     * Sets the current \a layer selected in the combo box.
     */
    void setLayer( QgsMapLayer *layer );

  signals:
    //! Emitted whenever the currently selected layer changes.
    void layerChanged( QgsMapLayer *layer );

  protected:
    void dragEnterEvent( QDragEnterEvent *event ) override;
    void dragLeaveEvent( QDragLeaveEvent *event ) override;
    void dropEvent( QDropEvent *event ) override;
    void paintEvent( QPaintEvent *e ) override;

  protected slots:
    void indexChanged( int i );
    void rowsChanged();

  private:
    QgsMapLayerProxyModel *mProxyModel = nullptr;
    bool mDragActive = false;
    bool mHighlight = false;

    /**
     * Returns a map layer, compatible with the filters set for the combo box, from
     * the specified mime \a data (if possible!).
     */
    QgsMapLayer *compatibleMapLayerFromMimeData( const QMimeData *data ) const;

    friend class QgsProcessingMapLayerComboBox;
};

#endif // QGSMAPLAYERCOMBOBOX_H
