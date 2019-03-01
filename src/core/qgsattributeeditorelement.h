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
#include <QColor>

class QgsRelationManager;

/**
 * \ingroup core
 * This is an abstract base class for any elements of a drag and drop form.
 *
 * This can either be a container which will be represented on the screen
 * as a tab widget or ca collapsible group box. Or it can be a field which will
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
      AeTypeQmlElement //!< A QML element
    };

    /**
     * Constructor
     *
     * \param type The type of the new element. Should never
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
     *
     * \since QGIS 2.18
     */
    void setShowLabel( bool showLabel );

  protected:
#ifndef SIP_RUN
    AttributeEditorType mType;
    QString mName;
    QgsAttributeEditorElement *mParent = nullptr;
    bool mShowLabel;
#endif

  private:

    /**
     * Should be implemented by subclasses to save type specific configuration.
     *
     * \since QGIS 2.18
     */
    virtual void saveConfiguration( QDomElement &elem ) const = 0;

    /**
     * All subclasses need to overwrite this method and return a type specific identifier.
     * Needs to be XML key compatible.
     *
     * \since QGIS 2.18
     */
    virtual QString typeIdentifier() const = 0;
};


/**
 * \ingroup core
 * This is a container for attribute editors, used to group them visually in the
 * attribute form if it is set to the drag and drop designer.
 */
class CORE_EXPORT QgsAttributeEditorContainer : public QgsAttributeEditorElement
{
  public:

    /**
     * Creates a new attribute editor container
     *
     * \param name   The name to show as title
     * \param parent The parent. May be another container.
     * \param backgroundColor The optional background color of the container.
     */
    QgsAttributeEditorContainer( const QString &name, QgsAttributeEditorElement *parent, const QColor &backgroundColor = QColor() )
      : QgsAttributeEditorElement( AeTypeContainer, name, parent )
      , mIsGroupBox( true )
      , mColumnCount( 1 )
      , mBackgroundColor( backgroundColor )
    {}


    ~QgsAttributeEditorContainer() override;

    /**
     * Add a child element to this container. This may be another container, a field or a relation.
     *
     * \param element The element to add as child
     */
    virtual void addChildElement( QgsAttributeEditorElement *element SIP_TRANSFER );

    /**
     * Determines if this container is rendered as collapsible group box or tab in a tabwidget
     *
     * \param isGroupBox If TRUE, this will be a group box
     */
    virtual void setIsGroupBox( bool isGroupBox ) { mIsGroupBox = isGroupBox; }

    /**
     * Returns if this container is going to be rendered as a group box
     *
     * \returns TRUE if it will be a group box, FALSE if it will be a tab
     */
    virtual bool isGroupBox() const { return mIsGroupBox; }

    /**
     * Gets a list of the children elements of this container
     *
     * \returns A list of elements
     */
    QList<QgsAttributeEditorElement *> children() const { return mChildren; }

    /**
     * Traverses the element tree to find any element of the specified type
     *
     * \param type The type which should be searched
     *
     * \returns A list of elements of the type which has been searched for
     */
    virtual QList<QgsAttributeEditorElement *> findElements( AttributeEditorType type ) const;

    /**
     * Clear all children from this container.
     */
    void clear();

    /**
     * Change the name of this container
     */
    void setName( const QString &name );

    /**
     * Gets the number of columns in this group
     */
    int columnCount() const;

    /**
     * Set the number of columns in this group
     */
    void setColumnCount( int columnCount );

    /**
     * Creates a deep copy of this element. To be implemented by subclasses.
     *
     * \since QGIS 3.0
     */
    QgsAttributeEditorElement *clone( QgsAttributeEditorElement *parent ) const override SIP_FACTORY;

    /**
     * The visibility expression is used in the attribute form to
     * show or hide this container based on an expression incorporating
     * the field value controlled by editor widgets.
     *
     * \since QGIS 3.0
     */
    QgsOptionalExpression visibilityExpression() const;

    /**
     * The visibility expression is used in the attribute form to
     * show or hide this container based on an expression incorporating
     * the field value controlled by editor widgets.
     *
     * \since QGIS 3.0
     */
    void setVisibilityExpression( const QgsOptionalExpression &visibilityExpression );

    /**
     * \brief backgroundColor
     * \return background color of the container
     * \since QGIS 3.8
     */
    QColor backgroundColor() const;

    /**
     * Sets the background color to \a backgroundColor
     */
    void setBackgroundColor( const QColor &backgroundColor );

  private:
    void saveConfiguration( QDomElement &elem ) const override;
    QString typeIdentifier() const override;

    bool mIsGroupBox;
    QList<QgsAttributeEditorElement *> mChildren;
    int mColumnCount;
    QgsOptionalExpression mVisibilityExpression;
    QColor mBackgroundColor;
};

/**
 * \ingroup core
 * This element will load a field's widget onto the form.
 */
class CORE_EXPORT QgsAttributeEditorField : public QgsAttributeEditorElement
{
  public:

