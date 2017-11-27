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

#include "qgssymbolwidgetcontext.h"
#include "qgssymbollayer.h"

#include <QWidget>
#include "qgis_gui.h"

class QgsSymbol;
class QgsStyle;

class QMenu;

/**
 * \ingroup gui
 * \class QgsSymbolsListWidget
 */
class GUI_EXPORT QgsSymbolsListWidget : public QWidget, private Ui::SymbolsListWidget, private QgsExpressionContextGenerator
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsSymbolsListWidget.
     * \param symbol the symbol
     * \param style the style
     * \param menu the menu where to show it
     * \param parent parent widget
     * \param layer associated vector layer
     */
    QgsSymbolsListWidget( QgsSymbol *symbol, QgsStyle *style, QMenu *menu, QWidget *parent SIP_TRANSFERTHIS, QgsVectorLayer *layer = nullptr );


    virtual ~QgsSymbolsListWidget();

    /**
     * Sets the context in which the symbol widget is shown, e.g., the associated map canvas and expression contexts.
     * \param context symbol widget context
     * \see context()
     * \since QGIS 3.0
     */
    void setContext( const QgsSymbolWidgetContext &context );

    /**
     * Returns the context in which the symbol widget is shown, e.g., the associated map canvas and expression contexts.
     * \see setContext()
     * \since QGIS 3.0
     */
    QgsSymbolWidgetContext context() const;

    /**
     * Returns the vector layer associated with the widget.
     * \since QGIS 2.12
     */
    const QgsVectorLayer *layer() const { return mLayer; }

  public slots:

    void setSymbolFromStyle( const QModelIndex &index );
    void setSymbolColor( const QColor &color );
    void setMarkerAngle( double angle );
    void setMarkerSize( double size );
    void setLineWidth( double width );
    void addSymbolToStyle();
    void saveSymbol();

    void symbolAddedToStyle( const QString &name, QgsSymbol *symbol );

    //! Pupulates the groups combo box with available tags and smartgroups
    void populateGroups();

    void openStyleManager();
    void clipFeaturesToggled( bool checked );

    void updateDataDefinedMarkerSize();
    void updateDataDefinedMarkerAngle();
    void updateDataDefinedLineWidth();

  signals:
    void changed();

  private slots:
    void mSymbolUnitWidget_changed();
    void groupsCombo_currentIndexChanged( int index );
    void updateAssistantSymbol();
    void opacityChanged( double value );
    void createAuxiliaryField();

  private:
    QgsSymbol *mSymbol = nullptr;
    std::shared_ptr< QgsSymbol > mAssistantSymbol;
    QgsStyle *mStyle = nullptr;
    QMenu *mAdvancedMenu = nullptr;
    QAction *mClipFeaturesAction = nullptr;
    QgsVectorLayer *mLayer = nullptr;
    QgsMapCanvas *mMapCanvas = nullptr;

    void populateSymbolView();
    void populateSymbols( const QStringList &symbols );
    void updateSymbolColor();
    void updateSymbolInfo();
    QgsSymbolWidgetContext mContext;

    QgsExpressionContext createExpressionContext() const override;
    void registerDataDefinedButton( QgsPropertyOverrideButton *button, QgsSymbolLayer::Property key );
};

#endif //QGSSYMBOLSLISTWIDGET_H



