/***************************************************************************
    qgsattributesformview.cpp
    ---------------------
    begin                : June 2025
    copyright            : (C) 2025 by Germán Carrillo
    email                : german at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsattributesformview.h"

#include "qgsapplication.h"
#include "qgsattributesformtreeviewindicator.h"
#include "qgsattributetypedialog.h"
#include "qgscodeeditorexpression.h"
#include "qgscodeeditorhtml.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgsexpressioncontextutils.h"
#include "qgsexpressionfinder.h"
#include "qgsgui.h"
#include "qgshtmlwidgetwrapper.h"
#include "qgsqmlwidgetwrapper.h"
#include "qgsscrollarea.h"
#include "qgstextwidgetwrapper.h"

#include <QAction>
#include <QClipboard>
#include <QDropEvent>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTreeView>
#include <QWidget>

#include "moc_qgsattributesformview.cpp"

QgsAttributesFormBaseView::QgsAttributesFormBaseView( QgsVectorLayer *layer, QWidget *parent )
  : QTreeView( parent )
  , mLayer( layer )
{
}

QModelIndex QgsAttributesFormBaseView::firstSelectedIndex() const
{
  if ( selectionModel()->selectedRows( 0 ).count() == 0 )
    return QModelIndex();

  return mModel->mapToSource( selectionModel()->selectedRows( 0 ).at( 0 ) );
}

QgsExpressionContext QgsAttributesFormBaseView::createExpressionContext() const
{
  QgsExpressionContext expContext;
  expContext << QgsExpressionContextUtils::globalScope()
             << QgsExpressionContextUtils::projectScope( QgsProject::instance() );

  if ( mLayer )
    expContext << QgsExpressionContextUtils::layerScope( mLayer );

  expContext.appendScope( QgsExpressionContextUtils::formScope() );
  return expContext;
}

void QgsAttributesFormBaseView::selectFirstMatchingItem( const QgsAttributesFormData::AttributesFormItemType &itemType, const QString &itemId )
{
  // To be used with Relations, fields and actions
  const auto *model = static_cast< QgsAttributesFormModel * >( mModel->sourceModel() );
  QModelIndex index = mModel->mapFromSource( model->firstRecursiveMatchingModelIndex( itemType, itemId ) );

  if ( index.isValid() )
  {
    // Check if selection is index is already selected (e.g., duplicate fields in
    // form layout, and selecting one after the other), otherwise set new selection
    const QModelIndexList selectedIndexes = selectionModel()->selectedRows( 0 );
    if ( !( selectedIndexes.count() == 1 && selectedIndexes.at( 0 ) == index ) )
    {
      selectionModel()->setCurrentIndex( index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
    }
  }
  else
  {
    selectionModel()->clearSelection();
  }
}

void QgsAttributesFormBaseView::setFilterText( const QString &text )
{
  mModel->setFilterText( text );
}

const QList<QgsAttributesFormTreeViewIndicator *> QgsAttributesFormBaseView::indicators( const QModelIndex &index ) const
{
  QgsAttributesFormItem *item = static_cast< QgsAttributesFormModel *>( mModel->sourceModel() )->itemForIndex( mModel->mapToSource( index ) );
  return mIndicators.value( item );
}

const QList<QgsAttributesFormTreeViewIndicator *> QgsAttributesFormBaseView::indicators( QgsAttributesFormItem *item ) const
{
  return mIndicators.value( item );
}

void QgsAttributesFormBaseView::addIndicator( QgsAttributesFormItem *item, QgsAttributesFormTreeViewIndicator *indicator )
{
  if ( !mIndicators[item].contains( indicator ) )
  {
    mIndicators[item].append( indicator );
    connect( indicator, &QgsAttributesFormTreeViewIndicator::changed, this, [this] {
      update();
      viewport()->repaint();
    } );
    update();
    viewport()->repaint(); //update() does not automatically trigger a repaint()
  }
}

void QgsAttributesFormBaseView::removeIndicator( QgsAttributesFormItem *item, QgsAttributesFormTreeViewIndicator *indicator )
{
  mIndicators[item].removeOne( indicator );
  update();
}

void QgsAttributesFormBaseView::removeAllIndicators()
{
  const QList<QgsAttributesFormItem *> keys = mIndicators.keys();
  for ( QgsAttributesFormItem *key : keys )
  {
    qDeleteAll( mIndicators[key] );
    mIndicators[key].clear();
  }
  mIndicators.clear();
  update();
}

QgsAttributesFormModel *QgsAttributesFormBaseView::sourceModel() const
{
  return mModel->sourceAttributesFormModel();
}


QgsAttributesAvailableWidgetsView::QgsAttributesAvailableWidgetsView( QgsVectorLayer *layer, QWidget *parent )
  : QgsAttributesFormBaseView( layer, parent )
{
}

void QgsAttributesAvailableWidgetsView::setModel( QAbstractItemModel *model )
{
  mModel = qobject_cast<QgsAttributesFormProxyModel *>( model );
  if ( !mModel )
    return;

  QTreeView::setModel( mModel );
}

QgsAttributesAvailableWidgetsModel *QgsAttributesAvailableWidgetsView::availableWidgetsModel() const
{
  return static_cast< QgsAttributesAvailableWidgetsModel * >( mModel->sourceModel() );
}


QgsAttributesFormLayoutView::QgsAttributesFormLayoutView( QgsVectorLayer *layer, QWidget *parent )
  : QgsAttributesFormBaseView( layer, parent )
{
  connect( this, &QTreeView::doubleClicked, this, &QgsAttributesFormLayoutView::onItemDoubleClicked );
}

void QgsAttributesFormLayoutView::setModel( QAbstractItemModel *model )
{
  mModel = qobject_cast<QgsAttributesFormProxyModel *>( model );
  if ( !mModel )
    return;

  QTreeView::setModel( mModel );

  const auto *formLayoutModel = static_cast< QgsAttributesFormLayoutModel * >( mModel->sourceModel() );
  connect( formLayoutModel, &QgsAttributesFormLayoutModel::externalItemDropped, this, &QgsAttributesFormLayoutView::handleExternalDroppedItem );
  connect( formLayoutModel, &QgsAttributesFormLayoutModel::internalItemDropped, this, &QgsAttributesFormLayoutView::handleInternalDroppedItem );
}


void QgsAttributesFormLayoutView::handleExternalDroppedItem( QModelIndex &index )
{
  selectionModel()->setCurrentIndex( mModel->mapFromSource( index ), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );

  const auto itemType = static_cast< QgsAttributesFormData::AttributesFormItemType >( index.data( QgsAttributesFormModel::ItemTypeRole ).toInt() );

  if ( itemType == QgsAttributesFormData::QmlWidget
       || itemType == QgsAttributesFormData::HtmlWidget
       || itemType == QgsAttributesFormData::TextWidget
       || itemType == QgsAttributesFormData::SpacerWidget )
  {
    onItemDoubleClicked( mModel->mapFromSource( index ) );
  }
}

void QgsAttributesFormLayoutView::handleInternalDroppedItem( QModelIndex &index )
{
  selectionModel()->clearCurrentIndex();
  const auto itemType = static_cast< QgsAttributesFormData::AttributesFormItemType >( index.data( QgsAttributesFormModel::ItemTypeRole ).toInt() );
  if ( itemType == QgsAttributesFormData::Container )
  {
    expandRecursively( mModel->mapFromSource( index ) );
  }
}

void QgsAttributesFormLayoutView::dragEnterEvent( QDragEnterEvent *event )
{
  const QMimeData *data = event->mimeData();

  if ( data->hasFormat( u"application/x-qgsattributesformavailablewidgetsrelement"_s )
       || data->hasFormat( u"application/x-qgsattributesformlayoutelement"_s ) )
  {
    // Inner drag and drop actions are always MoveAction
    if ( event->source() == this )
    {
      event->setDropAction( Qt::MoveAction );
    }
  }
  else
  {
    event->ignore();
  }

  QTreeView::dragEnterEvent( event );
}

/**
 * Is called when mouse is moved over attributes tree before a
 * drop event.
 */
