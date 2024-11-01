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
#include <QDialog>
#include "qgis_gui.h"
#include "qgis_sip.h"

class QgsAbstract3DSymbol;
class QgsVectorLayer;
class QDialogButtonBox;

/**
 * \ingroup gui
 * \class Qgs3DSymbolWidget
 * \brief Base class for 3D symbol configuration widgets.
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
     * Returns a new instance of the symbol defined by the widget.
     *
     * Caller takes ownership of the returned symbol.
     */
    virtual QgsAbstract3DSymbol *symbol() = 0 SIP_FACTORY;

    /**
     * Returns the symbol type handled by the widget.
     */
    virtual QString symbolType() const = 0;

  signals:

    /**
     * Emitted when the symbol is changed.
     */
    void changed();
};


/**
 * \ingroup gui
 * \brief A dialog for configuring a 3D symbol.
 * \since QGIS 3.16
 */
class GUI_EXPORT Qgs3DSymbolDialog : public QDialog
{
    Q_OBJECT

  public:
    /**
     * Constructor for Qgs3DSymbolDialog, initially showing the specified \a symbol.
     */
    Qgs3DSymbolDialog( const QgsAbstract3DSymbol *symbol, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns a new instance of the symbol defined by the dialog.
     *
     * Caller takes ownership of the returned symbol.
     */
    QgsAbstract3DSymbol *symbol() const SIP_FACTORY;

    /**
     * Returns a reference to the dialog's button box.
     */
    QDialogButtonBox *buttonBox() const;

  private:
    Qgs3DSymbolWidget *mWidget = nullptr;
    QDialogButtonBox *mButtonBox = nullptr;
};

#endif // QGS3DSYMBOLWIDGET_H
