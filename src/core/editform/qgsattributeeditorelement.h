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
#include "qgis_sip.h"
#include "qgis.h"
#include <QColor>
#include <QFont>

class QDomNode;
class QDomElement;
class QDomDocument;
class QgsFields;
class QgsReadWriteContext;

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
      case Qgis::AttributeEditorType::Container:
        sipType = sipType_QgsAttributeEditorContainer;
        break;
      case Qgis::AttributeEditorType::Field:
        sipType = sipType_QgsAttributeEditorField;
        break;
      case Qgis::AttributeEditorType::Relation:
        sipType = sipType_QgsAttributeEditorRelation;
        break;
      case Qgis::AttributeEditorType::Action:
        sipType = sipType_QgsAttributeEditorAction;
        break;
      default:
        sipType = nullptr;
        break;
    }
    SIP_END
#endif
  public:

    /**
     * The TabStyle struct defines color and font overrides for form fields, tabs and groups labels.
     * \since QGIS 3.26
     */
    struct CORE_EXPORT LabelStyle
    {

      //! Label font
      QColor color;

      //! Label font
      QFont font;

      //! Override label color
      bool overrideColor = false;

      //! Override label font
      bool overrideFont = false;

      /**
       * Reads configuration from \a node.
       * \note Not available in Python bindings
       */
      void readXml( const QDomNode &node ) SIP_SKIP;

      /**
       * Creates the XML configuration from \a document.
       * \note Not available in Python bindings
       */
      QDomElement writeXml( QDomDocument &document ) const SIP_SKIP;

      /**
       * Returns TRUE if the style is equal to \a other.
       * \note Not available in Python bindings
       */
      bool operator==( LabelStyle const  &other ) const SIP_SKIP;
    };

    /**
     * Constructor
     *
     * \param type The type of the new element.
     * \param name
     * \param parent
     */
    QgsAttributeEditorElement( Qgis::AttributeEditorType type, const QString &name, QgsAttributeEditorElement *parent = nullptr )
      : mType( type )
      , mName( name )
      , mParent( parent )
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
    Qgis::AttributeEditorType type() const { return mType; }

    /**
     * Gets the parent of this element.
     *
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
     */
    virtual QgsAttributeEditorElement *clone( QgsAttributeEditorElement *parent ) const = 0 SIP_FACTORY;

    /**
     * Controls if this element should be labeled with a title (field, relation or groupname).
     *
     */
    bool showLabel() const;

    /**
     * Controls if this element should be labeled with a title (field, relation or groupname).
     */
    void setShowLabel( bool showLabel );

    /**
     * Returns the horizontal stretch factor for the element.
     *
     * \see setHorizontalStretch()
     * \see verticalStretch()
     *
     * \since QGIS 3.32
     */
    int horizontalStretch() const { return mHorizontalStretch; }

    /**
     * Sets the horizontal \a stretch factor for the element.
     *
     * \see horizontalStretch()
     * \see setVerticalStretch()
     *
     * \since QGIS 3.32
     */
    void setHorizontalStretch( int stretch ) { mHorizontalStretch = stretch; }

    /**
     * Returns the vertical stretch factor for the element.
     *
     * \see setVerticalStretch()
     * \see horizontalStretch()
     *
     * \since QGIS 3.32
     */
    int verticalStretch() const { return mVerticalStretch; }

    /**
     * Sets the vertical \a stretch factor for the element.
     *
     * \see verticalStretch()
     * \see setHorizontalStretch()
     *
     * \since QGIS 3.32
     */
    void setVerticalStretch( int stretch ) { mVerticalStretch = stretch; }

    /**
     * Returns the label style.
     * \see setLabelStyle()
     * \since QGIS 3.26
     */
    LabelStyle labelStyle() const;

    /**
     * Sets the \a labelStyle.
     * \see labelStyle()
     * \since QGIS 3.26
     */
    void setLabelStyle( const LabelStyle &labelStyle );


  protected:
#ifndef SIP_RUN
    Qgis::AttributeEditorType mType = Qgis::AttributeEditorType::Invalid;
    QString mName;
    QgsAttributeEditorElement *mParent = nullptr;
    bool mShowLabel = true;
    int mHorizontalStretch = 0;
    int mVerticalStretch = 0;
    LabelStyle mLabelStyle;
#endif

  private:

    /**
     * Should be implemented by subclasses to save type specific configuration.
     *
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
     */
    virtual QString typeIdentifier() const = 0;

};

#endif // QGSATTRIBUTEEDITORELEMENT_H