void QgsAttributesFormLayoutView::dragMoveEvent( QDragMoveEvent *event )
{
  const QMimeData *data = event->mimeData();

  if ( data->hasFormat( u"application/x-qgsattributesformavailablewidgetsrelement"_s )
       || data->hasFormat( u"application/x-qgsattributesformlayoutelement"_s ) )
  {
    // Inner drag and drop actions are always MoveAction
    if ( event->source() == this )
    {
      event->setDropAction( Qt::MoveAction );
    }
  }
  else
  {
    event->ignore();
  }

  QTreeView::dragMoveEvent( event );
}

void QgsAttributesFormLayoutView::dropEvent( QDropEvent *event )
{
  if ( !( event->mimeData()->hasFormat( u"application/x-qgsattributesformavailablewidgetsrelement"_s )
          || event->mimeData()->hasFormat( u"application/x-qgsattributesformlayoutelement"_s ) ) )
    return;

  if ( event->source() == this )
  {
    event->setDropAction( Qt::MoveAction );
  }

  QTreeView::dropEvent( event );
}

void QgsAttributesFormLayoutView::onItemDoubleClicked( const QModelIndex &index )
{
  QModelIndex sourceIndex = mModel->mapToSource( index );
  QgsAttributesFormData::AttributeFormItemData itemData = sourceIndex.data( QgsAttributesFormModel::ItemDataRole ).value<QgsAttributesFormData::AttributeFormItemData>();
  const auto itemType = static_cast<QgsAttributesFormData::AttributesFormItemType>( sourceIndex.data( QgsAttributesFormModel::ItemTypeRole ).toInt() );
  const QString itemName = sourceIndex.data( QgsAttributesFormModel::ItemNameRole ).toString();

  QGroupBox *baseData = new QGroupBox( tr( "Base configuration" ) );

  QFormLayout *baseLayout = new QFormLayout();
  baseData->setLayout( baseLayout );
  QCheckBox *showLabelCheckbox = new QCheckBox( u"Show label"_s );
  showLabelCheckbox->setChecked( itemData.showLabel() );
  baseLayout->addRow( showLabelCheckbox );
  QWidget *baseWidget = new QWidget();
  baseWidget->setLayout( baseLayout );

  switch ( itemType )
  {
    case QgsAttributesFormData::Action:
    case QgsAttributesFormData::Container:
    case QgsAttributesFormData::WidgetType:
    case QgsAttributesFormData::Relation:
    case QgsAttributesFormData::Field:
      break;

    case QgsAttributesFormData::QmlWidget:
    {
      QDialog dlg;
      dlg.setObjectName( "QML Form Configuration Widget" );
      QgsGui::enableAutoGeometryRestore( &dlg );
      dlg.setWindowTitle( tr( "Configure QML Widget" ) );

      QVBoxLayout *mainLayout = new QVBoxLayout( &dlg );
      QSplitter *qmlSplitter = new QSplitter();
      QWidget *qmlConfigWiget = new QWidget();
      QVBoxLayout *layout = new QVBoxLayout( qmlConfigWiget );
      layout->setContentsMargins( 0, 0, 0, 0 );
      mainLayout->addWidget( qmlSplitter );
      qmlSplitter->addWidget( qmlConfigWiget );
      layout->addWidget( baseWidget );

      QLineEdit *title = new QLineEdit( itemName );

      //qmlCode
      QgsCodeEditor *qmlCode = new QgsCodeEditor( this );
      qmlCode->setEditingTimeoutInterval( 250 );
      qmlCode->setText( itemData.qmlElementEditorConfiguration().qmlCode );

      QgsQmlWidgetWrapper *qmlWrapper = new QgsQmlWidgetWrapper( mLayer, nullptr, this );
      QgsFeature previewFeature;
      mLayer->getFeatures().nextFeature( previewFeature );

      //update preview on text change
      connect( qmlCode, &QgsCodeEditor::editingTimeout, this, [qmlWrapper, qmlCode, previewFeature] {
        qmlWrapper->setQmlCode( qmlCode->text() );
        qmlWrapper->reinitWidget();
        qmlWrapper->setFeature( previewFeature );
      } );

      //templates
      QComboBox *qmlObjectTemplate = new QComboBox();
      qmlObjectTemplate->addItem( tr( "Free Text…" ) );
      qmlObjectTemplate->addItem( tr( "Rectangle" ) );
      qmlObjectTemplate->addItem( tr( "Pie Chart" ) );
      qmlObjectTemplate->addItem( tr( "Bar Chart" ) );
      connect( qmlObjectTemplate, qOverload<int>( &QComboBox::activated ), qmlCode, [qmlCode]( int index ) {
        qmlCode->clear();
        switch ( index )
        {
          case 0:
          {
            qmlCode->setText( QString() );
            break;
          }
          case 1:
          {
            qmlCode->setText( QStringLiteral( "import QtQuick 2.0\n"
                                              "\n"
                                              "Rectangle {\n"
                                              "    width: 100\n"
                                              "    height: 100\n"
                                              "    color: \"steelblue\"\n"
                                              "    Text{ text: \"A rectangle\" }\n"
                                              "}\n" ) );
            break;
          }
          case 2:
          {
            qmlCode->setText( QStringLiteral( "import QtQuick 2.0\n"
                                              "import QtCharts 2.0\n"
                                              "\n"
                                              "ChartView {\n"
                                              "    width: 400\n"
                                              "    height: 400\n"
                                              "\n"
                                              "    PieSeries {\n"
                                              "        id: pieSeries\n"
                                              "        PieSlice { label: \"First slice\"; value: 25 }\n"
                                              "        PieSlice { label: \"Second slice\"; value: 45 }\n"
                                              "        PieSlice { label: \"Third slice\"; value: 30 }\n"
                                              "    }\n"
                                              "}\n" ) );
            break;
          }
          case 3:
          {
            qmlCode->setText( QStringLiteral( "import QtQuick 2.0\n"
                                              "import QtCharts 2.0\n"
                                              "\n"
                                              "ChartView {\n"
                                              "    title: \"Bar series\"\n"
                                              "    width: 600\n"
                                              "    height:400\n"
                                              "    legend.alignment: Qt.AlignBottom\n"
                                              "    antialiasing: true\n"
                                              "    ValueAxis{\n"
                                              "        id: valueAxisY\n"
                                              "        min: 0\n"
                                              "        max: 15\n"
                                              "    }\n"
                                              "\n"
                                              "    BarSeries {\n"
                                              "        id: mySeries\n"
                                              "        axisY: valueAxisY\n"
                                              "        axisX: BarCategoryAxis { categories: [\"2007\", \"2008\", \"2009\", \"2010\", \"2011\", \"2012\" ] }\n"
                                              "        BarSet { label: \"Bob\"; values: [2, 2, 3, 4, 5, 6] }\n"
                                              "        BarSet { label: \"Susan\"; values: [5, 1, 2, 4, 1, 7] }\n"
                                              "        BarSet { label: \"James\"; values: [3, 5, 8, 13, 5, 8] }\n"
                                              "    }\n"
                                              "}\n" ) );
            break;
          }
          default:
            break;
        }
      } );

      QgsFieldExpressionWidget *expressionWidget = new QgsFieldExpressionWidget;
      expressionWidget->setButtonVisible( false );
      expressionWidget->registerExpressionContextGenerator( this );
      expressionWidget->setLayer( mLayer );
      QToolButton *addFieldButton = new QToolButton();
      addFieldButton->setIcon( QgsApplication::getThemeIcon( u"/symbologyAdd.svg"_s ) );

      QToolButton *editExpressionButton = new QToolButton();
      editExpressionButton->setIcon( QgsApplication::getThemeIcon( u"/mIconExpression.svg"_s ) );
      editExpressionButton->setToolTip( tr( "Insert/Edit Expression" ) );

      connect( addFieldButton, &QAbstractButton::clicked, this, [expressionWidget, qmlCode] {
        QString expression = expressionWidget->expression().trimmed().replace( '"', "\\\""_L1 );
        if ( !expression.isEmpty() )
          qmlCode->insertText( u"expression.evaluate(\"%1\")"_s.arg( expression ) );
      } );

      connect( editExpressionButton, &QAbstractButton::clicked, this, [this, qmlCode] {
        QString expression = QgsExpressionFinder::findAndSelectActiveExpression( qmlCode, u"expression\\.evaluate\\(\\s*\"(.*?)\\s*\"\\s*\\)"_s );
        expression.replace( "\\\""_L1, "\""_L1 );
        QgsExpressionContext context = createExpressionContext();
        QgsExpressionBuilderDialog exprDlg( mLayer, expression, this, u"generic"_s, context );

        exprDlg.setWindowTitle( tr( "Insert Expression" ) );
        if ( exprDlg.exec() == QDialog::Accepted && !exprDlg.expressionText().trimmed().isEmpty() )
        {
          QString expression = exprDlg.expressionText().trimmed().replace( '"', "\\\""_L1 );
          if ( !expression.isEmpty() )
            qmlCode->insertText( u"expression.evaluate(\"%1\")"_s.arg( expression ) );
        }
      } );

      layout->addWidget( new QLabel( tr( "Title" ) ) );
      layout->addWidget( title );
      QGroupBox *qmlCodeBox = new QGroupBox( tr( "QML Code" ) );
      qmlCodeBox->setLayout( new QVBoxLayout );
      qmlCodeBox->layout()->addWidget( qmlObjectTemplate );
      QWidget *expressionWidgetBox = new QWidget();
      qmlCodeBox->layout()->addWidget( expressionWidgetBox );
      expressionWidgetBox->setLayout( new QHBoxLayout );
      expressionWidgetBox->layout()->setContentsMargins( 0, 0, 0, 0 );
      expressionWidgetBox->layout()->addWidget( expressionWidget );
      expressionWidgetBox->layout()->addWidget( addFieldButton );
      expressionWidgetBox->layout()->addWidget( editExpressionButton );
      expressionWidgetBox->layout()->addWidget( editExpressionButton );
      layout->addWidget( qmlCodeBox );
      layout->addWidget( qmlCode );
      QScrollArea *qmlPreviewBox = new QgsScrollArea();
      qmlPreviewBox->setMinimumWidth( 200 );
      qmlPreviewBox->setWidget( qmlWrapper->widget() );
      //emit to load preview for the first time
      emit qmlCode->editingTimeout();
      qmlSplitter->addWidget( qmlPreviewBox );
      qmlSplitter->setChildrenCollapsible( false );
      qmlSplitter->setHandleWidth( 6 );
      qmlSplitter->setSizes( QList<int>() << 1 << 1 );

      QDialogButtonBox *buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help );

      connect( buttonBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept );
      connect( buttonBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject );
      connect( buttonBox, &QDialogButtonBox::helpRequested, &dlg, [] {
        QgsHelp::openHelp( u"working_with_vector/vector_properties.html#other-widgets"_s );
      } );

      mainLayout->addWidget( buttonBox );

      if ( dlg.exec() )
      {
        QgsAttributesFormData::QmlElementEditorConfiguration qmlEdCfg;
        qmlEdCfg.qmlCode = qmlCode->text();
        itemData.setQmlElementEditorConfiguration( qmlEdCfg );
        itemData.setShowLabel( showLabelCheckbox->isChecked() );

        mModel->sourceModel()->setData( sourceIndex, itemData, QgsAttributesFormModel::ItemDataRole );
        mModel->sourceModel()->setData( sourceIndex, title->text(), QgsAttributesFormModel::ItemNameRole );
      }
    }
    break;

    case QgsAttributesFormData::HtmlWidget:
    {
      QDialog dlg;
      dlg.setObjectName( "HTML Form Configuration Widget" );
      QgsGui::enableAutoGeometryRestore( &dlg );
      dlg.setWindowTitle( tr( "Configure HTML Widget" ) );

      QVBoxLayout *mainLayout = new QVBoxLayout( &dlg );
      QSplitter *htmlSplitter = new QSplitter();
      QWidget *htmlConfigWiget = new QWidget();
      QVBoxLayout *layout = new QVBoxLayout( htmlConfigWiget );
      layout->setContentsMargins( 0, 0, 0, 0 );
      mainLayout->addWidget( htmlSplitter );
      htmlSplitter->addWidget( htmlConfigWiget );
      htmlSplitter->setChildrenCollapsible( false );
      htmlSplitter->setHandleWidth( 6 );
      htmlSplitter->setSizes( QList<int>() << 1 << 1 );
      layout->addWidget( baseWidget );

      QLineEdit *title = new QLineEdit( itemName );

      //htmlCode
      QgsCodeEditorHTML *htmlCode = new QgsCodeEditorHTML();
      htmlCode->setSizePolicy( QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding );
      htmlCode->setText( itemData.htmlElementEditorConfiguration().htmlCode );

      QgsHtmlWidgetWrapper *htmlWrapper = new QgsHtmlWidgetWrapper( mLayer, nullptr, this );
      QgsFeature previewFeature;
      mLayer->getFeatures().nextFeature( previewFeature );

      //update preview on text change
      connect( htmlCode, &QgsCodeEditorHTML::textChanged, this, [htmlWrapper, htmlCode, previewFeature] {
        htmlWrapper->setHtmlCode( htmlCode->text() );
        htmlWrapper->reinitWidget();
        htmlWrapper->setFeature( previewFeature );
      } );

      QgsFieldExpressionWidget *expressionWidget = new QgsFieldExpressionWidget;
      expressionWidget->setButtonVisible( false );
      expressionWidget->registerExpressionContextGenerator( this );
      expressionWidget->setLayer( mLayer );
      QToolButton *addFieldButton = new QToolButton();
      addFieldButton->setIcon( QgsApplication::getThemeIcon( u"/symbologyAdd.svg"_s ) );

      QToolButton *editExpressionButton = new QToolButton();
      editExpressionButton->setIcon( QgsApplication::getThemeIcon( u"/mIconExpression.svg"_s ) );
      editExpressionButton->setToolTip( tr( "Insert/Edit Expression" ) );

      connect( addFieldButton, &QAbstractButton::clicked, this, [expressionWidget, htmlCode] {
        QString expression = expressionWidget->expression().trimmed().replace( '"', "\\\""_L1 );
        if ( !expression.isEmpty() )
          htmlCode->insertText( u"<script>document.write(expression.evaluate(\"%1\"));</script>"_s.arg( expression ) );
      } );

      connect( editExpressionButton, &QAbstractButton::clicked, this, [this, htmlCode] {
        QString expression = QgsExpressionFinder::findAndSelectActiveExpression( htmlCode, u"<script>\\s*document\\.write\\(\\s*expression\\.evaluate\\(\\s*\"(.*?)\\s*\"\\s*\\)\\s*\\)\\s*;?\\s*</script>"_s );
        expression.replace( "\\\""_L1, "\""_L1 );
        QgsExpressionContext context = createExpressionContext();
        QgsExpressionBuilderDialog exprDlg( mLayer, expression, this, u"generic"_s, context );

        exprDlg.setWindowTitle( tr( "Insert Expression" ) );
        if ( exprDlg.exec() == QDialog::Accepted && !exprDlg.expressionText().trimmed().isEmpty() )
        {
          QString expression = exprDlg.expressionText().trimmed().replace( '"', "\\\""_L1 );
          if ( !expression.isEmpty() )
            htmlCode->insertText( u"<script>document.write(expression.evaluate(\"%1\"));</script>"_s.arg( expression ) );
        }
      } );

      layout->addWidget( new QLabel( tr( "Title" ) ) );
      layout->addWidget( title );
      QGroupBox *expressionWidgetBox = new QGroupBox( tr( "HTML Code" ) );
      layout->addWidget( expressionWidgetBox );
      expressionWidgetBox->setLayout( new QHBoxLayout );
      expressionWidgetBox->layout()->addWidget( expressionWidget );
      expressionWidgetBox->layout()->addWidget( addFieldButton );
      expressionWidgetBox->layout()->addWidget( editExpressionButton );
      layout->addWidget( htmlCode );
      QScrollArea *htmlPreviewBox = new QgsScrollArea();
      htmlPreviewBox->setLayout( new QGridLayout );
      htmlPreviewBox->setMinimumWidth( 200 );
      htmlPreviewBox->layout()->addWidget( htmlWrapper->widget() );
      //emit to load preview for the first time
      emit htmlCode->textChanged();
      htmlSplitter->addWidget( htmlPreviewBox );
      htmlSplitter->setChildrenCollapsible( false );
      htmlSplitter->setHandleWidth( 6 );
      htmlSplitter->setSizes( QList<int>() << 1 << 1 );

      QDialogButtonBox *buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help );

      connect( buttonBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept );
      connect( buttonBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject );
      connect( buttonBox, &QDialogButtonBox::helpRequested, &dlg, [] {
        QgsHelp::openHelp( u"working_with_vector/vector_properties.html#other-widgets"_s );
      } );

      mainLayout->addWidget( buttonBox );

      if ( dlg.exec() )
      {
        QgsAttributesFormData::HtmlElementEditorConfiguration htmlEdCfg;
        htmlEdCfg.htmlCode = htmlCode->text();
        itemData.setHtmlElementEditorConfiguration( htmlEdCfg );
        itemData.setShowLabel( showLabelCheckbox->isChecked() );

        mModel->sourceModel()->setData( sourceIndex, itemData, QgsAttributesFormModel::ItemDataRole );
        mModel->sourceModel()->setData( sourceIndex, title->text(), QgsAttributesFormModel::ItemNameRole );
      }
      break;
    }

    case QgsAttributesFormData::TextWidget:
    {
      QDialog dlg;
      dlg.setObjectName( "Text Form Configuration Widget" );
      QgsGui::enableAutoGeometryRestore( &dlg );
      dlg.setWindowTitle( tr( "Configure Text Widget" ) );

      QVBoxLayout *mainLayout = new QVBoxLayout( &dlg );
      QSplitter *textSplitter = new QSplitter();
      QWidget *textConfigWiget = new QWidget();
      QVBoxLayout *layout = new QVBoxLayout( textConfigWiget );
      layout->setContentsMargins( 0, 0, 0, 0 );
      mainLayout->addWidget( textSplitter );
      textSplitter->addWidget( textConfigWiget );
      layout->addWidget( baseWidget );

      QLineEdit *title = new QLineEdit( itemName );

      QgsCodeEditorHTML *text = new QgsCodeEditorHTML();
      text->setSizePolicy( QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding );
      text->setText( itemData.textElementEditorConfiguration().text );

      QgsTextWidgetWrapper *textWrapper = new QgsTextWidgetWrapper( mLayer, nullptr, this );
      QgsFeature previewFeature;
      ( void ) mLayer->getFeatures( QgsFeatureRequest().setLimit( 1 ) ).nextFeature( previewFeature );

      //update preview on text change
      connect( text, &QgsCodeEditorExpression::textChanged, this, [textWrapper, previewFeature, text] {
        textWrapper->setText( text->text() );
        textWrapper->reinitWidget();
        textWrapper->setFeature( previewFeature );
      } );

      QgsFieldExpressionWidget *expressionWidget = new QgsFieldExpressionWidget;
      expressionWidget->setButtonVisible( false );
      expressionWidget->registerExpressionContextGenerator( this );
      expressionWidget->setLayer( mLayer );
      QToolButton *addFieldButton = new QToolButton();
      addFieldButton->setIcon( QgsApplication::getThemeIcon( u"/symbologyAdd.svg"_s ) );

      QToolButton *editExpressionButton = new QToolButton();
      editExpressionButton->setIcon( QgsApplication::getThemeIcon( u"/mIconExpression.svg"_s ) );
      editExpressionButton->setToolTip( tr( "Insert/Edit Expression" ) );

      connect( addFieldButton, &QAbstractButton::clicked, this, [expressionWidget, text] {
        QString expression = expressionWidget->expression().trimmed();
        if ( !expression.isEmpty() )
          text->insertText( u"[%%1%]"_s.arg( expression ) );
      } );
      connect( editExpressionButton, &QAbstractButton::clicked, this, [this, text] {
        QString expression = QgsExpressionFinder::findAndSelectActiveExpression( text );

        QgsExpressionContext context = createExpressionContext();
        QgsExpressionBuilderDialog exprDlg( mLayer, expression, this, u"generic"_s, context );

        exprDlg.setWindowTitle( tr( "Insert Expression" ) );
        if ( exprDlg.exec() == QDialog::Accepted && !exprDlg.expressionText().trimmed().isEmpty() )
        {
          QString expression = exprDlg.expressionText().trimmed();
          if ( !expression.isEmpty() )
            text->insertText( u"[%%1%]"_s.arg( expression ) );
        }
      } );

      layout->addWidget( new QLabel( tr( "Title" ) ) );
      layout->addWidget( title );
      QGroupBox *expressionWidgetBox = new QGroupBox( tr( "Text" ) );
      layout->addWidget( expressionWidgetBox );
      expressionWidgetBox->setLayout( new QHBoxLayout );
      expressionWidgetBox->layout()->addWidget( expressionWidget );
      expressionWidgetBox->layout()->addWidget( addFieldButton );
      expressionWidgetBox->layout()->addWidget( editExpressionButton );
      layout->addWidget( text );
      QScrollArea *textPreviewBox = new QgsScrollArea();
      textPreviewBox->setLayout( new QGridLayout );
      textPreviewBox->setMinimumWidth( 200 );
      textPreviewBox->layout()->addWidget( textWrapper->widget() );
      //emit to load preview for the first time
      emit text->textChanged();
      textSplitter->addWidget( textPreviewBox );
      textSplitter->setChildrenCollapsible( false );
      textSplitter->setHandleWidth( 6 );
      textSplitter->setSizes( QList<int>() << 1 << 1 );

      QDialogButtonBox *buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help );

      connect( buttonBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept );
      connect( buttonBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject );
      connect( buttonBox, &QDialogButtonBox::helpRequested, &dlg, [] {
        QgsHelp::openHelp( u"working_with_vector/vector_properties.html#other-widgets"_s );
      } );

      mainLayout->addWidget( buttonBox );

      if ( dlg.exec() )
      {
        QgsAttributesFormData::TextElementEditorConfiguration textEdCfg;
        textEdCfg.text = text->text();
        itemData.setTextElementEditorConfiguration( textEdCfg );
        itemData.setShowLabel( showLabelCheckbox->isChecked() );

        mModel->sourceModel()->setData( sourceIndex, itemData, QgsAttributesFormModel::ItemDataRole );
        mModel->sourceModel()->setData( sourceIndex, title->text(), QgsAttributesFormModel::ItemNameRole );
      }
      break;
    }

    case QgsAttributesFormData::SpacerWidget:
    {
      QDialog dlg;
      dlg.setObjectName( "Spacer Form Configuration Widget" );
      QgsGui::enableAutoGeometryRestore( &dlg );
      dlg.setWindowTitle( tr( "Configure Spacer Widget" ) );

      QVBoxLayout *mainLayout = new QVBoxLayout();
      mainLayout->addWidget( new QLabel( tr( "Title" ) ) );
      QLineEdit *title = new QLineEdit( itemName );
      mainLayout->addWidget( title );

      QHBoxLayout *cbLayout = new QHBoxLayout();
      mainLayout->addLayout( cbLayout );
      dlg.setLayout( mainLayout );
      QCheckBox *cb = new QCheckBox { &dlg };
      cb->setChecked( itemData.spacerElementEditorConfiguration().drawLine );
      cbLayout->addWidget( new QLabel( tr( "Draw horizontal line" ), &dlg ) );
      cbLayout->addWidget( cb );

      QDialogButtonBox *buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help );

      connect( buttonBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept );
      connect( buttonBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject );
      connect( buttonBox, &QDialogButtonBox::helpRequested, &dlg, [] {
        QgsHelp::openHelp( u"working_with_vector/vector_properties.html#other-widgets"_s );
      } );

      mainLayout->addWidget( buttonBox );

      if ( dlg.exec() )
      {
        QgsAttributesFormData::SpacerElementEditorConfiguration spacerEdCfg;
        spacerEdCfg.drawLine = cb->isChecked();
        itemData.setSpacerElementEditorConfiguration( spacerEdCfg );
        itemData.setShowLabel( false );

        mModel->sourceModel()->setData( sourceIndex, itemData, QgsAttributesFormModel::ItemDataRole );
        mModel->sourceModel()->setData( sourceIndex, title->text(), QgsAttributesFormModel::ItemNameRole );
      }

      break;
    }
  }
}
