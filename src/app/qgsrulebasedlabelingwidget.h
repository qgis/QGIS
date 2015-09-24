#ifndef QGSRULEBASEDLABELINGWIDGET_H
#define QGSRULEBASEDLABELINGWIDGET_H

#include <QWidget>

#include <ui_qgsrulebasedlabelingwidget.h>

#include "qgsrulebasedlabeling.h"

class QgsMapCanvas;
class QgsVectorLayer;


class APP_EXPORT QgsRuleBasedLabelingModel : public QAbstractItemModel
{
    Q_OBJECT

  public:
    QgsRuleBasedLabelingModel( QgsRuleBasedLabeling::Rule* rootRule, QObject* parent = 0 );

    virtual Qt::ItemFlags flags( const QModelIndex &index ) const override;
    virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    virtual QVariant headerData( int section, Qt::Orientation orientation,
                                 int role = Qt::DisplayRole ) const override;
    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    virtual int columnCount( const QModelIndex & = QModelIndex() ) const override;
    //! provide model index for parent's child item
    virtual QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    //! provide parent model index
    virtual QModelIndex parent( const QModelIndex &index ) const override;

    // editing support
    virtual bool setData( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole ) override;

    // drag'n'drop support
    Qt::DropActions supportedDropActions() const override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData( const QModelIndexList &indexes ) const override;
    bool dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent ) override;

    bool removeRows( int row, int count, const QModelIndex & parent = QModelIndex() ) override;

    // new methods

    QgsRuleBasedLabeling::Rule* ruleForIndex( const QModelIndex& index ) const;

    void insertRule( const QModelIndex& parent, int before, QgsRuleBasedLabeling::Rule* newrule );
    void updateRule( const QModelIndex& parent, int row );
    // update rule and all its descendants
    void updateRule( const QModelIndex& index );
    void removeRule( const QModelIndex& index );

    void willAddRules( const QModelIndex& parent, int count ); // call beginInsertRows
    void finishedAddingRules(); // call endInsertRows

  protected:
    QgsRuleBasedLabeling::Rule* mRootRule;
};



class QgsRuleBasedLabelingWidget : public QWidget, private Ui::QgsRuleBasedLabelingWidget
{
    Q_OBJECT
  public:
    QgsRuleBasedLabelingWidget( QgsVectorLayer* layer, QgsMapCanvas* canvas, QWidget* parent = 0 );
    ~QgsRuleBasedLabelingWidget();

    //! save config to layer
    void writeSettingsToLayer();

  signals:

  protected slots:
    void addRule();
    void editRule();
    void editRule( const QModelIndex& index );
    void removeRule();
    void copy();
    void paste();

  protected:
    QgsRuleBasedLabeling::Rule* currentRule();

  protected:
    QgsVectorLayer* mLayer;
    QgsMapCanvas* mCanvas;

    QgsRuleBasedLabeling::Rule* mRootRule;
    QgsRuleBasedLabelingModel* mModel;

    QAction* mCopyAction;
    QAction* mPasteAction;
    QAction* mDeleteAction;
};


//////

class QgsLabelingGui;

#include "ui_qgslabelingrulepropsdialog.h"

class APP_EXPORT QgsLabelingRulePropsDialog : public QDialog, private Ui::QgsLabelingRulePropsDialog
{
    Q_OBJECT

  public:
    QgsLabelingRulePropsDialog( QgsRuleBasedLabeling::Rule* rule, QgsVectorLayer* layer, QWidget* parent = 0, QgsMapCanvas* mapCanvas = 0 );
    ~QgsLabelingRulePropsDialog();

    QgsRuleBasedLabeling::Rule* rule() { return mRule; }

  public slots:
    void testFilter();
    void buildExpression();
    void accept() override;

  protected:
    QgsRuleBasedLabeling::Rule* mRule; // borrowed
    QgsVectorLayer* mLayer;

    QgsLabelingGui* mLabelingGui;
    QgsPalLayerSettings* mSettings; // a clone of original settings

    QgsMapCanvas* mMapCanvas;
};


#endif // QGSRULEBASEDLABELINGWIDGET_H
