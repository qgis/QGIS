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

/** Search proxy used to filter the QgsExpressionBuilderWidget tree.
  * The default search for a tree model only searches top level this will handle one
  * level down
  */
class QgsExpressionItemSearchProxy : public QSortFilterProxyModel
{
  public:
    QgsExpressionItemSearchProxy() { }

    bool filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const override
    {
      if ( source_parent == qobject_cast<QStandardItemModel*>( sourceModel() )->invisibleRootItem()->index() )
        return true;

      return QSortFilterProxyModel::filterAcceptsRow( source_row, source_parent );
    }
};

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
    }

    QgsExpressionItem( QString label,
                       QString expressionText,
                       QgsExpressionItem::ItemType itemType = ExpressionNode )
        : QStandardItem( label )
    {
      mExpressionText = expressionText;
      mType = itemType;
    }

    QString getExpressionText() { return mExpressionText; }

    /** Get the help text that is associated with this expression item.
      *
      * @return The help text.
      */
    QString getHelpText() {  return mHelpText; }
    /** Set the help text for the current item
      *
      * @note The help text can be set as a html string.
      */
    void setHelpText( QString helpText ) { mHelpText = helpText; }

    /** Get the type of expression item eg header, field, ExpressionNode.
      *
      * @return The QgsExpressionItem::ItemType
      */
    QgsExpressionItem::ItemType getItemType() { return mType; }

  private:
    QString mExpressionText;
    QString mHelpText;
    QgsExpressionItem::ItemType mType;
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

    /** Sets geometry calculator used in distance/area calculations. */
    void setGeomCalculator( const QgsDistanceArea & da );

    /** Gets the expression string that has been set in the expression area.
      * @returns The expression as a string. */
    QString expressionText();

    /** Sets the expression string for the widget */
    void setExpressionText( const QString& expression );

    /** Registers a node item for the expression builder.
      * @param group The group the item will be show in the tree view.  If the group doesn't exsit it will be created.
      * @param label The label that is show to the user for the item in the tree.
      * @param expressionText The text that is inserted into the expression area when the user double clicks on the item.
      * @param helpText The help text that the user will see when item is selected.
      * @param type The type of the expression item.
      */
    void registerItem( QString group, QString label, QString expressionText,
                       QString helpText = "",
                       QgsExpressionItem::ItemType type = QgsExpressionItem::ExpressionNode );

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
    void fillFieldValues( int fieldIndex, int countLimit );
    QString loadFunctionHelp( QgsExpressionItem* functionName );

    /** Formats an expression preview result for display in the widget
     * by truncating the string
     * @param previewString expression preview result to format
     */
    QString formatPreviewString( const QString &previewString ) const;

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

};

#endif // QGSEXPRESSIONBUILDER_H
