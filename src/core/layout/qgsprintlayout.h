/***************************************************************************
                              qgsprintlayout.h
                             -------------------
    begin                : December 2017
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
#ifndef QGSPRINTLAYOUT_H
#define QGSPRINTLAYOUT_H

#include "qgis_core.h"
#include "qgslayout.h"

class QgsLayoutAtlas;

/**
 * \ingroup core
 * \class QgsPrintLayout
 * \brief Print layout, a QgsLayout subclass for static or atlas-based layouts.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsPrintLayout : public QgsLayout, public QgsMasterLayoutInterface
{
    Q_OBJECT
    Q_PROPERTY( QString name READ name WRITE setName NOTIFY nameChanged )

  public:

    /**
     * Constructor for QgsPrintLayout.
     */
    QgsPrintLayout( QgsProject *project );

    QgsPrintLayout *clone() const override SIP_FACTORY;
    QgsProject *layoutProject() const override;
    QgsMasterLayoutInterface::Type layoutType() const override;
    QIcon icon() const override;

    /**
     * Returns the print layout's atlas.
     */
    QgsLayoutAtlas *atlas();

    QString name() const override { return mName; }
    void setName( const QString &name ) override;

    QDomElement writeXml( QDomDocument &document, const QgsReadWriteContext &context ) const override;
    bool readXml( const QDomElement &layoutElement, const QDomDocument &document, const QgsReadWriteContext &context ) override;

    // QgsLayoutInterface
    QDomElement writeLayoutXml( QDomDocument &document, const QgsReadWriteContext &context ) const override;
    bool readLayoutXml( const QDomElement &layoutElement, const QDomDocument &document, const QgsReadWriteContext &context ) override;
    QgsExpressionContext createExpressionContext() const override;
    void updateSettings() override;

  signals:

    /**
     * Emitted when the layout's name is changed.
     * \see setName()
     */
    void nameChanged( const QString &name );

  private:

    QString mName;
    QgsLayoutAtlas *mAtlas = nullptr;

};

#endif //QGSPRINTLAYOUT_H
