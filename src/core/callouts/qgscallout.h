/***************************************************************************
                             qgscallout.h
                             ----------------
    begin                : July 2019
    copyright            : (C) 2019 Nyall Dawson
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
#ifndef QGSCALLOUT_H
#define QGSCALLOUT_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsexpressioncontext.h"
#include <QString>
#include <QRectF>
#include <memory>

class QgsLineSymbol;
class QgsGeometry;
class QgsRenderContext;


/**
 * \ingroup core
 * \brief Abstract base class for callout renderers.
 *
 * Implementations of QgsCallout are responsible for performing the actual render of
 * callouts, including determining the desired shape of the callout and using any
 * relevant symbology elements to render them.
 *
 * \since QGIS 3.10
 */
class CORE_EXPORT QgsCallout
{

  public:

    /**
     * Constructor for QgsCallout.
     */
    QgsCallout() = default;
    virtual ~QgsCallout() = default;

    /**
     * Returns a unique string representing the callout type.
     */
    virtual QString type() const = 0;

    /**
     * Duplicates a callout by creating a deep copy of the callout.
     *
     * Caller takes ownership of the returned object.
     */
    virtual QgsCallout *clone() const = 0 SIP_FACTORY;

    /**
     * Returns the properties describing the callout encoded in a
     * string format.
     *
     * Subclasses must ensure that they include the base class' properties()
     * in their returned value.
     *
     * \see readProperties()
     * \see saveProperties()
     */
    virtual QVariantMap properties() const;

    /**
     * Reads a string map of an callout's properties and restores the callout
     * to the state described by the properties map.
     *
     * Subclasses must ensure that they call the base class' readProperties()
     * method.
     *
     * \see properties()
     */
    virtual void readProperties( const QVariantMap &props, const QgsReadWriteContext &context );

    /**
     * Saves the current state of the callout to a DOM \a element. The default
     * behavior is to save the properties string map returned by
     * properties().
     * \returns TRUE if save was successful
     * \see readProperties()
     */
    virtual bool saveProperties( QDomDocument &doc, QDomElement &element ) const;

    /**
     * Restores the callout's properties from a DOM element.
     *
     * The default behavior is the read the DOM contents and call readProperties() on the subclass.
     *
     * \see readProperties()
     */
    virtual void restoreProperties( const QDomElement &element, const QgsReadWriteContext &context );

    /**
     * Prepares the callout for rendering on the specified render \a context.
     *
     * \warning This MUST be called prior to calling render() on the callout, and must always
     * be accompanied by a corresponding call to stopRender().
     *
     * \see stopRender()
     */
    virtual void startRender( QgsRenderContext &context );

    /**
     * Finalises the callout after a set of rendering operations on the specified render \a context.
     *
     * \warning This MUST be called after to after render() operations on the callout, and must always
     * be accompanied by a corresponding prior call to startRender().
     *
     * \see startRender()
     */
    virtual void stopRender( QgsRenderContext &context );

    /**
     * Returns the set of attributes referenced by the callout. This includes attributes
     * required by any data defined properties associated with the callout.
     *
     * \warning This must only be called after a corresponding call to startRender() with
     * the same render \a context.
     */
    virtual QSet< QString > referencedFields( const QgsRenderContext &context ) const;

    /**
     * Renders the callout onto the specified render \a context.
     *
     * The \a rect argument gives the desired size and position of the body of the callout (e.g. the
     * actual label geometry). The \a angle argument specifies the rotation of the callout body
     * (in degrees clockwise from horizontal). It is assumed that angle rotation specified via \a angle
     * is applied around the center of \a rect.
     *
     * The \a anchor argument dictates the geometry which the callout should connect to. Depending on the
     * callout subclass and anchor geometry type, the actual shape of the rendered callout may vary.
     * E.g. a subclass may prefer to attach to the centroid of the \a anchor, while another subclass may
     * prefer to attach to the closest point on \a anchor instead.
     *
     * Both \a rect and \a anchor must be specified in painter coordinates (i.e. pixels).
     *
     * \warning A prior call to startRender() must have been made before calling this method, and
     * after all render() operations are complete a call to stopRender() must be made.
     */
    void render( QgsRenderContext &context, QRectF rect, const double angle, const QgsGeometry &anchor );

    /**
     * Returns TRUE if the the callout is enabled.
     * \see setEnabled()
     */
    bool enabled() const { return mEnabled; }

    /**
     * Sets whether the callout is \a enabled.
     * \see enabled()
     */
    void setEnabled( bool enabled );

  protected:

    /**
     * Performs the actual rendering of the callout implementation onto the specified render \a context.
     *
     * The \a rect argument gives the desired size and position of the body of the callout (e.g. the
     * actual label geometry). The \a angle argument specifies the rotation of the callout body
     * (in degrees clockwise from horizontal). It is assumed that angle rotation specified via \a angle
     * is applied around the center of \a rect.
     *
     * The \a anchor argument dictates the geometry which the callout should connect to. Depending on the
     * callout subclass and anchor geometry type, the actual shape of the rendered callout may vary.
     * E.g. a subclass may prefer to attach to the centroid of the \a anchor, while another subclass may
     * prefer to attach to the closest point on \a anchor instead.
     *
     * Both \a rect and \a anchor are specified in painter coordinates (i.e. pixels).
     */
    virtual void draw( QgsRenderContext &context, QRectF rect, const double angle, const QgsGeometry &anchor ) = 0;

  private:

    bool mEnabled = true;

};

/**
 * \ingroup core
 * \brief A simple direct line callout style.
 *
 * \since QGIS 3.10
 */
class CORE_EXPORT QgsSimpleLineCallout : public QgsCallout
{
  public:

    QgsSimpleLineCallout();
    QgsSimpleLineCallout( const QgsSimpleLineCallout &other );
    QgsSimpleLineCallout &operator=( const QgsSimpleLineCallout & );

    QString type() const override;
    QgsSimpleLineCallout *clone() const override;
    QVariantMap properties() const override;
    void readProperties( const QVariantMap &props, const QgsReadWriteContext &context ) override;
    void startRender( QgsRenderContext &context ) override;
    void stopRender( QgsRenderContext &context ) override;
    QSet< QString > referencedFields( const QgsRenderContext &context ) const override;

    QgsLineSymbol *lineSymbol();

    void setLineSymbol( QgsLineSymbol *symbol SIP_TRANSFER );

  protected:
    void draw( QgsRenderContext &context, QRectF rect, const double angle, const QgsGeometry &anchor ) override;

  private:

    std::unique_ptr< QgsLineSymbol > mLineSymbol;
};

#endif // QGSCALLOUT_H

