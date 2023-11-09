/***************************************************************************
  qgsattributeeditortextelement.h - QgsAttributeEditorTextElement

 ---------------------
 begin                : 28.12.2022
 copyright            : (C) 2022 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSATTRIBUTEEDITORTEXTELEMENT_H
#define QGSATTRIBUTEEDITORTEXTELEMENT_H

#include "qgis_core.h"
#include "qgsattributeeditorelement.h"



/**
 * \ingroup core
 * \brief An attribute editor widget that will represent arbitrary text code.
 *
 * \since QGIS 3.30
 */
class CORE_EXPORT QgsAttributeEditorTextElement : public QgsAttributeEditorElement
{
  public:

    /**
     * Creates a new element which can display text
     *
     * \param name         The name of the widget
     * \param parent       The parent (used as container)
    */
    QgsAttributeEditorTextElement( const QString &name, QgsAttributeEditorElement *parent )
      : QgsAttributeEditorElement( Qgis::AttributeEditorType::TextElement, name, parent )
    {}

    QgsAttributeEditorElement *clone( QgsAttributeEditorElement *parent ) const override SIP_FACTORY;

    /**
     * The Text that will be represented within this widget.
     */
    QString text() const;

    /**
     * Sets the text that will be represented within this widget to \a text
     */
    void setText( const QString &text );

  private:
    void saveConfiguration( QDomElement &elem, QDomDocument &doc ) const override;
    void loadConfiguration( const QDomElement &element,  const QString &layerId, const QgsReadWriteContext &context, const QgsFields &fields ) override;
    QString typeIdentifier() const override;
    QString mText;
};


#endif // QGSATTRIBUTEEDITORTEXTELEMENT_H
