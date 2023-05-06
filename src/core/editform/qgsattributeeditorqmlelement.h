/***************************************************************************
  qgsattributeeditorqmlelement.h - QgsAttributeEditorElement

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
#ifndef QGSATTRIBUTEEDITORQMLELEMENT_H
#define QGSATTRIBUTEEDITORQMLELEMENT_H

#include "qgis_core.h"
#include "qgsattributeeditorelement.h"

/**
 * \ingroup core
 * \brief An attribute editor widget that will represent arbitrary QML code.
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
      : QgsAttributeEditorElement( Qgis::AttributeEditorType::QmlElement, name, parent )
    {}

    QgsAttributeEditorElement *clone( QgsAttributeEditorElement *parent ) const override SIP_FACTORY;

    /**
     * The QML code that will be represented within this widget.
     *
     * \since QGIS 3.4
     */
    QString qmlCode() const;

    /**
     * Sets the QML code that will be represented within this widget to \a qmlCode.
     */
    void setQmlCode( const QString &qmlCode );

  private:
    void saveConfiguration( QDomElement &elem, QDomDocument &doc ) const override;
    void loadConfiguration( const QDomElement &element,  const QString &layerId, const QgsReadWriteContext &context, const QgsFields &fields ) override;
    QString typeIdentifier() const override;
    QString mQmlCode;
};

#endif // QGSATTRIBUTEEDITORQMLELEMENT_H
