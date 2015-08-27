/***************************************************************************
    qgisexpressionbuilderwidget.h - A genric expression string builder widget.
     --------------------------------------
    Date                 :  29-May-2011
    Copyright            : (C) 2011 by Nathan Woodrow
    Email                : woodrow.nathan at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSEXPRESSIONBUILDER_H
#define QGSEXPRESSIONBUILDER_H

#include <QWidget>
#include "ui_qgsexpressionbuilder.h"
#include "qgsvectorlayer.h"
#include "qgsexpressionhighlighter.h"
#include "qgsdistancearea.h"

#include "QStandardItemModel"
#include "QStandardItem"
#include "QSortFilterProxyModel"

/** An expression item that can be used in the QgsExpressionBuilderWidget tree.
  */
class QgsExpressionItem : public QStandardItem
{
  public:
    enum ItemType
    {
      Header,
      Field,
      ExpressionNode
    };

    QgsExpressionItem( QString label,
                       QString expressionText,
                       QString helpText,
                       QgsExpressionItem::ItemType itemType = ExpressionNode )
        : QStandardItem( label )
    {
      mExpressionText = expressionText;
      mHelpText = helpText;
      mType = itemType;
      setData( itemType, ItemTypeRole );
    }

    QgsExpressionItem( QString label,
                       QString expressionText,
                       QgsExpressionItem::ItemType itemType = ExpressionNode )
        : QStandardItem( label )
    {
      mExpressionText = expressionText;
      mType = itemType;
      setData( itemType, ItemTypeRole );
    }

    QString getExpressionText() const { return mExpressionText; }

    /** Get the help text that is associated with this expression item.
      *
      * @return The help text.
      */
    QString getHelpText() const { return mHelpText; }
    /** Set the help text for the current item
      *
      * @note The help text can be set as a html string.
      */
    void setHelpText( QString helpText ) { mHelpText = helpText; }

    /** Get the type of expression item eg header, field, ExpressionNode.
      *
      * @return The QgsExpressionItem::ItemType
      */
    QgsExpressionItem::ItemType getItemType() const { return mType; }

    //! Custom sort order role
    static const int CustomSortRole = Qt::UserRole + 1;
    //! Item type role
    static const int ItemTypeRole = Qt::UserRole + 2;

  private:
    QString mExpressionText;
    QString mHelpText;
    QgsExpressionItem::ItemType mType;

};

/** Search proxy used to filter the QgsExpressionBuilderWidget tree.
  * The default search for a tree model only searches top level this will handle one
  * level down
  */
class QgsExpressionItemSearchProxy : public QSortFilterProxyModel
{
  public:
    QgsExpressionItemSearchProxy()
    {
      setFilterCaseSensitivity( Qt::CaseInsensitive );
    }

    bool filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const override
    {
      QModelIndex index = sourceModel()->index( source_row, 0, source_parent );
      QgsExpressionItem::ItemType itemType = QgsExpressionItem::ItemType( sourceModel()->data( index, QgsExpressionItem::ItemTypeRole ).toInt() );

      if ( itemType == QgsExpressionItem::Header )
        return true;

      return QSortFilterProxyModel::filterAcceptsRow( source_row, source_parent );
    }

  protected:

    bool lessThan( const QModelIndex &left, const QModelIndex &right ) const override
    {
      int leftSort = sourceModel()->data( left, QgsExpressionItem::CustomSortRole ).toInt();
      int rightSort = sourceModel()->data( right,  QgsExpressionItem::CustomSortRole ).toInt();
      if ( leftSort != rightSort )
        return leftSort < rightSort;

      QString leftString = sourceModel()->data( left, Qt::DisplayRole ).toString();
      QString rightString = sourceModel()->data( right, Qt::DisplayRole ).toString();

      //ignore $ prefixes when sorting
      if ( leftString.startsWith( "$" ) )
        leftString = leftString.mid( 1 );
      if ( rightString.startsWith( "$" ) )
        rightString = rightString.mid( 1 );

      return QString::localeAwareCompare( leftString, rightString ) < 0;
    }
};

/** A reusable widget that can be used to build a expression string.
  * See QgsExpressionBuilderDialog for exmaple of usage.
  */
class GUI_EXPORT QgsExpressionBuilderWidget : public QWidget, private Ui::QgsExpressionBuilderWidgetBase
{
    Q_OBJECT
  public:
    QgsExpressionBuilderWidget( QWidget *parent );
    ~QgsExpressionBuilderWidget();

    /** Sets layer in order to get the fields and values
      * @note this needs to be called before calling loadFieldNames().
      */
    void setLayer( QgsVectorLayer* layer );

