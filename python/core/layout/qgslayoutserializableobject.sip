/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutserializableobject.h                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsLayoutSerializableObject : QgsLayoutUndoObjectInterface
{
%Docstring
An interface for layout objects which can be stored and read from DOM elements.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutserializableobject.h"
%End
  public:

    virtual QString stringType() const = 0;
%Docstring
Return the object type as a string.

This string must be a unique, single word, character only representation of the item type, eg "LayoutScaleBar"
%End

    virtual QgsLayout *layout() = 0;
%Docstring
Returns the layout the object belongs to.
%End

    virtual bool writeXml( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext &context ) const = 0;
%Docstring
Stores the objects's state in a DOM element. The ``parentElement`` should refer to the parent layout's DOM element.

.. seealso:: :py:func:`readXml`
%End

    virtual bool readXml( const QDomElement &element, const QDomDocument &document, const QgsReadWriteContext &context ) = 0;
%Docstring
Sets the objects's state from a DOM element. ``element`` is the DOM node corresponding to the object.

.. seealso:: :py:func:`writeXml`
%End

    virtual QgsAbstractLayoutUndoCommand *createCommand( const QString &text, int id, QUndoCommand *parent = 0 ) /Factory/;


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutserializableobject.h                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
