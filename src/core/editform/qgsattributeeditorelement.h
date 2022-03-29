/***************************************************************************
  qgsattributeeditorelement.h - QgsAttributeEditorElement

 ---------------------
 begin                : 18.8.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSATTRIBUTEEDITORELEMENT_H
#define QGSATTRIBUTEEDITORELEMENT_H

#include "qgis_core.h"
#include "qgsrelation.h"
#include "qgsoptionalexpression.h"
#include "qgspropertycollection.h"
#include <QColor>
#include <QFont>

/**
 * \ingroup core
 * \brief This is an abstract base class for any elements of a drag and drop form.
 *
 * This can either be a container which will be represented on the screen
 * as a tab widget or a collapsible group box. Or it can be a field which will
 * then be represented based on the QgsEditorWidget type and configuration.
 * Or it can be a relation and embed the form of several children of another
 * layer.
 */

class CORE_EXPORT QgsAttributeEditorElement SIP_ABSTRACT
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    switch ( sipCpp->type() )
    {
      case QgsAttributeEditorElement::AeTypeContainer:
        sipType = sipType_QgsAttributeEditorContainer;
        break;
      case QgsAttributeEditorElement::AeTypeField:
        sipType = sipType_QgsAttributeEditorField;
        break;
      case QgsAttributeEditorElement::AeTypeRelation:
        sipType = sipType_QgsAttributeEditorRelation;
        break;
      case QgsAttributeEditorElement::AeTypeAction:
        sipType = sipType_QgsAttributeEditorAction;
        break;
      default:
        sipType = nullptr;
        break;
    }
    SIP_END
#endif
  public:
    enum AttributeEditorType
    {
      AeTypeContainer, //!< A container
      AeTypeField,     //!< A field
      AeTypeRelation,  //!< A relation
      AeTypeInvalid,   //!< Invalid
      AeTypeQmlElement, //!< A QML element
      AeTypeHtmlElement, //!< A HTML element
      AeTypeAction //!< A layer action element (since QGIS 3.22)
    };

    /**
     * Constructor
     *
     * \param type The type of the new element.
     * \param name
     * \param parent
     */
    QgsAttributeEditorElement( AttributeEditorType type, const QString &name, QgsAttributeEditorElement *parent = nullptr )
      : mType( type )
      , mName( name )
      , mParent( parent )
      , mShowLabel( true )
    {}

    virtual ~QgsAttributeEditorElement() = default;

    /**
     * Constructs the editor element from the given element
     *
     * \since QGIS 3.18
     */
    static QgsAttributeEditorElement *create( const QDomElement &element, const QString &layerId, const QgsFields &fields, const QgsReadWriteContext &context, QgsAttributeEditorElement *parent = nullptr ) SIP_FACTORY;

    /**
     * Returns the name of this element
     *
     * \returns The name for this element
     */
    QString name() const { return mName; }

    /**
     * The type of this element
     *
     * \returns The type
     */
    AttributeEditorType type() const { return mType; }

    /**
     * Gets the parent of this element.
     *
     * \since QGIS 3.0
     */
    QgsAttributeEditorElement *parent() const { return mParent; }

    /**
     * Gets the XML Dom element to save this element.
     *
     * \param doc The QDomDocument which is used to create new XML elements
     *
     * \returns A DOM element to serialize this element
     */
    QDomElement toDomElement( QDomDocument &doc ) const;

    /**
     * Returns a clone of this element. To be implemented by subclasses.
     *
     * \since QGIS 3.0
     */
    virtual QgsAttributeEditorElement *clone( QgsAttributeEditorElement *parent ) const = 0 SIP_FACTORY;

    /**
     * Controls if this element should be labeled with a title (field, relation or groupname).
     *
     * \since QGIS 2.18
     */
    bool showLabel() const;

    /**
     * Controls if this element should be labeled with a title (field, relation or groupname).
     * \since QGIS 2.18
     */
    void setShowLabel( bool showLabel );

    /**
     * Returns the custom label font, this is only effective if overrideStyleLabel() is also set.
     * \see setLabelFont()
     * \see overrideLabelStyle()
     * \see setOverrideLabelStyle()
     * \since QGIS 3.26
     */
    const QFont labelFont() const;

    /**
     * Sets the custom \a labelFont, this is only effective if overrideStyleLabel() is also set.
     * \see setLabelFont()
     * \see overrideLabelStyle()
     * \see setOverrideLabelStyle()
     * \since QGIS 3.26
     */
    void setLabelFont( const QFont &labelFont );

    /**
     * Returns the custom label color, this is only effective if overrideStyleLabel() is also set.
     * \see setLabelColor()
     * \see overrideLabelStyle()
     * \see setOverrideLabelStyle()
     * \since QGIS 3.26
     */
    const QColor labelColor() const;

    /**
     * Sets the custom \a labelColor, this is only effective if overrideStyleLabel() is also set.
     * \see setLabelColor()
     * \see overrideLabelStyle()
     * \see setOverrideLabelStyle()
     * \since QGIS 3.26
     */
    void setLabelColor( const QColor &labelColor );

    /**
     * Returns TRUE if the label style (font and color) is overridden.
     * \see labelColor()
     * \see labelFont()
     * \see setOverrideLabelStyle()
     * \since QGIS 3.26
     */
    bool overrideLabelStyle() const;

    /**
     * Sets \a overrideLabelStyle flag which determines if label style (font and color) is overridden.
     * \see labelColor()
     * \see labelFont()
     * \see overrideLabelStyle()
     * \since QGIS 3.26
     */
    void setOverrideLabelStyle( bool overrideLabelStyle );

  protected:
#ifndef SIP_RUN
    AttributeEditorType mType;
    QString mName;
    QgsAttributeEditorElement *mParent = nullptr;
    bool mShowLabel;
    QFont mLabelFont;
    QColor mLabelColor;
    bool mOverrideLabelStyle = false;
#endif

  private:

    /**
     * Should be implemented by subclasses to save type specific configuration.
     *
     * \since QGIS 2.18
     */
    virtual void saveConfiguration( QDomElement &elem, QDomDocument &doc ) const = 0;

    /**
      * Should be implemented by subclasses to read specific configuration
      * \since QGIS 3.18
      */
    virtual void loadConfiguration( const QDomElement &element,  const QString &layerId, const QgsReadWriteContext &context, const QgsFields &fields ) = 0;

    /**
     * All subclasses need to overwrite this method and return a type specific identifier.
     * Needs to be XML key compatible.
     *
     * \since QGIS 2.18
     */
    virtual QString typeIdentifier() const = 0;

};

#endif // QGSATTRIBUTEEDITORELEMENT_H
