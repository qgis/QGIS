/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsattributeeditorelement.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsAttributeEditorElement /Abstract/
{
%Docstring
 This is an abstract base class for any elements of a drag and drop form.

 This can either be a container which will be represented on the screen
 as a tab widget or ca collapsible group box. Or it can be a field which will
 then be represented based on the QgsEditorWidget type and configuration.
 Or it can be a relation and embed the form of several children of another
 layer.
%End

%TypeHeaderCode
#include "qgsattributeeditorelement.h"
%End
%ConvertToSubClassCode
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
%End
  public:
    enum AttributeEditorType
    {
      AeTypeContainer,
      AeTypeField,
      AeTypeRelation,
      AeTypeInvalid
    };

    QgsAttributeEditorElement( AttributeEditorType type, const QString &name, QgsAttributeEditorElement *parent = 0 );
%Docstring
 Constructor

 \param type The type of the new element. Should never
 \param name
 \param parent
%End

    virtual ~QgsAttributeEditorElement();

    QString name() const;
%Docstring
 Return the name of this element

 :return: The name for this element
 :rtype: str
%End

    AttributeEditorType type() const;
%Docstring
 The type of this element

 :return: The type
 :rtype: AttributeEditorType
%End

    QgsAttributeEditorElement *parent() const;
%Docstring
 Get the parent of this element.

.. versionadded:: 3.0
 :rtype: QgsAttributeEditorElement
%End

    QDomElement toDomElement( QDomDocument &doc ) const;
%Docstring
 Get the XML Dom element to save this element.

 \param doc The QDomDocument which is used to create new XML elements

 :return: A DOM element to serialize this element
 :rtype: QDomElement
%End

    virtual QgsAttributeEditorElement *clone( QgsAttributeEditorElement *parent ) const = 0 /Factory/;
%Docstring
 Returns a clone of this element. To be implemented by subclasses.

.. versionadded:: 3.0
 :rtype: QgsAttributeEditorElement
%End

    bool showLabel() const;
%Docstring
 Controls if this element should be labeled with a title (field, relation or groupname).

.. versionadded:: 2.18
 :rtype: bool
%End

    void setShowLabel( bool showLabel );
%Docstring
 Controls if this element should be labeled with a title (field, relation or groupname).

.. versionadded:: 2.18
%End

  protected:

};


class QgsAttributeEditorContainer : QgsAttributeEditorElement
{
%Docstring
 This is a container for attribute editors, used to group them visually in the
 attribute form if it is set to the drag and drop designer.
%End

%TypeHeaderCode
#include "qgsattributeeditorelement.h"
%End
  public:

    QgsAttributeEditorContainer( const QString &name, QgsAttributeEditorElement *parent );
%Docstring
 Creates a new attribute editor container

 \param name   The name to show as title
 \param parent The parent. May be another container.
%End


    virtual ~QgsAttributeEditorContainer();

    virtual void addChildElement( QgsAttributeEditorElement *element /Transfer/ );
%Docstring
 Add a child element to this container. This may be another container, a field or a relation.

 \param element The element to add as child
%End

    virtual void setIsGroupBox( bool isGroupBox );
%Docstring
 Determines if this container is rendered as collapsible group box or tab in a tabwidget

 \param isGroupBox If true, this will be a group box
%End

    virtual bool isGroupBox() const;
%Docstring
 Returns if this container is going to be rendered as a group box

 :return: True if it will be a group box, false if it will be a tab
 :rtype: bool
%End

    QList<QgsAttributeEditorElement *> children() const;
%Docstring
 Get a list of the children elements of this container

 :return: A list of elements
 :rtype: list of QgsAttributeEditorElement
%End

    virtual QList<QgsAttributeEditorElement *> findElements( AttributeEditorType type ) const;
%Docstring
 Traverses the element tree to find any element of the specified type

 \param type The type which should be searched

