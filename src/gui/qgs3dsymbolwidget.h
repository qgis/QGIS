/***************************************************************************
  qgs3dsymbolwidget.h
  --------------------------------------
  Date                 : July 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS3DSYMBOLWIDGET_H
#define QGS3DSYMBOLWIDGET_H

#include <QWidget>
#include "qgis_gui.h"
#include "qgis_sip.h"

class QgsAbstract3DSymbol;
class QgsVectorLayer;

/**
 * \ingroup gui
 * \class Qgs3DSymbolWidget
 * Base class for 3D symbol configuration widgets.
 * \since QGIS 3.16
 */
class GUI_EXPORT Qgs3DSymbolWidget : public QWidget
{
    Q_OBJECT

  public:

    /**
     * Constructor for Qgs3DSymbolWidget with the specified \a parent widget.
     */
    Qgs3DSymbolWidget( QWidget *parent );

    /**
     * Sets the \a symbol to show in the widget.
     */
    virtual void setSymbol( const QgsAbstract3DSymbol *symbol, QgsVectorLayer *layer ) = 0;

    /**
     * Return a new instance of the symbol defined by the widget.
     *
     * Caller takes ownership of the returned symbol.
     */
    virtual QgsAbstract3DSymbol *symbol() = 0 SIP_FACTORY;
};

#endif // QGS3DSYMBOLWIDGET_H