    /** Loads all the field names from the layer.
      * @remarks Should this really be public couldn't we just do this for the user?
      */
    void loadFieldNames();

    void loadFieldNames( const QgsFields& fields );

    /** Loads field names and values from the specified map.
     *  @note The field values must be quoted appropriately if they are strings.
     *  @note added in QGIS 2.12
     */
    void loadFieldsAndValues( const QMap<QString, QStringList>& fieldValues );

    /** Sets geometry calculator used in distance/area calculations. */
    void setGeomCalculator( const QgsDistanceArea & da );

    /** Gets the expression string that has been set in the expression area.
      * @returns The expression as a string. */
    QString expressionText();

    /** Sets the expression string for the widget */
    void setExpressionText( const QString& expression );

    /** Returns the expression context for the widget. The context is used for the expression
     * preview result and for populating the list of available functions and variables.
     * @see setExpressionContext
     * @note added in QGIS 2.12
     */
    QgsExpressionContext expressionContext() const { return mExpressionContext; }

    /** Sets the expression context for the widget. The context is used for the expression
     * preview result and for populating the list of available functions and variables.
     * @param context expression context
     * @see expressionContext
     * @note added in QGIS 2.12
     */
    void setExpressionContext( const QgsExpressionContext& context );

    /** Registers a node item for the expression builder.
      * @param group The group the item will be show in the tree view.  If the group doesn't exsit it will be created.
      * @param label The label that is show to the user for the item in the tree.
      * @param expressionText The text that is inserted into the expression area when the user double clicks on the item.
      * @param helpText The help text that the user will see when item is selected.
      * @param type The type of the expression item.
      * @param highlightedItem set to true to make the item highlighted, which inserts a bold copy of the item at the top level
      * @param sortOrder sort ranking for item
      */
    void registerItem( QString group, QString label, QString expressionText,
                       QString helpText = "",
                       QgsExpressionItem::ItemType type = QgsExpressionItem::ExpressionNode,
                       bool highlightedItem = false, int sortOrder = 1 );

    bool isExpressionValid();

    void saveToRecent( QString key );

    void loadRecent( QString key );

    /** Create a new file in the function editor
     */
    void newFunctionFile( QString fileName = "scratch" );

    /** Save the current function editor text to the given file.
     */
    void saveFunctionFile( QString fileName );

    /** Load code from the given file into the function editor
     */
    void loadCodeFromFile( QString path );

    /** Load code into the function editor
     */
    void loadFunctionCode( QString code );

    /** Update the list of function files found at the given path
     */
    void updateFunctionFileList( QString path );

  public slots:
    void currentChanged( const QModelIndex &index, const QModelIndex & );
    void on_btnRun_pressed();
    void on_btnNewFile_pressed();
    void on_cmbFileNames_currentIndexChanged( int index );
    void on_btnSaveFile_pressed();
    void on_expressionTree_doubleClicked( const QModelIndex &index );
    void on_txtExpressionString_textChanged();
    void on_txtSearchEdit_textChanged();
    void on_lblPreview_linkActivated( QString link );
    void on_mValueListWidget_itemDoubleClicked( QListWidgetItem* item );
    void operatorButtonClicked();
    void showContextMenu( const QPoint & );
    void loadSampleValues();
    void loadAllValues();

  private slots:
    void setExpressionState( bool state );

  signals:
    /** Emitted when the user changes the expression in the widget.
      * Users of this widget should connect to this signal to decide if to let the user
      * continue.
      * @param isValid Is true if the expression the user has typed is valid.
      */
    void expressionParsed( bool isValid );

  private:
    void runPythonCode( QString code );
    void updateFunctionTree();
    void fillFieldValues( const QString &fieldName, int countLimit );
    QString loadFunctionHelp( QgsExpressionItem* functionName );

    /** Formats an expression preview result for display in the widget
     * by truncating the string
     * @param previewString expression preview result to format
     */
    QString formatPreviewString( const QString &previewString ) const;

    void loadExpressionContext();

    QString mFunctionsPath;
    QgsVectorLayer *mLayer;
    QStandardItemModel *mModel;
    QgsExpressionItemSearchProxy *mProxyModel;
    QMap<QString, QgsExpressionItem*> mExpressionGroups;
    QgsFeature mFeature;
    QgsExpressionHighlighter* highlighter;
    bool mExpressionValid;
    QgsDistanceArea mDa;
    QString mRecentKey;
    QMap<QString, QStringList> mFieldValues;
    QgsExpressionContext mExpressionContext;

};

#endif // QGSEXPRESSIONBUILDER_H
