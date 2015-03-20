/***************************************************************************
    qgsrendererv2widget.h
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSRENDERERV2WIDGET_H
#define QGSRENDERERV2WIDGET_H

#include <QWidget>
#include <QMenu>
#include "qgssymbolv2.h"

class QgsVectorLayer;
class QgsStyleV2;
class QgsFeatureRendererV2;
class QgsSymbolV2SelectorDialog;


/**
  Base class for renderer settings widgets

WORKFLOW:
- open renderer dialog with some RENDERER  (never null!)
- find out which widget to use
- instantiate it and set in stacked widget
- on any change of renderer type, create some default (dummy?) version and change the stacked widget
- when clicked ok/apply, get the renderer from active widget and clone it for the layer
*/
class GUI_EXPORT QgsRendererV2Widget : public QWidget
{
    Q_OBJECT
  public:
    QgsRendererV2Widget( QgsVectorLayer* layer, QgsStyleV2* style );

    virtual ~QgsRendererV2Widget() {}

    //! return pointer to the renderer (no transfer of ownership)
    virtual QgsFeatureRendererV2* renderer() = 0;

    //! show a dialog with renderer's symbol level settings
    void showSymbolLevelsDialog( QgsFeatureRendererV2* r );

  protected:
    QgsVectorLayer* mLayer;
    QgsStyleV2* mStyle;
    QMenu* contextMenu;
    QAction* mCopyAction;
    QAction* mPasteAction;

    /**Subclasses may provide the capability of changing multiple symbols at once by implementing the following two methods
      and by connecting the slot contextMenuViewCategories(const QPoint&)*/
    virtual QList<QgsSymbolV2*> selectedSymbols() { return QList<QgsSymbolV2*>(); }
    virtual void refreshSymbolView() {}

  protected slots:
    void  contextMenuViewCategories( const QPoint& p );
    /**Change color of selected symbols*/
    void changeSymbolColor();
    /**Change opacity of selected symbols*/
    void changeSymbolTransparency();
    /**Change units mm/map units of selected symbols*/
    void changeSymbolUnit();
    /**Change line widths of selected symbols*/
    void changeSymbolWidth();
    /**Change marker sizes of selected symbols*/
    void changeSymbolSize();
    /**Change marker angles of selected symbols*/
    void changeSymbolAngle();

    virtual void copy() {}
    virtual void paste() {}

};


////////////

#include "ui_widget_en_masse_value.h"
#include "qgssizescalewidget.h"

/**
Utility classes for "en masse" size definition
*/
class QgsEnMasseValueDialog : public QDialog, public Ui::QgsEnMasseValueDialog
{
    Q_OBJECT

  public:
    /** Constructor
     * @param symbolList must not be empty
     * @param layer must not be null
     */
    QgsEnMasseValueDialog( const QList<QgsSymbolV2*>& symbolList, QgsVectorLayer * layer, const QString & label );
    virtual ~QgsEnMasseValueDialog() {};

  public slots:
    void setSymbolExpression( const QString & definition );
    void setActiveSymbolExpression( bool active );

  protected:
    QgsDataDefined symbolExpression() const;
    void init( const QString & description ); // needed in children ctor to call virtual

    virtual QgsDataDefined expression( const QgsSymbolV2 * ) const = 0;
    virtual double value( const QgsSymbolV2 * ) const = 0;
    virtual void setExpression( QgsSymbolV2 * symbol, const QString & exprStr ) = 0;

    QList<QgsSymbolV2*>  mSymbolList;
    QgsVectorLayer * mLayer;
};

class QgsEnMasseSizeDialog : public QgsEnMasseValueDialog
{
    Q_OBJECT
  public:
    QgsEnMasseSizeDialog( const QList<QgsSymbolV2*>& symbolList, QgsVectorLayer * layer )
        : QgsEnMasseValueDialog( symbolList, layer, tr( "Size" ) )
    {
      init( tr( "En masse size expression" ) );
      if ( symbolList.length() )
        mDDBtn->setAssistant( new QgsSizeScaleWidget( mLayer, static_cast<const QgsMarkerSymbolV2*>( symbolList[0] ) ) );

    }

  protected:
    QgsDataDefined expression( const QgsSymbolV2 * symbol ) const { return QgsDataDefined( static_cast<const QgsMarkerSymbolV2*>( symbol )->sizeExpression() ); }

    double value( const QgsSymbolV2 * symbol ) const { return static_cast<const QgsMarkerSymbolV2*>( symbol )->size(); }

    void setExpression( QgsSymbolV2 * symbol, const QString & exprStr ) { static_cast<QgsMarkerSymbolV2*>( symbol )->setSizeExpression( exprStr ); }
};

class QgsEnMasseRotationDialog : public QgsEnMasseValueDialog
{
    Q_OBJECT
  public:
    QgsEnMasseRotationDialog( const QList<QgsSymbolV2*>& symbolList, QgsVectorLayer * layer )
        : QgsEnMasseValueDialog( symbolList, layer, tr( "Rotation" ) )
    {
      init( tr( "En masse rotation expression" ) );
    }

  protected:
    QgsDataDefined expression( const QgsSymbolV2 * symbol ) const { return QgsDataDefined( static_cast<const QgsMarkerSymbolV2*>( symbol )->angleExpression() ); }

    double value( const QgsSymbolV2 * symbol ) const { return static_cast<const QgsMarkerSymbolV2*>( symbol )->angle(); }

    void setExpression( QgsSymbolV2 * symbol, const QString & exprStr ) { static_cast<QgsMarkerSymbolV2*>( symbol )->setAngleExpression( exprStr ); }
};


class QgsEnMasseWidthDialog : public QgsEnMasseValueDialog
{
    Q_OBJECT
  public:
    QgsEnMasseWidthDialog( const QList<QgsSymbolV2*>& symbolList, QgsVectorLayer * layer )
        : QgsEnMasseValueDialog( symbolList, layer, tr( "Width" ) )
    {
      init( tr( "En masse width expression" ) );
    }

  protected:
    QgsDataDefined expression( const QgsSymbolV2 * symbol ) const { return QgsDataDefined( static_cast<const QgsLineSymbolV2*>( symbol )->widthExpression() ); }

    double value( const QgsSymbolV2 * symbol ) const { return static_cast<const QgsLineSymbolV2*>( symbol )->width(); }

    void setExpression( QgsSymbolV2 * symbol, const QString & exprStr ) { static_cast<QgsLineSymbolV2*>( symbol )->setWidthExpression( exprStr ); }
};



#endif // QGSRENDERERV2WIDGET_H
