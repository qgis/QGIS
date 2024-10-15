/***************************************************************************
                         qgsmeshlayerlabeling.h
                         ---------------------
    begin                : November 2023
    copyright            : (C) 2023 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSMESHLAYERLABELING_H
#define QGSMESHLAYERLABELING_H

#include <memory>

#include <QString>
#include <QStringList>
#include <QDomNode>

#include "qgis.h"

class QDomDocument;
class QDomElement;

class QgsPalLayerSettings;
class QgsReadWriteContext;
class QgsMeshLayer;
class QgsMeshLayerLabelProvider;
class QgsStyleEntityVisitorInterface;


/**
 * \ingroup core
 * \brief Abstract base class - its implementations define different approaches to the labeling of a mesh layer.
 *
 * \since QGIS 3.36
 */
class CORE_EXPORT QgsAbstractMeshLayerLabeling
{
  public:

    QgsAbstractMeshLayerLabeling() = default;
    virtual ~QgsAbstractMeshLayerLabeling() = default;

    //! Unique type string of the labeling configuration implementation
    virtual QString type() const = 0;

    //! Returns a new copy of the object
    virtual QgsAbstractMeshLayerLabeling *clone() const = 0 SIP_FACTORY;

    /**
     * Factory for label provider implementation
     * \note not available in Python bindings
     */
    virtual QgsMeshLayerLabelProvider *provider( QgsMeshLayer *layer ) const SIP_SKIP { Q_UNUSED( layer ) return nullptr; }

    //! Returns labeling configuration as XML element
    virtual QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context ) const = 0;

    //! Gets list of sub-providers within the layer's labeling.
    virtual QStringList subProviders() const { return QStringList( QString() ); }

    /**
     * Gets associated label settings. In case of multiple sub-providers with different settings,
     * they are identified by their ID.
     */
    virtual QgsPalLayerSettings settings( const QString &providerId = QString() ) const = 0;

    /**
     * Set pal settings for a specific provider (takes ownership).
     *
     * \param settings Pal layer settings
     * \param providerId The id of the provider
     */
    virtual void setSettings( QgsPalLayerSettings *settings SIP_TRANSFER, const QString &providerId = QString() ) = 0;

    /**
     * Returns TRUE if drawing labels requires advanced effects like composition
     * modes, which could prevent it being used as an isolated cached image
     * or exported to a vector format.
     */
    virtual bool requiresAdvancedEffects() const = 0;

    /**
     * Multiply opacity by \a opacityFactor.
     *
     * This method multiplies the opacity of the labeling elements (text, shadow, buffer etc.)
     * by \a opacity effectively changing the opacity of the whole labeling elements.
     */
    virtual void multiplyOpacity( double opacityFactor ) { Q_UNUSED( opacityFactor ); };


    // static stuff

    //! Try to create instance of an implementation based on the XML data
    static QgsAbstractMeshLayerLabeling *create( const QDomElement &element, const QgsReadWriteContext &context ) SIP_FACTORY;

    /**
     * Writes the SE 1.1 TextSymbolizer element based on the current layer labeling settings
     */
    virtual void toSld( QDomNode &parent, const QVariantMap &props ) const
    {
      Q_UNUSED( parent )
      Q_UNUSED( props )
      QDomDocument doc = parent.ownerDocument();
      parent.appendChild( doc.createComment( QStringLiteral( "SE Export for %1 not implemented yet" ).arg( type() ) ) );
    }

    /**
     * Accepts the specified symbology \a visitor, causing it to visit all symbols associated
     * with the labeling.
     *
     * Returns TRUE if the visitor should continue visiting other objects, or FALSE if visiting
     * should be canceled.
     */
    virtual bool accept( QgsStyleEntityVisitorInterface *visitor ) const;

    /**
     * Returns the default layer settings to use for the specified mesh \a layer.
     */
    static QgsPalLayerSettings defaultSettingsForLayer( const QgsMeshLayer *layer );

  private:
    Q_DISABLE_COPY( QgsAbstractMeshLayerLabeling )

#ifdef SIP_RUN
    QgsAbstractMeshLayerLabeling( const QgsAbstractMeshLayerLabeling &rhs );
#endif

};

/**
 * \ingroup core
 * \brief Basic implementation of the labeling interface for mesh layer.
 *
 * \since QGIS 3.36
 */
class CORE_EXPORT QgsMeshLayerSimpleLabeling : public QgsAbstractMeshLayerLabeling
{
  public:

    /**
     *  Constructs simple labeling configuration with given initial \a settings.
     *  Labels are placed on mesh vertices unless \a labelFaces is TRUE, when they are placed on mesh faces.
     */
    explicit QgsMeshLayerSimpleLabeling( const QgsPalLayerSettings &settings, bool labelFaces = false );

    QString type() const override;
    QgsMeshLayerSimpleLabeling *clone() const override SIP_FACTORY;
    //! \note not available in Python bindings
    QgsMeshLayerLabelProvider *provider( QgsMeshLayer *layer ) const override SIP_SKIP;
    QDomElement save( QDomDocument &doc, const QgsReadWriteContext &context ) const override;
    QgsPalLayerSettings settings( const QString &providerId = QString() ) const override;
    bool accept( QgsStyleEntityVisitorInterface *visitor ) const override;

    /**
     * Set pal settings (takes ownership).
     *
     * \param settings Pal layer settings
     * \param providerId Unused parameter
     */
    void setSettings( QgsPalLayerSettings *settings SIP_TRANSFER, const QString &providerId = QString() ) override;

    bool requiresAdvancedEffects() const override;
    void multiplyOpacity( double opacityFactor ) override;
    //! Create the instance from a DOM element with saved configuration
    static QgsMeshLayerSimpleLabeling *create( const QDomElement &element, const QgsReadWriteContext &context ); // cppcheck-suppress duplInheritedMember

  private:
    std::unique_ptr<QgsPalLayerSettings> mSettings;
    bool mLabelFaces = false;
};

#endif // QGSMESHLAYERLABELING_H
