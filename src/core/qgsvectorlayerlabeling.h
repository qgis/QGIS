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

#include <memory>

#include <QString>
#include <QStringList>
#include <QDomNode>

#include "qgis.h"

class QDomDocument;
class QDomElement;

class QgsPalLayerSettings;
class QgsReadWriteContext;
class QgsVectorLayer;
class QgsVectorLayerLabelProvider;

/**
 * \ingroup core
 * Abstract base class - its implementations define different approaches to the labeling of a vector layer.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsAbstractVectorLayerLabeling
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( sipCpp->type() == "simple" )
      sipType = sipType_QgsVectorLayerSimpleLabeling;
    else if ( sipCpp->type() == "rule-based" )
      sipType = sipType_QgsRuleBasedLabeling;
    else
      sipType = 0;
    SIP_END
#endif

  public:
    //! Default constructor
    QgsAbstractVectorLayerLabeling() = default;
    virtual ~QgsAbstractVectorLayerLabeling() = default;

    //! Unique type string of the labeling configuration implementation
    virtual QString type() const = 0;

    //! Returns a new copy of the object
    virtual QgsAbstractVectorLayerLabeling *clone() const = 0 SIP_FACTORY;

    /**
     * Factory for label provider implementation
     * \note not available in Python bindings
     */
    virtual QgsVectorLayerLabelProvider *provider( QgsVectorLayer *layer ) const SIP_SKIP { Q_UNUSED( layer ); return nullptr; }

    //! Returns labeling configuration as XML element
    virtual QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context ) const = 0;

    //! Gets list of sub-providers within the layer's labeling.
    virtual QStringList subProviders() const { return QStringList( QString() ); }

    /**
     * Gets associated label settings. In case of multiple sub-providers with different settings,
     * they are identified by their ID (e.g. in case of rule-based labeling, provider ID == rule key)
     */
    virtual QgsPalLayerSettings settings( const QString &providerId = QString() ) const = 0;

    /**
     * Set pal settings for a specific provider (takes ownership).
     *
     * \param settings Pal layer settings
     * \param providerId The id of the provider
     *
     * \since QGIS 3.0
     */
    virtual void setSettings( QgsPalLayerSettings *settings SIP_TRANSFER, const QString &providerId = QString() ) = 0;

    /**
     * Returns TRUE if drawing labels requires advanced effects like composition
     * modes, which could prevent it being used as an isolated cached image
     * or exported to a vector format.
     * \since QGIS 3.0
     */
    virtual bool requiresAdvancedEffects() const = 0;

    // static stuff

    //! Try to create instance of an implementation based on the XML data
    static QgsAbstractVectorLayerLabeling *create( const QDomElement &element, const QgsReadWriteContext &context ) SIP_FACTORY;

    /**
     * Writes the SE 1.1 TextSymbolizer element based on the current layer labeling settings
     */
    virtual void toSld( QDomNode &parent, const QgsStringMap &props ) const
    {
      Q_UNUSED( parent )
      Q_UNUSED( props )
      QDomDocument doc = parent.ownerDocument();
      parent.appendChild( doc.createComment( QStringLiteral( "SE Export for %1 not implemented yet" ).arg( type() ) ) );
    }

  protected:

    /**
     * Writes a TextSymbolizer element contents based on the provided labeling settings
     * \param parent the node that will have the text symbolizer element added to it
     * \param settings the settings getting translated to a TextSymbolizer
     * \param props a open ended set of properties that can drive/inform the SLD encoding
     */
    virtual void writeTextSymbolizer( QDomNode &parent, QgsPalLayerSettings &settings, const QgsStringMap &props ) const;

  private:
    Q_DISABLE_COPY( QgsAbstractVectorLayerLabeling )

#ifdef SIP_RUN
    QgsAbstractVectorLayerLabeling( const QgsAbstractVectorLayerLabeling &rhs );
#endif

};

/**
 * \ingroup core
 * Basic implementation of the labeling interface.
 *
 * The configuration is kept in layer's custom properties for backward compatibility.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsVectorLayerSimpleLabeling : public QgsAbstractVectorLayerLabeling
{
  public:
    //! Constructs simple labeling configuration with given initial settings
    explicit QgsVectorLayerSimpleLabeling( const QgsPalLayerSettings &settings );

    QString type() const override;
    QgsAbstractVectorLayerLabeling *clone() const override SIP_FACTORY;
    //! \note not available in Python bindings
    QgsVectorLayerLabelProvider *provider( QgsVectorLayer *layer ) const override SIP_SKIP;
    QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context ) const override;
    QgsPalLayerSettings settings( const QString &providerId = QString() ) const override;

    /**
     * Set pal settings (takes ownership).
     *
     * \param settings Pal layer settings
     * \param providerId Unused parameter
     *
     * \since QGIS 3.0
     */
    void setSettings( QgsPalLayerSettings *settings SIP_TRANSFER, const QString &providerId = QString() ) override;

    bool requiresAdvancedEffects() const override;
    void toSld( QDomNode &parent, const QgsStringMap &props ) const override;

    //! Create the instance from a DOM element with saved configuration
    static QgsVectorLayerSimpleLabeling *create( const QDomElement &element, const QgsReadWriteContext &context );

  private:
    std::unique_ptr<QgsPalLayerSettings> mSettings;
};

#endif // QGSVECTORLAYERLABELING_H
