/***************************************************************************
    qgsvectorlayerlabeling.h
    ---------------------
    begin                : September 2015
    copyright            : (C) 2015 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSVECTORLAYERLABELING_H
#define QGSVECTORLAYERLABELING_H

#include <QString>
#include <QStringList>

class QDomDocument;
class QDomElement;

class QgsPalLayerSettings;
class QgsVectorLayer;
class QgsVectorLayerLabelProvider;

/** \ingroup core
 * Abstract base class - its implementations define different approaches to the labeling of a vector layer.
 *
 * @note added in 2.12
 * @note not available in Python bindings
 * @note this class is not a part of public API yet. See notes in QgsLabelingEngineV2
 */
class CORE_EXPORT QgsAbstractVectorLayerLabeling
{
  public:

    virtual ~QgsAbstractVectorLayerLabeling();

    //! Unique type string of the labeling configuration implementation
    virtual QString type() const = 0;

    //! Factory for label provider implementation
    virtual QgsVectorLayerLabelProvider* provider( QgsVectorLayer* layer ) const = 0;

    //! Return labeling configuration as XML element
    virtual QDomElement save( QDomDocument& doc ) const = 0;

    //! Get list of sub-providers within the layer's labeling.
    virtual QStringList subProviders() const { return QStringList( QString() ); }

    //! Get associated label settings. In case of multiple sub-providers with different settings,
    //! they are identified by their ID (e.g. in case of rule-based labeling, provider ID == rule key)
    virtual QgsPalLayerSettings settings( QgsVectorLayer* layer, const QString& providerId = QString() ) const = 0;

    // static stuff

    //! Try to create instance of an implementation based on the XML data
    static QgsAbstractVectorLayerLabeling* create( const QDomElement& element );
};

/** \ingroup core
 * Basic implementation of the labeling interface.
 *
 * The configuration is kept in layer's custom properties for backward compatibility.
 *
 * @note added in 2.12
 * @note not available in Python bindings
 * @note this class is not a part of public API yet. See notes in QgsLabelingEngineV2
 */
class CORE_EXPORT QgsVectorLayerSimpleLabeling : public QgsAbstractVectorLayerLabeling
{
  public:

    virtual QString type() const override;
    virtual QgsVectorLayerLabelProvider* provider( QgsVectorLayer* layer ) const override;
    virtual QDomElement save( QDomDocument& doc ) const override;
    virtual QgsPalLayerSettings settings( QgsVectorLayer* layer, const QString& providerId = QString() ) const override;
};

#endif // QGSVECTORLAYERLABELING_H
