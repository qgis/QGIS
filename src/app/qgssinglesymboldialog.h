/***************************************************************************
                         qgssinglesymboldialog.h  -  description
                             -------------------
    begin                : Oct 2003
    copyright            : (C) 2003 by Marco Hugentobler
    email                : mhugent@geo.unizh.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#ifndef QGSSINGLESYMBOLDIALOG_H
#define QGSSINGLESYMBOLDIALOG_H

#include "ui_qgssinglesymboldialogbase.h"
#include <vector>

class QgsSymbol;
class QgsVectorLayer;


/**QgsSingleSymbolDialog is a dialog to set symbology for the legend type 'single symbol'*/
class QgsSingleSymbolDialog: public QDialog, private Ui::QgsSingleSymbolDialogBase
{
    Q_OBJECT
  public:
    QgsSingleSymbolDialog( QgsVectorLayer* layer, bool disabled = false );
    ~QgsSingleSymbolDialog();
    QColor getOutlineColor();
    Qt::PenStyle getOutlineStyle();
    double getOutlineWidth();
    QColor getFillColor();
    Qt::BrushStyle getFillStyle();
    void setOutlineColor( QColor& c );
    void setOutlineStyle( Qt::PenStyle pstyle );
    void setOutlineWidth( double width );
    void setFillColor( QColor& c );
    void setFillStyle( Qt::BrushStyle fstyle );
    void setLabel( QString label );
    QString label();

  protected:
    QgsVectorLayer* mVectorLayer;
    bool mDisabled;

    // Reimplements dialog keyPress event so we can ignore it
    void keyPressEvent( QKeyEvent * event );

  public slots:
    /* arrange the widgets on this dialog to reflect the current state of QgsSymbol */
    void unset();
    void set( const QgsSymbol *sy );
    void updateSet( const QgsSymbol *sy );
    /**applies the changes to the vector layer*/
    void apply();
    /**applies the changes to the QgsSymbol */
    void apply( QgsSymbol *sy );
    /**emits the signal settingsChanged()*/
    void resendSettingsChanged();
    /**changes the texture selection button to enabled or
     * disabled depending if the texture entry in the combo
     */
    void fillStyleChanged( int theIndex );

    void refreshMarkers();

  protected slots:
    void selectOutlineColor();
    void selectFillColor();
    void selectTextureImage();
    void symbolChanged( QListWidgetItem * current, QListWidgetItem * previous );
  private:
    /** Default constructor is private, do not use this */
    QgsSingleSymbolDialog();
    QString mTexturePath;

  signals:
    void settingsChanged();
};

#endif
