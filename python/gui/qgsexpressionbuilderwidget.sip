/** Search proxy used to filter the QgsExpressionBuilderWidget tree.
  * The default search for a tree model only searches top level this will handle one
  * level down
  */
class QgsExpressionItemSearchProxy : QSortFilterProxyModel
{
%TypeHeaderCode
#include <qgsexpressionbuilderwidget.h>
%End

  public:
    QgsExpressionItemSearchProxy();

    bool filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const;
};

/** An expression item that can be used in the QgsExpressionBuilderWidget tree.
  */
class QgsExpressionItem : QStandardItem
{
%TypeHeaderCode
#include <qgsexpressionbuilderwidget.h>
%End

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
                       QgsExpressionItem::ItemType itemType = ExpressionNode );

    QgsExpressionItem( QString label,
                       QString expressionText,
                       QgsExpressionItem::ItemType itemType = ExpressionNode );

    QString getExpressionText();

    /** Get the help text that is associated with this expression item.
      *
      * @return The help text.
      */
    QString getHelpText();
    /** Set the help text for the current item
      *
      * @note The help text can be set as a html string.
      */
    void setHelpText( QString helpText );

    /** Get the type of expression item eg header, field, ExpressionNode.
      *
      * @return The QgsExpressionItem::ItemType
      */
    QgsExpressionItem::ItemType getItemType();
};

/** A reusable widget that can be used to build a expression string.
  * See QgsExpressionBuilderDialog for exmaple of usage.
  */
class QgsExpressionBuilderWidget : QWidget
{
%TypeHeaderCode
#include <qgsexpressionbuilderwidget.h>
%End

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

  public slots:
    void currentChanged( const QModelIndex &index, const QModelIndex & );
    void on_expressionTree_doubleClicked( const QModelIndex &index );
    void on_txtExpressionString_textChanged();
    void on_txtSearchEdit_textChanged();
    void on_lblPreview_linkActivated( QString link );
    void on_mValueListWidget_itemDoubleClicked( QListWidgetItem* item );
    void operatorButtonClicked();
    void showContextMenu( const QPoint & );
    void loadSampleValues();
    void loadAllValues();

  signals:
    /** Emitted when the user changes the expression in the widget.
      * Users of this widget should connect to this signal to decide if to let the user
      * continue.
      * @param isValid Is true if the expression the user has typed is valid.
      */
    void expressionParsed( bool isValid );
};
