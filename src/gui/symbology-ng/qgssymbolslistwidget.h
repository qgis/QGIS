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
    QgsSymbolsListWidget( QgsSymbolV2* symbol, QgsStyleV2* style, QMenu* menu, QWidget* parent, const QgsVectorLayer * layer = 0 );

  public slots:
    void setSymbolFromStyle( const QModelIndex & index );
    void setSymbolColor( const QColor& color );
    void setMarkerAngle( double angle );
    void setMarkerSize( double size );
    void setLineWidth( double width );
    void addSymbolToStyle();
    void symbolAddedToStyle( QString name, QgsSymbolV2* symbol );
    void on_mSymbolUnitWidget_changed();
    void on_mTransparencySlider_valueChanged( int value );

    void on_groupsCombo_currentIndexChanged( int index );
    void on_groupsCombo_editTextChanged( const QString &text );

    void openStyleManager();
    void clipFeaturesToggled( bool checked );

    void updateDataDefinedMarkerSize();
    void updateDataDefinedMarkerAngle();
    void updateDataDefinedLineWidth();

  signals:
    void changed();

  protected:
    QgsSymbolV2* mSymbol;
    QgsStyleV2* mStyle;
    QMenu* mAdvancedMenu;
    QAction* mClipFeaturesAction;
    const QgsVectorLayer* mLayer;

    void populateSymbolView();
    void populateSymbols( QStringList symbols );
    void updateSymbolColor();
    void updateSymbolInfo();

  private:
    /**Displays alpha value as transparency in mTransparencyLabel*/
    void displayTransparency( double alpha );
    /** Recursive function to create the group tree in the widget */
    void populateGroups( QString parent = "", QString prepend = "" );
};

#endif //QGSSYMBOLSLISTWIDGET_H



