/***************************************************************************
  qgsmaplayerrenderer.h
  --------------------------------------
  Date                 : December 2013
  Copyright            : (C) 2013 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPLAYERRENDERER_H
#define QGSMAPLAYERRENDERER_H

#include <QStringList>

/**
 * Base class for utility classes that encapsulate information necessary
 * for rendering of map layers. The rendering is typically done in a background
 * thread, so it is necessary to keep all structures required for rendering away
 * from the original map layer because it may change any time.
 *
 * Because the data needs to be copied (to avoid the need for locking),
 * is is highly desirable to use copy-on-write where possible. This way,
 * the overhead of copying (both memory and CPU) will be kept low.
 * Qt containers and various Qt classes use implicit sharing.
 *
 * The scenario will be:
 * 1. renderer job (doing preparation in the GUI thread) calls
 *    QgsMapLayer::createMapRenderer() and gets instance of this class.
 *    The instance is initialized at that point and should not need
 *    additional calls to QgsVectorLayer.
 * 2. renderer job (still in GUI thread) stores the renderer for later use.
 * 3. renderer job (in worker thread) calls QgsMapLayerRenderer::render()
 * 4. renderer job (again in GUI thread) will check errors() and report them
 *
 * @note added in 2.4
 */
class CORE_EXPORT QgsMapLayerRenderer
{
  public:
    QgsMapLayerRenderer( const QString& layerID ) : mLayerID( layerID ) {}
    virtual ~QgsMapLayerRenderer() {}

    //! Do the rendering (based on data stored in the class)
    virtual bool render() = 0;

    //! Return list of errors (problems) that happened during the rendering
    QStringList errors() const { return mErrors; }

    //! Get access to the ID of the layer rendered by this class
    QString layerID() const { return mLayerID; }

  protected:
    QStringList mErrors;
    QString mLayerID;
};

#endif // QGSMAPLAYERRENDERER_H
