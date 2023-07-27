/***************************************************************************
                         qgsvectorlayerselectionproperties.h
                         ---------------
    begin                : July 2023
    copyright            : (C) 2023 by Nyall Dawson
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


#ifndef QGSVECTORLAYERSELECTIONPROPERTIES_H
#define QGSVECTORLAYERSELECTIONPROPERTIES_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgis_sip.h"
#include "qgsmaplayerselectionproperties.h"

#include <QColor>

/**
 * \class QgsVectorLayerSelectionProperties
 * \ingroup core
 * \brief Implementation of layer selection properties for vector layers.
 *
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsVectorLayerSelectionProperties : public QgsMapLayerSelectionProperties
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsVectorLayerSelectionProperties, with the specified \a parent object.
     */
    QgsVectorLayerSelectionProperties( QObject *parent SIP_TRANSFERTHIS = nullptr );

    QDomElement writeXml( QDomElement &element, QDomDocument &doc, const QgsReadWriteContext &context ) override;
    bool readXml( const QDomElement &element, const QgsReadWriteContext &context ) override;
    QgsVectorLayerSelectionProperties *clone() const override SIP_FACTORY;

    /**
     * Returns the color to use for rendering selected features in the layer.
     *
     * An invalid color indicates that the default (i.e. project level) selection
     * color should be used instead.
     *
     * \see setSelectionColor()
     */
    QColor selectionColor() const;

    /**
     * Sets the \a color to use for rendering selected features in the layer.
     *
     * An invalid \a color indicates that the default (i.e. project level) selection
     * color should be used instead.
     *
     * \see selectionColor()
     */
    void setSelectionColor( const QColor &color );

  private:

    QColor mSelectionColor;

};

#endif // QGSVECTORLAYERSELECTIONPROPERTIES_H
