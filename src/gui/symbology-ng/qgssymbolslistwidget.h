/***************************************************************************
    qgssymbolslistwidget.h
    ---------------------
    begin                : June 2012
    copyright            : (C) 2012 by Arunmozhi
    email                : aruntheguy at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSYMBOLSLISTWIDGET_H
#define QGSSYMBOLSLISTWIDGET_H

#include "ui_widget_symbolslist.h"

#include <QWidget>

class QgsSymbolV2;
class QgsStyleV2;

class QMenu;

class GUI_EXPORT QgsSymbolsListWidget : public QWidget, private Ui::SymbolsListWidget
{
    Q_OBJECT

  public:
    QgsSymbolsListWidget( QgsSymbolV2* symbol, QgsStyleV2* style, QMenu* menu, QWidget* parent = NULL );

  public slots:
    void setSymbolFromStyle( const QModelIndex & index );
    void setSymbolColor();
    void setMarkerAngle( double angle );
    void setMarkerSize( double size );
    void setLineWidth( double width );
    void addSymbolToStyle();
    void on_mSymbolUnitComboBox_currentIndexChanged( const QString & text );
    void on_mTransparencySlider_valueChanged( int value );

    void openStyleManager();

  signals:
    void changed();

  protected:
    QgsSymbolV2* mSymbol;
    QgsStyleV2* mStyle;

    void populateSymbolView();
    void updateSymbolColor();
    void updateSymbolInfo();

  private:
    /**Displays alpha value as transparency in mTransparencyLabel*/
    void displayTransparency( double alpha );
};

#endif //QGSSYMBOLSLISTWIDGET_H


