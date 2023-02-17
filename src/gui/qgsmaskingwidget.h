/***************************************************************************
    qgsmaskingwidget.h
    ---------------------
    begin                : September 2019
    copyright            : (C) 2019 by Hugo Mercier
    email                : hugo dot mercier at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSMASKINGWIDGET_H
#define QGSMASKINGWIDGET_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

/// @cond PRIVATE

#include <QPointer>

#include "qgspanelwidget.h"
#include "ui_qgsmaskingwidgetbase.h"
#include "qgsstyleentityvisitor.h"
#include "qgis_sip.h"
#include "qgis_gui.h"

class QgsMessageBarItem;

/**
 * \ingroup gui
 * \brief Main widget for the configuration of mask sources and targets.
 *
 * \note This class is not a part of public API
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsMaskingWidget: public QgsPanelWidget, private Ui::QgsMaskingWidgetBase
{
    Q_OBJECT
  public:
    //! constructor
    QgsMaskingWidget( QWidget *parent = nullptr );

    //! Sets the layer to configure
    void setLayer( QgsVectorLayer *layer );

    //! Applies the changes
    void apply();

    //! Widget has been populated or not
    bool hasBeenPopulated();

  protected:

    void showEvent( QShowEvent * ) override;

  private slots:

    /**
     * Called whenever mask sources or targets selection has changed
     */
    void onSelectionChanged();

  private:
    QgsVectorLayer *mLayer = nullptr;
    //! Populate the mask source and target widgets
    void populate();

    QPointer<QgsMessageBarItem> mMessageBarItem;
    bool mMustPopulate = false;
};


/**
 * \ingroup gui
 * \brief Generic visitor that collects symbol layers of a vector layer's renderer and call a callback function on them with their corresponding QgsSymbolLayerId.
 *
 * \note This class is not a part of public API
 * \since QGIS 3.14
 */
class SymbolLayerVisitor : public QgsStyleEntityVisitorInterface
{
  public:
    typedef std::function<void( const QgsSymbolLayer *, const QgsSymbolLayerId & )> SymbolLayerCallback;

    //! constructor
    SymbolLayerVisitor( SymbolLayerCallback callback );

    bool visitEnter( const QgsStyleEntityVisitorInterface::Node &node ) override;

    //! Process a symbol
    void visitSymbol( const QgsSymbol *symbol, const QString &leafIdentifier, QVector<int> rootPath );

    bool visit( const QgsStyleEntityVisitorInterface::StyleLeaf &leaf ) override;

  private:
    QString mSymbolKey;
    QList<QPair<QgsSymbolLayerId, QList<QgsSymbolLayerReference>>> mMasks;
    SymbolLayerCallback mCallback;
};

/// @endcond private

#endif
