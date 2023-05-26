/***************************************************************************
  qgsattributeeditorfield.h - QgsAttributeEditorElement

 ---------------------
 begin                : 12.01.2021
 copyright            : (C) 2021 by Denis Rouzaud
 email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSATTRIBUTEEDITORFIELD_H
#define QGSATTRIBUTEEDITORFIELD_H

#include "qgis_core.h"
#include "qgsattributeeditorelement.h"


/**
 * \ingroup core
 * \brief This element will load a field's widget onto the form.
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
      : QgsAttributeEditorElement( Qgis::AttributeEditorType::Field, name, parent )
      , mIdx( idx )
    {}

    /**
     * Returns the index of the field.
     */
    int idx() const { return mIdx; }

    QgsAttributeEditorElement *clone( QgsAttributeEditorElement *parent ) const override SIP_FACTORY;

  private:
    void saveConfiguration( QDomElement &elem, QDomDocument &doc ) const override;
    void loadConfiguration( const QDomElement &element,  const QString &layerId, const QgsReadWriteContext &context, const QgsFields &fields ) override;
    QString typeIdentifier() const override;
    int mIdx;
};



#endif // QGSATTRIBUTEEDITORFIELD_H
