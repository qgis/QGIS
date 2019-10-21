/***************************************************************************
    qgsannotationlayer.h
    ----------------
    copyright            : (C) 2019 by Sandro Mani
    email                : smani at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSANNOTATIONLAYER_H
#define QGSANNOTATIONLAYER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerrenderer.h"

class QgsAnnotationItem;

class CORE_EXPORT QgsAnnotationLayer : public QgsMapLayer
{
    Q_OBJECT

  public:


    /**
     * Setting options for loading annotation layers.
     * \since QGIS 3.12
     */
    struct LayerOptions
    {

      /**
       * Constructor for LayerOptions.
       */
      explicit LayerOptions( const QgsCoordinateTransformContext &transformContext )
        : transformContext( transformContext )
      {}

      /**
       * Coordinate transform context
       */
      QgsCoordinateTransformContext transformContext;

    };


    QgsAnnotationLayer( const QString &name, const QgsAnnotationLayer::LayerOptions &options );

    void addItem( QgsAnnotationItem *item SIP_TRANSFER );

    //KadasMapItem *takeItem( const QString &itemId );

    const QMap<QString, QgsAnnotationItem *> &items() const { return mItems; }

    void setOpacity( double opacity ) { mOpacity = opacity; }
    double opacity() const { return mOpacity; }

//    QRectF margin() const;

    QgsAnnotationLayer *clone() const override SIP_FACTORY;

    QgsMapLayerRenderer *createMapRenderer( QgsRenderContext &rendererContext ) override SIP_FACTORY;

    QgsRectangle extent() const override;

#if 0
    virtual QString pickItem( const QgsRectangle &pickRect, const QgsMapSettings &mapSettings ) const;
    QString pickItem( const QgsPointXY &mapPos, const QgsMapSettings &mapSettings ) const;
#endif
    void setTransformContext( const QgsCoordinateTransformContext &ctx ) override;

    bool writeSymbology( QDomNode &node, QDomDocument &doc, QString &errorMessage, const QgsReadWriteContext &context, StyleCategories categories = AllStyleCategories ) const override { return true; }
    bool readSymbology( const QDomNode &node, QString &errorMessage, QgsReadWriteContext &context, StyleCategories categories = AllStyleCategories ) override { return true; }

  protected:

    QMap<QString, QgsAnnotationItem *> mItems;
    QgsCoordinateTransformContext mTransformContext;
    double mOpacity = 100;
};

#endif // QGSANNOTATIONLAYER_H
