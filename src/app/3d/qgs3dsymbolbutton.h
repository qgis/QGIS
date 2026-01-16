/***************************************************************************
  qgs3dsymbolbutton.h
  --------------------------------------
  Date                 : January 2026
  Copyright            : (C) 2026 by Jean Felder
  Email                : jean dot felder at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS3DSYMBOLBUTTON_H
#define QGS3DSYMBOLBUTTON_H

#include <memory>

#include "qgsabstract3dsymbol.h"
#include "qgsvectorlayer.h"

#include <QToolButton>

class QgsSingleSymbol3DRendererWidget;


/**
 * \class Qgs3DSymbolButton
 * \brief A push button widget for selecting and displaying a 3D symbol.
 * \since QGIS 4.0
 */
class Qgs3DSymbolButton : public QToolButton
{
    Q_OBJECT

  public:
    /**
     * Constructor for Qgs3DSymbolButton
     *
     * \param parent parent widget
     */
    Qgs3DSymbolButton( QWidget *parent );


    /**
     * Sets the button's symbol.
     *
     * \param symbol 3D symbol to represent
     * \see symbol()
     */
    void setSymbol( std::unique_ptr<QgsAbstract3DSymbol> symbol );

    /**
    * Returns the current symbol defined by the button.
    * \see setSymbol()
    */
    QgsAbstract3DSymbol *symbol() const;

    /**
   * Sets a \a layer to associate with the widget. This allows the
   * widget to setup layer related settings within the symbol settings dialog.
   *
   * \param layer layer to associate. Ownership of \a layer is not transferred.
   */
    void setLayer( QgsVectorLayer *layer );

    /**
   * Sets the \a title for the symbol settings dialog window.
   *
   * \param title dialog title
   */
    void setDialogTitle( const QString &title );

  signals:
    /**
   * Emitted when the symbol's settings are changed.
   */
    void changed();

  protected:
    void resizeEvent( QResizeEvent *event ) override;

  private slots:
    void showSettingsDialog();
    void updateSymbolFromWidget( QgsSingleSymbol3DRendererWidget *widget );

  private:
    void updatePreview();

  private:
    std::unique_ptr<QgsAbstract3DSymbol> mSymbol = nullptr;
    QPointer<QgsVectorLayer> mLayer;
    QString mDialogTitle;
};

#endif // QGS3DSYMBOLBUTTON_H
