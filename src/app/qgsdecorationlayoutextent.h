/***************************************************************************
                          qgsdecorationlayoutextent.h
                              -------------------
    begin                : May 2017
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
#ifndef QGSDECORATIONLAYOUTEXTENT_H
#define QGSDECORATIONLAYOUTEXTENT_H

#include "qgsdecorationitem.h"

#include <QColor>
#include <QFont>
#include <QObject>
#include "qgis_app.h"
#include "qgssymbol.h"
#include "qgstextrenderer.h"
#include <memory>

class QgsDecorationLayoutExtentDialog;

class APP_EXPORT QgsDecorationLayoutExtent : public QgsDecorationItem
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsDecorationLayoutExtent.
    */
    QgsDecorationLayoutExtent( QObject *parent = nullptr );

    /**
     * Returns the fill symbol used for shading layout extents.
     * \see setSymbol()
     */
    QgsFillSymbol *symbol() const;

    /**
     * Sets the fill \a symbol used for shading layout extents. Ownership of
     * \a symbol is transferred.
     * \see symbol()
     */
    void setSymbol( QgsFillSymbol *symbol SIP_TRANSFER );

    /**
     * Returns the text format for extent labels.
     * \see setTextFormat()
     * \see labelExtents()
     */
    QgsTextFormat textFormat() const { return mTextFormat; }

    /**
     * Sets the text \a format for extent labels.
     * \see textFormat()
     * \see setLabelExtents()
     */
    void setTextFormat( const QgsTextFormat &format ) { mTextFormat = format; }

    /**
     * Returns true if layout extents should be labeled with the name of the associated layout & map.
     * \see setLabelExtents()
     * \see textFormat()
     */
    bool labelExtents() const;

    /**
     * Sets whether layout extents should be labeled with the name of the associated layout & map.
     * \see labelExtents()
     * \see setTextFormat()
     */
    void setLabelExtents( bool labelExtents );

  public slots:
    void projectRead() override;
    void saveToProject() override;
    void run() override;
    void render( const QgsMapSettings &mapSettings, QgsRenderContext &context ) override;

  private:
    std::unique_ptr< QgsFillSymbol > mSymbol;
    QgsTextFormat mTextFormat;
    bool mLabelExtents = true;

    friend class QgsDecorationLayoutExtentDialog;
};

#endif //QGSDECORATIONLAYOUTEXTENT_H