 :return: A list of elements of the type which has been searched for
 :rtype: list of QgsAttributeEditorElement
%End

    void clear();
%Docstring
 Clear all children from this container.
%End

    void setName( const QString &name );
%Docstring
 Change the name of this container
%End

    int columnCount() const;
%Docstring
 Get the number of columns in this group
 :rtype: int
%End

    void setColumnCount( int columnCount );
%Docstring
 Set the number of columns in this group
%End

    virtual QgsAttributeEditorElement *clone( QgsAttributeEditorElement *parent ) const /Factory/;
%Docstring
 Creates a deep copy of this element. To be implemented by subclasses.

.. versionadded:: 3.0
 :rtype: QgsAttributeEditorElement
%End

    QgsOptionalExpression visibilityExpression() const;
%Docstring
 The visibility expression is used in the attribute form to
 show or hide this container based on an expression incorporating
 the field value controlled by editor widgets.

.. versionadded:: 3.0
 :rtype: QgsOptionalExpression
%End

    void setVisibilityExpression( const QgsOptionalExpression &visibilityExpression );
%Docstring
 The visibility expression is used in the attribute form to
 show or hide this container based on an expression incorporating
 the field value controlled by editor widgets.

.. versionadded:: 3.0
%End

};

class QgsAttributeEditorField : QgsAttributeEditorElement
{
%Docstring
 This element will load a field's widget onto the form.
%End

%TypeHeaderCode
#include "qgsattributeeditorelement.h"
%End
  public:

    QgsAttributeEditorField( const QString &name, int idx, QgsAttributeEditorElement *parent );
%Docstring
 Creates a new attribute editor element which represents a field

 \param name   The name of the element
 \param idx    The index of the field which should be embedded
 \param parent The parent of this widget (used as container)
%End

    int idx() const;
%Docstring
 Return the index of the field
 :return:
 :rtype: int
%End

    virtual QgsAttributeEditorElement *clone( QgsAttributeEditorElement *parent ) const /Factory/;

};

class QgsAttributeEditorRelation : QgsAttributeEditorElement
{
%Docstring
 This element will load a relation editor onto the form.
%End

%TypeHeaderCode
#include "qgsattributeeditorelement.h"
%End
  public:

    QgsAttributeEditorRelation( const QString &name, const QString &relationId, QgsAttributeEditorElement *parent );
%Docstring
 Creates a new element which embeds a relation.

 \param name         The name of this element
 \param relationId   The id of the relation to embed
 \param parent       The parent (used as container)
%End

    QgsAttributeEditorRelation( const QString &name, const QgsRelation &relation, QgsAttributeEditorElement *parent );
%Docstring
 Creates a new element which embeds a relation.

 \param name         The name of this element
 \param relation     The relation to embed
 \param parent       The parent (used as container)
%End

    const QgsRelation &relation() const;
%Docstring
 Get the id of the relation which shall be embedded

 :return: the id
 :rtype: QgsRelation
%End

    bool init( QgsRelationManager *relManager );
%Docstring
 Initializes the relation from the id

 \param relManager The relation manager to use for the initialization
 :return: true if the relation was found in the relationmanager
 :rtype: bool
%End

    virtual QgsAttributeEditorElement *clone( QgsAttributeEditorElement *parent ) const /Factory/;

    bool showLinkButton() const;
%Docstring
 Determines if the "link feature" button should be shown

.. versionadded:: 2.18
 :rtype: bool
%End

    void setShowLinkButton( bool showLinkButton );
%Docstring
 Determines if the "link feature" button should be shown

.. versionadded:: 2.18
%End

    bool showUnlinkButton() const;
%Docstring
 Determines if the "unlink feature" button should be shown

.. versionadded:: 2.18
 :rtype: bool
%End

    void setShowUnlinkButton( bool showUnlinkButton );
%Docstring
 Determines if the "unlink feature" button should be shown

.. versionadded:: 2.18
%End


};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsattributeeditorelement.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
