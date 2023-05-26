/***************************************************************************
  qgsattributeeditorspacerelement.h - QgsAttributeEditorSpacerElement

 ---------------------
 begin                : 16.1.2023
 copyright            : (C) 2023 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSATTRIBUTEEDITORSPACERELEMENT_H
#define QGSATTRIBUTEEDITORSPACERELEMENT_H

#include "qgsattributeeditorelement.h"
#include "qgis_core.h"

/**
 * \ingroup core
 * \brief An attribute editor widget that will represent a spacer.
 *
 * \since QGIS 3.30
 */
class CORE_EXPORT QgsAttributeEditorSpacerElement : public QgsAttributeEditorElement
{
  public:

    /**
     * Creates a new element which represents a spacer
     *
     * \param name         The name of the widget
     * \param parent       The parent (used as container)
    */
    QgsAttributeEditorSpacerElement( const QString &name, QgsAttributeEditorElement *parent )
      : QgsAttributeEditorElement( Qgis::AttributeEditorType::SpacerElement, name, parent )
    {}

    QgsAttributeEditorElement *clone( QgsAttributeEditorElement *parent ) const override SIP_FACTORY;

    /**
     * Returns TRUE if the spacer element will contain an horizontal line.
     */
    bool drawLine() const;

    /**
     * Sets a flag to define if the spacer element will contain an horizontal line.
     * \param drawLine flag status
     */
    void setDrawLine( bool drawLine );

  private:
    void saveConfiguration( QDomElement &elem, QDomDocument &doc ) const override;
    void loadConfiguration( const QDomElement &element, const QString &layerId, const QgsReadWriteContext &context, const QgsFields &fields ) override;
    QString typeIdentifier() const override;
    bool mDrawLine = false;
};

#endif // QGSATTRIBUTEEDITORSPACERELEMENT_H
