/***************************************************************************
    qgsrenderstats.h
    ---------------------
    begin                : October 2013
    copyright            : (C) 2013 by Alvaro Huarte
    email                : http://wiki.osgeo.org/wiki/Alvaro_Huarte

***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRENDERSTATS_H
#define QGSRENDERSTATS_H

#include <QtCore/qdatetime.h>

//#ifdef QGISDEBUG
//#define QGISDEBUG_STATS 1
//#endif

#ifdef QGISDEBUG_STATS

/** \ingroup core
 * Contains statistical information about of a rendering operation.
 **/
class QgsRenderStats
{
public:
    QgsRenderStats();

private:
	QTime mRenderTime;

public:
    /** Number of layers drawed in the rendering operation */
	int LayerCount;

	/** Number of generalized features drawed in the rendering operation */
	int GeneralizedFeatureCount;
	/** Number of features drawed in the rendering operation */
	int FeatureCount;

	/** Number of generalized points drawed in the rendering operation */
	__int64 GeneralizedPointCount;
	/** Number of points drawed in the rendering operation */
	__int64 PointCount;

public:

	/** Starts the statistics of the rendering operation */
	bool start();

	/** Stops and flush the statistics of the rendering operation */
	bool stopAndFlush(const char* logFileName);
};
#endif

#endif
