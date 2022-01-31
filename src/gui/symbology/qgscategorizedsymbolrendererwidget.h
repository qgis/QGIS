/***************************************************************************
    qgscategorizedsymbolrendererwidget.h
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
#ifndef QGSCATEGORIZEDSYMBOLRENDERERWIDGET_H
#define QGSCATEGORIZEDSYMBOLRENDERERWIDGET_H

#include "qgscategorizedsymbolrenderer.h"
#include "qgis_sip.h"
#include "qgsrendererwidget.h"
#include "qgsproxystyle.h"
#include <QStandardItem>
#include <QStyledItemDelegate>


class QgsCategorizedSymbolRenderer;
class QgsRendererCategory;

#include "ui_qgscategorizedsymbolrendererwidget.h"
#include "qgis_gui.h"


#ifndef SIP_RUN
///@cond PRIVATE

class GUI_EXPORT QgsCategorizedSymbolRendererModel : public QAbstractItemModel
{
    Q_OBJECT
  public:
    QgsCategorizedSymbolRendererModel( QObject *parent = nullptr );
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

    void setRenderer( QgsCategorizedSymbolRenderer *renderer );

    void addCategory( const QgsRendererCategory &cat );
    QgsRendererCategory category( const QModelIndex &index );
    void deleteRows( QList<int> rows );
    void removeAllRows();
    void sort( int column, Qt::SortOrder order = Qt::AscendingOrder ) override;
    void updateSymbology();

  signals:
    void rowsMoved();

    /**
     * Signals emitted when a modified key is held and the state is toggled.
     * 
     * \since QGIS 3.28
     */
    void toggleSelectedSymbols( const bool state );

  private:
    QgsCategorizedSymbolRenderer *mRenderer = nullptr;
    QString mMimeFormat;
};

/**
 * \ingroup gui
 * \brief View style which shows drop indicator line between items
 */
class QgsCategorizedSymbolRendererViewStyle: public QgsProxyStyle
{
    Q_OBJECT

  public:
    explicit QgsCategorizedSymbolRendererViewStyle( QWidget *parent );

    void drawPrimitive( PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = nullptr ) const override;
};

/**
 * \ingroup gui
 * \brief Custom delegate for localized numeric input.
 */
class QgsCategorizedRendererViewItemDelegate: public QStyledItemDelegate
{
    Q_OBJECT

  public:
    explicit QgsCategorizedRendererViewItemDelegate( QgsFieldExpressionWidget *expressionWidget, QObject *parent = nullptr );

    // QAbstractItemDelegate interface
    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;

  private:

    QgsFieldExpressionWidget *mFieldExpressionWidget = nullptr;
};


///@endcond

#endif

/**
 * \ingroup gui
 * \class QgsCategorizedSymbolRendererWidget
 */
class GUI_EXPORT QgsCategorizedSymbolRendererWidget : public QgsRendererWidget, private Ui::QgsCategorizedSymbolRendererWidget, private QgsExpressionContextGenerator
{
    Q_OBJECT
  public:

    /**
     * CustomRoles enum represent custom roles for the widget.
     * \since QGIS 3.22.1
     */
    enum CustomRoles
    {
      ValueRole = Qt::UserRole + 1 //!< Category value
    };

    static QgsRendererWidget *create( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer ) SIP_FACTORY;

    QgsCategorizedSymbolRendererWidget( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer );
    ~QgsCategorizedSymbolRendererWidget() override;

    QgsFeatureRenderer *renderer() override;
    void setContext( const QgsSymbolWidgetContext &context ) override;
    void disableSymbolLevels() override SIP_SKIP;

    /**
     * Replaces category symbols with the symbols from a style that have a matching
     * name.
     * \param style style containing symbols to match with
     * \returns number of symbols matched
     * \see matchToSymbolsFromLibrary
     * \see matchToSymbolsFromXml
     * \since QGIS 2.9
     */
    int matchToSymbols( QgsStyle *style );

  public slots:
    void changeCategorizedSymbol();
    void categoryColumnChanged( const QString &field );
    void categoriesDoubleClicked( const QModelIndex &idx );
    void addCategory();
    void addCategories();

    /**
     * Applies the color ramp passed on by the color ramp button
     */
    void applyColorRamp();

    void deleteCategories();
    void deleteAllCategories();

    void showSymbolLevels();

    void rowsMoved();

    /**
     * Replaces category symbols with the symbols from the users' symbol library that have a
     * matching name.
     * \see matchToSymbolsFromXml
     * \see matchToSymbols
     * \since QGIS 2.9
     */
    void matchToSymbolsFromLibrary();

    /**
     * Prompts for selection of an xml file, then replaces category symbols with the symbols
     * from the XML file with a matching name.
     * \see matchToSymbolsFromLibrary
     * \see matchToSymbols
     * \since QGIS 2.9
     */
    void matchToSymbolsFromXml();

  protected:
    void setSymbolLevels( const QgsLegendSymbolList &levels, bool enabled ) override;

  protected slots:

    void pasteSymbolToSelection() override;

  private slots:

    void cleanUpSymbolSelector( QgsPanelWidget *container );
    void updateSymbolsFromWidget();
    void updateSymbolsFromButton();
    void dataDefinedSizeLegend();

    /**
     * Merges all selected categories into a single multi-value category.
     *
     * \see unmergeSelectedCategories()
     */
    void mergeSelectedCategories();

    /**
     * Unmerges all selected multi-value categories into a individual value categories.
     *
     * \see mergeSelectedCategories()
     */
    void unmergeSelectedCategories();

    void showContextMenu( QPoint p );

    void selectionChanged( const QItemSelection &selected, const QItemSelection &deselected );

    /**
     * Slot used to change the state of all selected items.
     * 
     * \since QGIS 3.28
     */
    void toggleSelectedSymbols( const bool state );

  protected:

    void updateUiFromRenderer();

    // Called by virtual refreshSymbolView()
    void populateCategories();

    //! Returns row index for the currently selected category (-1 if on no selection)
    int currentCategoryRow();

    //! Returns a list of indexes for the categories under selection
    QList<int> selectedCategories();

    //! Changes the selected symbols alone for the change button, if there is a selection
    void changeSelectedSymbols();

    void changeCategorySymbol();
    //! Applies current symbol to selected categories, or to all categories if none is selected
    void applyChangeToSymbol();

    QList<QgsSymbol *> selectedSymbols() override;
    QgsCategoryList selectedCategoryList();
    void refreshSymbolView() override;
    void keyPressEvent( QKeyEvent *event ) override;

  protected:
    std::unique_ptr< QgsCategorizedSymbolRenderer > mRenderer;

    std::unique_ptr< QgsSymbol > mCategorizedSymbol;

    QgsCategorizedSymbolRendererModel *mModel = nullptr;

  private:
    QString mOldClassificationAttribute;
    QgsCategoryList mCopyBuffer;
    QMenu *mContextMenu = nullptr;
    QAction *mMergeCategoriesAction = nullptr;
    QAction *mUnmergeCategoriesAction = nullptr;
    QAction *mActionLevels = nullptr;

    QgsExpressionContext createExpressionContext() const override;

    friend class TestQgsCategorizedRendererWidget;
};

#endif // QGSCATEGORIZEDSYMBOLRENDERERWIDGET_H
