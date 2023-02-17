/***************************************************************************
                              qgswmsgetlegendgraphics.h
                              -------------------------
  begin                : December 20 , 2016
  copyright            : (C) 2007 by Marco Hugentobler  (original code)
                         (C) 2014 by Alessandro Pasotti (original code)
                         (C) 2016 by David Marteau
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
                         a dot pasotti at itopen dot it
                         david dot marteau at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgslayertreemodel.h"

#include "qgswmsrendercontext.h"

namespace QgsWms
{

  /**
   * Output GetLegendGRaphics response
   */
  void writeGetLegendGraphics( QgsServerInterface *serverIface, const QgsProject *project,
                               const QgsWmsRequest &request,
                               QgsServerResponse &response );

  /**
   * checkParameters checks request \a parameters and sets SRCHEIGHT and SRCWIDTH to default values
   * in case BBOX is specified for contextual legend and (SRC)HEIGHT or (SRC)WIDTH are not.
   */
  void checkParameters( QgsWmsParameters &parameters );

  QgsLayerTreeModel *legendModel( const QgsWmsRenderContext &context, QgsLayerTree &tree );

  QgsLayerTree *layerTree( const QgsWmsRenderContext &context );

  QgsLayerTree *layerTreeWithGroups( const QgsWmsRenderContext &context, QgsLayerTree *projectRoot );

  QgsLayerTreeModelLegendNode *legendNode( const QString &rule, QgsLayerTreeModel &model );
} // namespace QgsWms
