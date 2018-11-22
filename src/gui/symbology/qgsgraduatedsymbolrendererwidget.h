/***************************************************************************
    qgsgraduatedsymbolrendererwidget.h
    ---------------------
    begin                : December 2009
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

#ifndef QGSGRADUATEDSYMBOLRENDERERWIDGET_H
#define QGSGRADUATEDSYMBOLRENDERERWIDGET_H

#include "qgsgraduatedsymbolrenderer.h"
#include "qgis.h"
#include "qgsrendererwidget.h"
#include "qgsproxystyle.h"
#include <QStandardItem>

#include "ui_qgsgraduatedsymbolrendererwidget.h"
#include "qgis_gui.h"

#ifndef SIP_RUN
/// @cond PRIVATE

class GUI_EXPORT QgsGraduatedSymbolRendererModel : public QAbstractItemModel
{
    Q_OBJECT
  public:
    QgsGraduatedSymbolRendererModel( QObject *parent = nullptr );
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    Qt::DropActions supportedDropActions() const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role ) override;
    QVariant headerData( int section, Qt::Orientation orientation, int role ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex & = QModelIndex() ) const override;
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &index ) const override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData( const QModelIndexList &indexes ) const override;
    bool dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent ) override;

    void setRenderer( QgsGraduatedSymbolRenderer *renderer );

    QgsRendererRange rendererRange( const QModelIndex &index );
    void addClass( QgsSymbol *symbol );
    void addClass( const QgsRendererRange &range );
    void deleteRows( QList<int> rows );
    void removeAllRows();
    void sort( int column, Qt::SortOrder order = Qt::AscendingOrder ) override;
    void updateSymbology( bool resetModel = false );
    void updateLabels();

  signals:
    void rowsMoved();

  private:
    QgsGraduatedSymbolRenderer *mRenderer = nullptr;
    QString mMimeFormat;
};

// View style which shows drop indicator line between items
class QgsGraduatedSymbolRendererViewStyle: public QgsProxyStyle
{
    Q_OBJECT

  public:
    explicit QgsGraduatedSymbolRendererViewStyle( QWidget *parent );

    void drawPrimitive( PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = nullptr ) const override;
};

///@endcond
#endif

/**
 * \ingroup gui
 * \class QgsGraduatedSymbolRendererWidget
 */
class GUI_EXPORT QgsGraduatedSymbolRendererWidget : public QgsRendererWidget, private Ui::QgsGraduatedSymbolRendererWidget, private QgsExpressionContextGenerator
{
    Q_OBJECT

  public:
    static QgsRendererWidget *create( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer ) SIP_FACTORY;

    QgsGraduatedSymbolRendererWidget( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer );
    ~QgsGraduatedSymbolRendererWidget() override;

    QgsFeatureRenderer *renderer() override;

  public slots:
    void changeGraduatedSymbol();
    void graduatedColumnChanged( const QString &field );
    void classifyGraduated();
    void reapplyColorRamp();
    void reapplySizes();
    void rangesDoubleClicked( const QModelIndex &idx );
    void rangesClicked( const QModelIndex &idx );
    void changeCurrentValue( QStandardItem *item );

    //! Adds a class manually to the classification
    void addClass();
    //! Removes currently selected classes
    void deleteClasses();
    //! Removes all classes from the classification
    void deleteAllClasses();
    //! Toggle the link between classes boundaries
    void toggleBoundariesLink( bool linked );

    void labelFormatChanged();

    void showSymbolLevels();

    void rowsMoved();
    void modelDataChanged();
    void refreshRanges( bool reset = false );

  private slots:
    void mSizeUnitWidget_changed();
    void methodComboBox_currentIndexChanged( int );
    void cleanUpSymbolSelector( QgsPanelWidget *container );
    void updateSymbolsFromWidget();
    void toggleMethodWidgets( int idx );
    void dataDefinedSizeLegend();

  protected:
    void updateUiFromRenderer( bool updateCount = true );
    void connectUpdateHandlers();
    void disconnectUpdateHandlers();
    bool rowsOrdered();

    void updateGraduatedSymbolIcon();

    //! Returns a list of indexes for the classes under selection
    QList<int> selectedClasses();
    QgsRangeList selectedRanges();

    void changeRangeSymbol( int rangeIdx );
    void changeRange( int rangeIdx );

    void changeSelectedSymbols();
    //! Applies current symbol to selected ranges, or to all ranges if none is selected
    void applyChangeToSymbol();

    QList<QgsSymbol *> selectedSymbols() override;
    QgsSymbol *findSymbolForRange( double lowerBound, double upperBound, const QgsRangeList &ranges ) const;
    void refreshSymbolView() override;

    void keyPressEvent( QKeyEvent *event ) override;

  private:
    std::unique_ptr< QgsGraduatedSymbolRenderer > mRenderer;

    std::unique_ptr< QgsSymbol > mGraduatedSymbol;

    int mRowSelected;

    QgsGraduatedSymbolRendererModel *mModel = nullptr;

    QgsRangeList mCopyBuffer;

    QgsExpressionContext createExpressionContext() const override;
};


#endif // QGSGRADUATEDSYMBOLRENDERERWIDGET_H
