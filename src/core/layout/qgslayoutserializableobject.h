/***************************************************************************
                          qgslayoutserializableobject.h
                          -----------------------------
    begin                : July 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#ifndef QGSLAYOUTSERIALIZABLEOBJECT_H
#define QGSLAYOUTSERIALIZABLEOBJECT_H

#include "qgis.h"
#include "qgis_core.h"
#include "qgslayoutundocommand.h"

class QDomElement;
class QDomDocument;
class QgsReadWriteContext;
class QgsAbstractLayoutUndoCommand;

/**
 * \ingroup core
 * An interface for layout objects which can be stored and read from DOM elements.
 * \since QGIS 3.0
*/
class CORE_EXPORT QgsLayoutSerializableObject : public QgsLayoutUndoObjectInterface
{
  public:

    /**
     * Returns the object type as a string.
     *
     * This string must be a unique, single word, character only representation of the item type, eg "LayoutScaleBar"
     */
    virtual QString stringType() const = 0;

    /**
     * Returns the layout the object belongs to.
     */
    virtual QgsLayout *layout() = 0;

    /**
     * Stores the objects's state in a DOM element. The \a parentElement should refer to the parent layout's DOM element.
     * \see readXml()
     */
    virtual bool writeXml( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext &context ) const = 0;

    /**
     * Sets the objects's state from a DOM element. \a element is the DOM node corresponding to the object.
     * \see writeXml()
     */
    virtual bool readXml( const QDomElement &element, const QDomDocument &document, const QgsReadWriteContext &context ) = 0;

    QgsAbstractLayoutUndoCommand *createCommand( const QString &text, int id, QUndoCommand *parent = nullptr ) override SIP_FACTORY;

  private:

    friend class QgsLayoutSerializableObjectUndoCommand;

};

#endif // QGSLAYOUTSERIALIZABLEOBJECT_H
