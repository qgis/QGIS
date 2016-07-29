class QgsRuleBasedRendererV2Model : QAbstractItemModel
{
%TypeHeaderCode
#include <qgsrulebasedrendererv2widget.h>
%End

  public:
    QgsRuleBasedRendererV2Model( QgsRuleBasedRendererV2* r );

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

    QgsRuleBasedRendererV2::Rule* ruleForIndex( const QModelIndex& index ) const;

    void insertRule( const QModelIndex& parent, int before, QgsRuleBasedRendererV2::Rule* newrule );
    void updateRule( const QModelIndex& parent, int row );
    // update rule and all its descendants
    void updateRule( const QModelIndex& index );
    void removeRule( const QModelIndex& index );

    void willAddRules( const QModelIndex& parent, int count ); // call beginInsertRows
    void finishedAddingRules(); // call endInsertRows

    //! @note not available in python bindungs
    // void setFeatureCounts( QMap<QgsRuleBasedRendererV2::Rule*, QgsRuleBasedRendererV2Count> theCountMap );
    void clearFeatureCounts();

};

class QgsRuleBasedRendererV2Widget : QgsRendererV2Widget
{
%TypeHeaderCode
#include <qgsrulebasedrendererv2widget.h>
%End

  public:

    static QgsRendererV2Widget* create( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer ) /Factory/;

    QgsRuleBasedRendererV2Widget( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer );
    ~QgsRuleBasedRendererV2Widget();

    virtual QgsFeatureRendererV2* renderer();

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
    void refineRuleCategoriesGui( const QModelIndexList& index );
    void refineRuleRangesGui( const QModelIndexList& index );
    void refineRuleScalesGui( const QModelIndexList& index );

    QgsRuleBasedRendererV2::Rule* currentRule();

    QList<QgsSymbolV2*> selectedSymbols();
    QList<QgsRuleBasedRendererV2::Rule*> selectedRules();
    void refreshSymbolView();
    void keyPressEvent( QKeyEvent* event );
};

///////

class QgsRendererRulePropsWidget: QgsPanelWidget
{
%TypeHeaderCode
#include <qgsrulebasedrendererv2widget.h>
%End

  public:
    QgsRendererRulePropsWidget( QgsRuleBasedRendererV2::Rule* rule, QgsVectorLayer* layer, QgsStyleV2* style, QWidget* parent /TransferThis/ = 0, QgsMapCanvas* mapCanvas = 0 );
    ~QgsRendererRulePropsWidget();

    QgsRuleBasedRendererV2::Rule* rule();

  public slots:
    void testFilter();
    void buildExpression();
    void apply();
    virtual void setDockMode( bool dockMode);
};


class QgsRendererRulePropsDialog : QDialog
{
%TypeHeaderCode
#include <qgsrulebasedrendererv2widget.h>
%End

  public:
    QgsRendererRulePropsDialog( QgsRuleBasedRendererV2::Rule* rule, QgsVectorLayer* layer, QgsStyleV2* style, QWidget* parent /TransferThis/ = 0, QgsMapCanvas* mapCanvas = 0 );
    ~QgsRendererRulePropsDialog();

    QgsRuleBasedRendererV2::Rule* rule();

  public slots:
    void testFilter();
    void buildExpression();
    void accept();
};
