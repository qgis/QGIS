/***************************************************************************
                          qgselevationprofilecanvas.h
                          ---------------
    begin                : March 2022
    copyright            : (C) 2022 by Nyall Dawson
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

#ifndef QGSELEVATIONPROFILECANVAS_H
#define QGSELEVATIONPROFILECANVAS_H

#include "qgsconfig.h"
#include "qgis_sip.h"
#include "qgis_gui.h"
#include "qgsdistancevselevationplotcanvas.h"
#include "qgsmaplayer.h"

class QgsElevationProfilePlotItem;
class QgsAbstractProfileResults;
class QgsProfilePlotRenderer;

/**
 * \ingroup gui
 * \brief A canvas for elevation profiles.
 *
 * \since QGIS 3.26
 */
class GUI_EXPORT QgsElevationProfileCanvas : public QgsDistanceVsElevationPlotCanvas
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( qobject_cast<QgsElevationProfileCanvas *>( sipCpp ) != nullptr )
      sipType = sipType_QgsElevationProfileCanvas;
    else
      sipType = nullptr;
    SIP_END
#endif

    Q_OBJECT

  public:

    /**
     * Constructor for QgsElevationProfileCanvas, with the specified \a parent widget.
     */
    QgsElevationProfileCanvas( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    ~QgsElevationProfileCanvas() override;

    void update();

    void setProject( QgsProject *project );

    /**
     * Sets the list of \a layers to include in the profile.
     *
     * \see layers()
     */
    void setLayers( const QList< QgsMapLayer * > &layers );

    /**
     * Returns the list of layers included in the profile.
     *
     * \see setLayers()
     */
    QList< QgsMapLayer * > layers() const;

    void resizeEvent( QResizeEvent *event ) override;
  private slots:

    void generationFinished();

  private:

    void redrawResults();

    QgsProject *mProject = nullptr;

    QgsWeakMapLayerPointerList mLayers;

    QgsElevationProfilePlotItem *mPlotItem = nullptr;

    std::vector< std::unique_ptr< QgsAbstractProfileResults > > mProfileResults;

    QgsProfilePlotRenderer *mCurrentJob = nullptr;

};

#endif // QGSELEVATIONPROFILECANVAS_H
