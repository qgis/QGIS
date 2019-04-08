/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/symbology/qgsrulebasedrendererwidget.h                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/







class QgsRuleBasedRendererModel : QAbstractItemModel
{
%Docstring
Tree model for the rules:

(invalid)  == root node
+--- top level rule
+--- top level rule
%End

%TypeHeaderCode
#include "qgsrulebasedrendererwidget.h"
%End
  public:

    QgsRuleBasedRendererModel( QgsRuleBasedRenderer *renderer, QObject *parent );
%Docstring
Constructor for QgsRuleBasedRendererModel, for the specified ``renderer``.
%End

    virtual Qt::ItemFlags flags( const QModelIndex &index ) const;

    virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;

     virtual QVariant headerData( int section, Qt::Orientation orientation,
                         int role = Qt::DisplayRole ) const;
    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const;

    virtual int columnCount( const QModelIndex & = QModelIndex() ) const;

    virtual QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const;

    virtual QModelIndex parent( const QModelIndex &index ) const;


    virtual bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole );


    virtual Qt::DropActions supportedDropActions() const;

    virtual QStringList mimeTypes() const;

    virtual QMimeData *mimeData( const QModelIndexList &indexes ) const;

    virtual bool dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent );


    virtual bool removeRows( int row, int count, const QModelIndex &parent = QModelIndex() );



    QgsRuleBasedRenderer::Rule *ruleForIndex( const QModelIndex &index ) const;

    void insertRule( const QModelIndex &parent, int before, QgsRuleBasedRenderer::Rule *newrule );
    void updateRule( const QModelIndex &parent, int row );
    void updateRule( const QModelIndex &index );
    void removeRule( const QModelIndex &index );

    void willAddRules( const QModelIndex &parent, int count ); // call beginInsertRows
    void finishedAddingRules(); // call endInsertRows

    void clearFeatureCounts();

  protected:
};




class QgsRuleBasedRendererWidget : QgsRendererWidget
{

%TypeHeaderCode
#include "qgsrulebasedrendererwidget.h"
%End
  public:

    static QgsRendererWidget *create( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer ) /Factory/;

    QgsRuleBasedRendererWidget( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer );
    ~QgsRuleBasedRendererWidget();

    virtual QgsFeatureRenderer *renderer();


  public slots:

    void addRule();
    void editRule();
    void editRule( const QModelIndex &index );
    void removeRule();
    void countFeatures();
    void clearFeatureCounts();

    void refineRuleScales();
    void refineRuleCategories();
    void refineRuleRanges();

    void setRenderingOrder();

    void currentRuleChanged( const QModelIndex &current = QModelIndex(), const QModelIndex &previous = QModelIndex() );
    void selectedRulesChanged();

    void saveSectionWidth( int section, int oldSize, int newSize );
    void restoreSectionWidths();

  protected:
    void refineRule( int type );
    void refineRuleCategoriesGui();
%Docstring
Opens the dialog for refining a rule using categories
%End
    void refineRuleRangesGui();
%Docstring
Opens the dialog for refining a rule using ranges
%End
    void refineRuleScalesGui( const QModelIndexList &index );

    QgsRuleBasedRenderer::Rule *currentRule();

    virtual QList<QgsSymbol *> selectedSymbols();

    QgsRuleBasedRenderer::RuleList selectedRules();
    virtual void refreshSymbolView();

    virtual void keyPressEvent( QKeyEvent *event );





  protected slots:
    virtual void copy();

    virtual void paste();


};




class QgsRendererRulePropsWidget : QgsPanelWidget
{

%TypeHeaderCode
#include "qgsrulebasedrendererwidget.h"
%End
  public:

    QgsRendererRulePropsWidget( QgsRuleBasedRenderer::Rule *rule,
                                QgsVectorLayer *layer,
                                QgsStyle *style,
                                QWidget *parent /TransferThis/ = 0,
                                const QgsSymbolWidgetContext &context = QgsSymbolWidgetContext() );
%Docstring
Widget to edit the details of a rule based renderer rule.

:param rule: The rule to edit.
:param layer: The layer used to pull layer related information.
:param style: The active QGIS style.
:param parent: The parent widget.
:param context: the symbol widget context
%End

    QgsRuleBasedRenderer::Rule *rule();
%Docstring
Returns the current set rule.

:return: The current rule.
%End

  public slots:

    void testFilter();
%Docstring
Test the filter that is set in the widget
%End

    void buildExpression();
%Docstring
Open the expression builder widget to check if the
%End

    void apply();
%Docstring
Apply any changes from the widget to the set rule.
%End

    virtual void setDockMode( bool dockMode );

%Docstring
Set the widget in dock mode.

:param dockMode: ``True`` for dock mode.
%End

  protected:


};

class QgsRendererRulePropsDialog : QDialog
{

%TypeHeaderCode
#include "qgsrulebasedrendererwidget.h"
%End
  public:

    QgsRendererRulePropsDialog( QgsRuleBasedRenderer::Rule *rule, QgsVectorLayer *layer, QgsStyle *style, QWidget *parent /TransferThis/ = 0, const QgsSymbolWidgetContext &context = QgsSymbolWidgetContext() );
%Docstring
Constructor for QgsRendererRulePropsDialog

:param rule: associated rule based renderer rule
:param layer: source vector layer
:param style: style collection
:param parent: parent widget
:param context: symbol widget context
%End

    ~QgsRendererRulePropsDialog();

    QgsRuleBasedRenderer::Rule *rule();

  public slots:
    void testFilter();
    void buildExpression();
    virtual void accept();


};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/symbology/qgsrulebasedrendererwidget.h                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