    /**
     * Creates a new attribute editor element which represents a field
     *
     * \param name   The name of the element
     * \param idx    The index of the field which should be embedded
     * \param parent The parent of this widget (used as container)
     */
    QgsAttributeEditorField( const QString &name, int idx, QgsAttributeEditorElement *parent )
      : QgsAttributeEditorElement( AeTypeField, name, parent )
      , mIdx( idx )
    {}

    /**
     * Returns the index of the field.
     */
    int idx() const { return mIdx; }

    QgsAttributeEditorElement *clone( QgsAttributeEditorElement *parent ) const override SIP_FACTORY;

  private:
    void saveConfiguration( QDomElement &elem ) const override;
    QString typeIdentifier() const override;
    int mIdx;
};

/**
 * \ingroup core
 * This element will load a relation editor onto the form.
 */
class CORE_EXPORT QgsAttributeEditorRelation : public QgsAttributeEditorElement
{
  public:

    /**
     * \deprecated since QGIS 3.0.2. The name parameter is not used for anything and overwritten by the relationId internally.
     */
    Q_DECL_DEPRECATED QgsAttributeEditorRelation( const QString &name, const QString &relationId, QgsAttributeEditorElement *parent )
      : QgsAttributeEditorElement( AeTypeRelation, name, parent )
      , mRelationId( relationId )
    {}

    /**
     * \deprecated since QGIS 3.0.2. The name parameter is not used for anything and overwritten by the relationId internally.
     */
    Q_DECL_DEPRECATED QgsAttributeEditorRelation( const QString &name, const QgsRelation &relation, QgsAttributeEditorElement *parent )
      : QgsAttributeEditorElement( AeTypeRelation, name, parent )
      , mRelationId( relation.id() )
      , mRelation( relation )
    {}

    /**
     * Creates a new element which embeds a relation.
     *
     * \param relationId   The id of the relation to embed
     * \param parent       The parent (used as container)
     */
    QgsAttributeEditorRelation( const QString &relationId, QgsAttributeEditorElement *parent )
      : QgsAttributeEditorElement( AeTypeRelation, relationId, parent )
      , mRelationId( relationId )
    {}

    /**
     * Creates a new element which embeds a relation.
     *
     * \param relation     The relation to embed
     * \param parent       The parent (used as container)
     */
    QgsAttributeEditorRelation( const QgsRelation &relation, QgsAttributeEditorElement *parent )
      : QgsAttributeEditorElement( AeTypeRelation, relation.id(), parent )
      , mRelationId( relation.id() )
      , mRelation( relation )
    {}


    /**
     * Gets the id of the relation which shall be embedded
     *
     * \returns the id
     */
    const QgsRelation &relation() const { return mRelation; }

    /**
     * Initializes the relation from the id
     *
     * \param relManager The relation manager to use for the initialization
     * \returns TRUE if the relation was found in the relationmanager
     */
    bool init( QgsRelationManager *relManager );

    QgsAttributeEditorElement *clone( QgsAttributeEditorElement *parent ) const override SIP_FACTORY;

    /**
     * Determines if the "link feature" button should be shown
     *
     * \since QGIS 2.18
     */
    bool showLinkButton() const;

    /**
     * Determines if the "link feature" button should be shown
     *
     * \since QGIS 2.18
     */
    void setShowLinkButton( bool showLinkButton );

    /**
     * Determines if the "unlink feature" button should be shown
     *
     * \since QGIS 2.18
     */
    bool showUnlinkButton() const;

    /**
     * Determines if the "unlink feature" button should be shown
     *
     * \since QGIS 2.18
     */
    void setShowUnlinkButton( bool showUnlinkButton );


  private:
    void saveConfiguration( QDomElement &elem ) const override;
    QString typeIdentifier() const override;
    QString mRelationId;
    QgsRelation mRelation;
    bool mShowLinkButton = true;
    bool mShowUnlinkButton = true;
};

/**
 * \ingroup core
 * An attribute editor widget that will represent arbitrary QML code.
 *
 * \since QGIS 3.4
 */
class CORE_EXPORT QgsAttributeEditorQmlElement : public QgsAttributeEditorElement
{
  public:

    /**
     * Creates a new element which can display QML
     *
     * \param name         The name of the widget
     * \param parent       The parent (used as container)
    */
    QgsAttributeEditorQmlElement( const QString &name, QgsAttributeEditorElement *parent )
      : QgsAttributeEditorElement( AeTypeQmlElement, name, parent )
    {}

    QgsAttributeEditorElement *clone( QgsAttributeEditorElement *parent ) const override SIP_FACTORY;

    /**
     * The QML code that will be represented within this widget.
     *
     * \since QGIS 3.4
     */
    QString qmlCode() const;

    /**
     * The QML code that will be represented within this widget.
     *
     * @param qmlCode
     */
    void setQmlCode( const QString &qmlCode );

  private:
    void saveConfiguration( QDomElement &elem ) const override;
    QString typeIdentifier() const override;
    QString mQmlCode;
};

#endif // QGSATTRIBUTEEDITORELEMENT_H
