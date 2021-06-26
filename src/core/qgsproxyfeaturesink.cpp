/***************************************************************************
                         qgsproxyfeaturesink.cpp
                         ----------------------
    begin                : April 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsproxyfeaturesink.h"


QgsProxyFeatureSink::QgsProxyFeatureSink( QgsFeatureSink *sink )
  : mSink( sink )
{}
