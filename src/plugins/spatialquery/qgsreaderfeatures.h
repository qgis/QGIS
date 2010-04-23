/***************************************************************************
                          qgsreaderfeatures.h
                             -------------------
    begin                : Dec 29, 2009
    copyright            : (C) 2009 by Diego Moreira And Luiz Motta
    email                : moreira.geo at gmail.com And motta.luiz at gmail.com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*  $Id: $ */
#ifndef READERFEATURES_H
#define READERFEATURES_H

#include <qgsvectorlayer.h>
#include <qgsfeature.h>

/**
* \class QgsReaderFeatures
* \brief Reader Features
*/
class QgsReaderFeatures
{
public:
    /**
    * \brief Constructor for a Reader Features.
    * \param layer Pointer to the layer.
    * \param useSelection Use or not use the features selected
    */
    QgsReaderFeatures(QgsVectorLayer *layer, bool useSelection);

    /**
    * \brief Destructor
    */
    ~QgsReaderFeatures();

    /**
    * \brief Next feature
    * \param feature reference to next Feature.
    * \returns True if has next feature.
    */
    bool nextFeature(QgsFeature & feature);

private:
    /**
    * \brief init Reader
    * \param useSelection Use or not use the features selected
    */
    void initReader(bool useSelection);

    /**
    * \brief Next feature, not using the features selected
    * \param feature reference to next Feature.
    * \returns True if has next feature.
    */
    bool nextFeatureTotal(QgsFeature & feature);

    /**
    * \brief Next feature, using the features selected
    * \param feature reference to next Feature.
    * \returns True if has next feature.
    */
    bool nextFeatureSelected(QgsFeature & feature);

    QgsVectorLayer * mLayer;
    QgsFeatureList   mListSelectedFeature;
    QList<QgsFeature>::iterator mIterSelectedFeature;
    bool (QgsReaderFeatures::* mFuncNextFeature) ( QgsFeature &);
};

#endif // READERFEATURES_H
