/***************************************************************************
                qgsmaprendererjobproxy.h
                ------------------------
    begin                : January 2017
    copyright            : (C) 2017 by Paul Blottiere
    email                : paul dot blottiere at oslandia dot com
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef QGSMAPRENDERERJOBPROXY_H
#define QGSMAPRENDERERJOBPROXY_H

#include "qgsmapsettings.h"
#include "qgsaccesscontrol.h"

namespace QgsWms
{

  /** \ingroup server
    * thiss class provides a proxy for sequential or parallel map render job by
    * reading qsettings.
    * @note added in QGIS 3.0
    */
  class QgsMapRendererJobProxy
  {
    public:

      /** Constructor.
        * @param accessControl Does not take ownership of QgsAccessControl
        */
      QgsMapRendererJobProxy(
        bool parallelRendering
        , int maxThreads
#ifdef HAVE_SERVER_PYTHON_PLUGINS
        , QgsAccessControl* accessControl
#endif
      );

      /** Sequential or parallel map rendering according to qsettings.
        * @param mapSettings passed to MapRendererJob
        * @param the rendered image
        */
      void render( const QgsMapSettings& mapSettings, QImage* image );

      /** Take ownership of the painter used for rendering.
        * @return painter
        */
      QPainter* takePainter();

    private:
      bool mParallelRendering;
#ifdef HAVE_SERVER_PYTHON_PLUGINS
      QgsAccessControl* mAccessControl;
#endif
      QScopedPointer<QPainter> mPainter;
  };


}
#endif
