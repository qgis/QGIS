/***************************************************************************
                              qgspointclusterrenderer.h
                              -------------------------
  begin                : February 2016
  copyright            : (C) 2016 by Nyall Dawson
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

#ifndef QGSPOINTCLUSTERRENDERER_H
#define QGSPOINTCLUSTERRENDERER_H

#include "qgspointdistancerenderer.h"

/** \class QgsPointClusterRenderer
 * \ingroup core
 * A renderer that automatically clusters points with the same geographic position.
 * \note added in QGIS 3.0
*/
class CORE_EXPORT QgsPointClusterRenderer: public QgsPointDistanceRenderer
{
  public:

    QgsPointClusterRenderer();

    QgsPointClusterRenderer* clone() const override;
    virtual void startRender( QgsRenderContext& context, const QgsFields& fields ) override;
    void stopRender( QgsRenderContext& context ) override;
    QDomElement save( QDomDocument& doc ) override;
    virtual QSet<QString> usedAttributes() const override;

    //! Creates a renderer from XML element
    static QgsFeatureRenderer* create( QDomElement& symbologyElem );

    /** Returns the symbol used for rendering clustered groups (but not ownership of the symbol).
     * @see setClusterSymbol()
    */
    QgsMarkerSymbol* clusterSymbol();

    /** Sets the symbol for rendering clustered groups.
     * @param symbol new cluster symbol. Ownership is transferred to the renderer.
     * @see clusterSymbol()
    */
    void setClusterSymbol( QgsMarkerSymbol* symbol );

    /** Creates a QgsPointClusterRenderer from an existing renderer.
     * @returns a new renderer if the conversion was possible, otherwise nullptr.
     */
    static QgsPointClusterRenderer* convertFromRenderer( const QgsFeatureRenderer* renderer );

  private:

    //! Symbol for point clusters
    QScopedPointer< QgsMarkerSymbol > mClusterSymbol;

    void drawGroup( QPointF centerPoint, QgsRenderContext& context, const QgsPointDistanceRenderer::ClusteredGroup& group ) override;

};

#endif // QGSPOINTCLUSTERRENDERER_H
