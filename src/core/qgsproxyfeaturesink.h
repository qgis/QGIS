/***************************************************************************
                         qgsproxyfeaturesink.h
                         ----------------------
    begin                : April 2020
    copyright            : (C) 2020 by Nyall Dawson
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

#ifndef QGSPROXYFEATURESINK_H
#define QGSPROXYFEATURESINK_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgsfeaturesink.h"


/**
 * \class QgsProxyFeatureSink
 * \ingroup core
 * \brief A simple feature sink which proxies feature addition on to another feature sink.
 *
 * This class is designed to allow factory methods which always return new QgsFeatureSink
 * objects. Since it is not always possible to create an entirely new QgsFeatureSink
 * (e.g. if the feature sink is a layer's data provider), a new QgsProxyFeatureSink
 * can instead be returned which forwards features on to the destination sink. The
 * proxy sink can be safely deleted without affecting the destination sink.
 *
 */
class CORE_EXPORT QgsProxyFeatureSink : public QgsFeatureSink
{
  public:

    /**
     * Constructs a new QgsProxyFeatureSink which forwards features onto a destination \a sink.
     */
    QgsProxyFeatureSink( QgsFeatureSink *sink );
    bool addFeature( QgsFeature &feature, QgsFeatureSink::Flags flags = QgsFeatureSink::Flags() ) override;
    bool addFeatures( QgsFeatureList &features, QgsFeatureSink::Flags flags = QgsFeatureSink::Flags() ) override;
    bool addFeatures( QgsFeatureIterator &iterator, QgsFeatureSink::Flags flags = QgsFeatureSink::Flags() ) override;
    QString lastError() const override;
    bool flushBuffer() override;
    void finalize() override;

    /**
     * Returns the destination QgsFeatureSink which the proxy will forward features to.
     */
    QgsFeatureSink *destinationSink() { return mSink; }

  protected:

    //! Underlying destination sink
    QgsFeatureSink *mSink = nullptr;
};


#endif // QGSPROXYFEATURESINK_H




