/***************************************************************************
    qgsinvertedpolygonrendererwidget.h
    ---------------------
    begin                : April 2014
    copyright            : (C) 2014 Hugo Mercier / Oslandia
    email                : hugo dot mercier at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSINVERTEDPOLYGONRENDERERWIDGET_H
#define QGSINVERTEDPOLYGONRENDERERWIDGET_H

#include "ui_qgsinvertedpolygonrendererwidgetbase.h"
#include "qgis_sip.h"
#include "qgsinvertedpolygonrenderer.h"
#include "qgsrendererwidget.h"
#include "qgis_gui.h"

class QMenu;

/**
 * \ingroup gui
 * A widget used represent options of a QgsInvertedPolygonRenderer
 *
 * \since QGIS 2.4
 */
class GUI_EXPORT QgsInvertedPolygonRendererWidget : public QgsRendererWidget, private Ui::QgsInvertedPolygonRendererWidgetBase
{
    Q_OBJECT

  public:

    /**
     * Static creation method
     * \param layer the layer where this renderer is applied
     * \param style
     * \param renderer the mask renderer (will not take ownership)
     */
    static QgsRendererWidget *create( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer ) SIP_FACTORY;

    /**
     * Constructor
     * \param layer the layer where this renderer is applied
     * \param style
     * \param renderer the mask renderer (will not take ownership)
     */
    QgsInvertedPolygonRendererWidget( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer );

    QgsFeatureRenderer *renderer() override;

    void setContext( const QgsSymbolWidgetContext &context ) override;

  protected:
    //! The mask renderer
    std::unique_ptr<QgsInvertedPolygonRenderer> mRenderer;
    //! The widget used to represent the mask's embedded renderer
    std::unique_ptr<QgsRendererWidget> mEmbeddedRendererWidget;

  private slots:
    void mRendererComboBox_currentIndexChanged( int index );
    void mMergePolygonsCheckBox_stateChanged( int state );
};


#endif // QGSINVERTEDPOLYGONRENDERERWIDGET_H
