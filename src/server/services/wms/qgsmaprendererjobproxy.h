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
#include "qgsmaprendererjob.h"

class QgsFeatureFilterProvider;

namespace QgsWms
{

  /**
   * \ingroup server
   * \class QgsWms::QgsMapRendererJobProxy
   * \brief Proxy for sequential or parallel map render job
   * \since QGIS 3.0
   */
  class QgsMapRendererJobProxy
  {
    public:

      /**
       * Constructor for QgsMapRendererJobProxy. Does not take ownership of
       * \a featureFilterProvider.
       * \param parallelRendering True to activate parallel rendering, false otherwise
       * \param maxThreads The number of threads to use in case of parallel rendering
       * \param featureFilterProvider Features filtering
       */
      QgsMapRendererJobProxy(
        bool parallelRendering
        , int maxThreads
        , QgsFeatureFilterProvider *featureFilterProvider
      );

      /**
       * Sequential or parallel map rendering.
       * \param mapSettings Passed to MapRendererJob
       * \param image The resulting image
       */
      void render( const QgsMapSettings &mapSettings, QImage *image );

      /**
       * Takes ownership of the painter used for rendering.
       * \returns painter
       */
      QPainter *takePainter();

      /**
       * \brief Returns map rendering errors
       * \returns error list
       */
      QgsMapRendererJob::Errors errors() const { return mErrors; }

    private:
      bool mParallelRendering;
      QgsFeatureFilterProvider *mFeatureFilterProvider = nullptr;
      std::unique_ptr<QPainter> mPainter;

      void getRenderErrors( const QgsMapRendererJob *job );

      //! Layer id / error message
      QgsMapRendererJob::Errors mErrors;
  };


}
#endif
