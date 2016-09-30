class QgsRuleBasedRendererModel : QAbstractItemModel
{
%TypeHeaderCode
#include <qgsrulebasedrendererwidget.h>
%End

  public:
    QgsRuleBasedRendererModel( QgsRuleBasedRenderer* r );

    virtual Qt::ItemFlags flags( const QModelIndex &index ) const;
    virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;
    virtual QVariant headerData( int section, Qt::Orientation orientation,
                                 int role = Qt::DisplayRole ) const;
    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const;
    virtual int columnCount( const QModelIndex & = QModelIndex() ) const;
    //! provide model index for parent's child item
    virtual QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const;
    //! provide parent model index
    virtual QModelIndex parent( const QModelIndex &index ) const;

    // editing support
    virtual bool setData( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );

    // drag'n'drop support
    Qt::DropActions supportedDropActions() const;
    QStringList mimeTypes() const;
    QMimeData *mimeData( const QModelIndexList &indexes ) const;
    bool dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent );

    bool removeRows( int row, int count, const QModelIndex & parent = QModelIndex() );

    // new methods

    QgsRuleBasedRenderer::Rule* ruleForIndex( const QModelIndex& index ) const;

    void insertRule( const QModelIndex& parent, int before, QgsRuleBasedRenderer::Rule* newrule );
    void updateRule( const QModelIndex& parent, int row );
    // update rule and all its descendants
    void updateRule( const QModelIndex& index );
    void removeRule( const QModelIndex& index );

    void willAddRules( const QModelIndex& parent, int count ); // call beginInsertRows
    void finishedAddingRules(); // call endInsertRows

    //! @note not available in python bindungs
    // void setFeatureCounts( QMap<QgsRuleBasedRenderer::Rule*, QgsRuleBasedRendererCount> theCountMap );
    void clearFeatureCounts();

};

class QgsRuleBasedRendererWidget : QgsRendererWidget
{
%TypeHeaderCode
#include <qgsrulebasedrendererwidget.h>
%End

  public:

    static QgsRendererWidget* create( QgsVectorLayer* layer, QgsStyle* style, QgsFeatureRenderer* renderer ) /Factory/;

    QgsRuleBasedRendererWidget( QgsVectorLayer* layer, QgsStyle* style, QgsFeatureRenderer* renderer );
    ~QgsRuleBasedRendererWidget();

    virtual QgsFeatureRenderer* renderer();

  public slots:

    void addRule();
    void editRule();
    void editRule( const QModelIndex& index );
    void removeRule();
    void countFeatures();
    void clearFeatureCounts();

    void refineRuleScales();
    void refineRuleCategories();
    void refineRuleRanges();

    void setRenderingOrder();

    void currentRuleChanged( const QModelIndex& current = QModelIndex(), const QModelIndex& previous = QModelIndex() );
    void selectedRulesChanged();

    void saveSectionWidth( int section, int oldSize, int newSize );
    void restoreSectionWidths();

  protected:
    void refineRule( int type );
    //! Opens the dialog for refining a rule using categories
    void refineRuleCategoriesGui();
    //! Opens the dialog for refining a rule using ranges
    void refineRuleRangesGui();
    void refineRuleScalesGui( const QModelIndexList& index );

    QgsRuleBasedRenderer::Rule* currentRule();

    QList<QgsSymbol*> selectedSymbols();
    QList<QgsRuleBasedRenderer::Rule*> selectedRules();
    void refreshSymbolView();
    void keyPressEvent( QKeyEvent* event );
};

///////

class QgsRendererRulePropsWidget: QgsPanelWidget
{
%TypeHeaderCode
#include <qgsrulebasedrendererwidget.h>
%End

  public:
    /**
       * Widget to edit the details of a rule based renderer rule.
       * @param rule The rule to edit.
       * @param layer The layer used to pull layer related information.
       * @param style The active QGIS style.
       * @param parent The parent widget.
       * @param context the symbol widget context
       */
    QgsRendererRulePropsWidget( QgsRuleBasedRenderer::Rule* rule, QgsVectorLayer* layer, QgsStyle* style, QWidget* parent /TransferThis/ = 0, const QgsSymbolWidgetContext& context= QgsSymbolWidgetContext() );
    ~QgsRendererRulePropsWidget();

    QgsRuleBasedRenderer::Rule* rule();

  public slots:
    void testFilter();
    void buildExpression();
    void apply();
    virtual void setDockMode( bool dockMode);
};


class QgsRendererRulePropsDialog : QDialog
{
%TypeHeaderCode
#include <qgsrulebasedrendererwidget.h>
%End

  public:

    QgsRendererRulePropsDialog( QgsRuleBasedRenderer::Rule* rule, QgsVectorLayer* layer, QgsStyle* style, QWidget* parent /TransferThis/ = 0, const QgsSymbolWidgetContext& context = QgsSymbolWidgetContext() );
    ~QgsRendererRulePropsDialog();

    QgsRuleBasedRenderer::Rule* rule();

  public slots:
    void testFilter();
    void buildExpression();
    void accept();
};
