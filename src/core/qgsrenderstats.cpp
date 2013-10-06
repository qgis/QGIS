/***************************************************************************
    qgsrenderstats.cpp
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

#include "qgsrenderstats.h"

QgsRenderStats::QgsRenderStats() 
	: LayerCount(0), GeneralizedFeatureCount(0), FeatureCount(0), GeneralizedPointCount(0), PointCount(0)
{
}

/** Starts the statistics of the rendering operation */
bool QgsRenderStats::start()
{
	LayerCount = 0;
	GeneralizedFeatureCount = 0;
	FeatureCount = 0;
	GeneralizedPointCount = 0;
	PointCount = 0;		

	mRenderTime.start();
	return true;
}

/** Stops and flush the statistics of the rendering operation */
bool QgsRenderStats::stopAndFlush(const char* logFileName)
{
	int elapsedTime = mRenderTime.elapsed();

	if (logFileName && strlen(logFileName))
	{
		FILE* fp = fopen(logFileName, "a");

		char  messageText[255];
		float featFactor = FeatureCount==0 ? 0 : (float)(100.0*(GeneralizedFeatureCount/(double)FeatureCount));
		float geomFactor = PointCount==0   ? 0 : (float)(100.0*(GeneralizedPointCount  /(double)PointCount  ));

		sprintf(messageText, "Rendering stats:\r\n");
		fputs  (messageText, fp);
		sprintf(messageText, "LayerCount=%li FeatureCount=[%li gn=(%li)] PointCount=[%I64d gn=(%I64d)]\r\n", LayerCount, FeatureCount, GeneralizedFeatureCount, PointCount, GeneralizedPointCount);
		fputs  (messageText, fp);
		sprintf(messageText, "GeneralizedFT=%.2f%s GeneralizedPT=%.2f%s\r\n", featFactor, "%", geomFactor, "%");
		fputs  (messageText, fp);
		sprintf(messageText, "Rendering completed in (seconds): %li\r\n\r\n\r\n", (int)(elapsedTime/1000.0));
		fputs  (messageText, fp);

		fclose(fp);
		fp = NULL;

		return true;
	}
	return false;
}
