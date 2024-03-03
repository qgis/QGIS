/***************************************************************************
    qgsvaluerelationwidgetwrapper.cpp
     --------------------------------------
    Date                 : 5.1.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvaluerelationwidgetwrapper.h"

#include "qgis.h"
#include "qgsfields.h"
#include "qgsproject.h"
#include "qgsvaluerelationwidgetfactory.h"
#include "qgsvectorlayer.h"
#include "qgsfilterlineedit.h"
#include "qgsfeatureiterator.h"
#include "qgsvaluerelationfieldformatter.h"
#include "qgsattributeform.h"
#include "qgsattributes.h"
#include "qgsjsonutils.h"
#include "qgspostgresstringutils.h"
#include "qgsapplication.h"

#include <QComboBox>
#include <QLineEdit>
#include <QStringListModel>
#include <QCompleter>
#include <QTimer>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QKeyEvent>
#include <QStandardItemModel>

#include <nlohmann/json.hpp>
using namespace nlohmann;

///@cond PRIVATE
QgsFilteredTableWidget::QgsFilteredTableWidget( QWidget *parent, bool showSearch, bool displayGroupName )
  : QWidget( parent )
  , mDisplayGroupName( displayGroupName )
{
  mSearchWidget = new QgsFilterLineEdit( this );
  mSearchWidget->setShowSearchIcon( true );
  mSearchWidget->setShowClearButton( true );
  mTableWidget  = new QTableWidget( this );
  mTableWidget->horizontalHeader()->setSectionResizeMode( QHeaderView::Stretch );
  mTableWidget->horizontalHeader()->setVisible( false );
  mTableWidget->verticalHeader()->setSectionResizeMode( QHeaderView::Stretch );
  mTableWidget->verticalHeader()->setVisible( false );
  mTableWidget->setShowGrid( false );
  mTableWidget->setEditTriggers( QAbstractItemView::NoEditTriggers );
  mTableWidget->setSelectionMode( QAbstractItemView::NoSelection );
  QVBoxLayout *layout = new QVBoxLayout();
  layout->addWidget( mSearchWidget );
  layout->addWidget( mTableWidget );
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->setSpacing( 0 );
  if ( showSearch )
  {
    mTableWidget->setFocusProxy( mSearchWidget );
    connect( mSearchWidget, &QgsFilterLineEdit::textChanged, this, &QgsFilteredTableWidget::filterStringChanged );
    installEventFilter( this );
  }
  else
  {
    mSearchWidget->setVisible( false );
  }
  setLayout( layout );
  connect( mTableWidget, &QTableWidget::itemChanged, this, &QgsFilteredTableWidget::itemChanged_p );
}

bool QgsFilteredTableWidget::eventFilter( QObject *watched, QEvent *event )
{
  Q_UNUSED( watched )
  if ( event->type() == QEvent::KeyPress )
  {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>( event );
    if ( keyEvent->key() == Qt::Key_Escape &&
         !mSearchWidget->text().isEmpty() )
    {
      mSearchWidget->clear();
      return true;
    }
  }
  return false;
}

void QgsFilteredTableWidget::filterStringChanged( const QString &filterString )
{
  auto signalBlockedTableWidget = whileBlocking( mTableWidget );
  Q_UNUSED( signalBlockedTableWidget )

  mTableWidget->clearContents();
  if ( !mCache.isEmpty() )
  {
    QVariantList groups;
    groups << QVariant();
    for ( const QPair<QgsValueRelationFieldFormatter::ValueRelationItem, Qt::CheckState> &pair : std::as_const( mCache ) )
    {
      if ( !groups.contains( pair.first.group ) )
      {
        groups << pair.first.group;
      }
    }
    const int groupsCount = mDisplayGroupName ? groups.count() : groups.count() - 1;

    const int rCount = std::max( 1, ( int ) std::ceil( ( float )( mCache.count() + groupsCount ) / ( float ) mColumnCount ) );
    mTableWidget->setRowCount( rCount );

    int row = 0;
    int column = 0;
    QVariant currentGroup;
    for ( const QPair<QgsValueRelationFieldFormatter::ValueRelationItem, Qt::CheckState> &pair : std::as_const( mCache ) )
    {
      if ( column == mColumnCount )
      {
        row++;
        column = 0;
      }
      if ( currentGroup != pair.first.group )
      {
        currentGroup = pair.first.group;
        if ( mDisplayGroupName || !( row == 0 && column == 0 ) )
        {
          QTableWidgetItem *item = new QTableWidgetItem( mDisplayGroupName ? pair.first.group.toString() : QString() );
          item->setFlags( item->flags() & ~Qt::ItemIsEnabled );
          mTableWidget->setItem( row, column, item );
          column++;
          if ( column == mColumnCount )
          {
            row++;
            column = 0;
          }
        }
      }
      if ( pair.first.value.contains( filterString, Qt::CaseInsensitive ) )
      {
        QTableWidgetItem *item = new QTableWidgetItem( pair.first.value );
        item->setData( Qt::UserRole, pair.first.key );
        item->setData( Qt::ToolTipRole, pair.first.description );
        item->setCheckState( pair.second );
        item->setFlags( mEnabledTable ? item->flags() | Qt::ItemIsEnabled : item->flags() & ~Qt::ItemIsEnabled );
        mTableWidget->setItem( row, column, item );
        column++;
      }
    }
    mTableWidget->setRowCount( row + 1 );
  }
}

QStringList QgsFilteredTableWidget::selection() const
{
  QStringList sel;
  for ( const QPair<QgsValueRelationFieldFormatter::ValueRelationItem, Qt::CheckState> &pair : std::as_const( mCache ) )
  {
    if ( pair.second == Qt::Checked )
      sel.append( pair.first.key.toString() );
  }
  return sel;
}

void QgsFilteredTableWidget::checkItems( const QStringList &checked )
{
  for ( QPair<QgsValueRelationFieldFormatter::ValueRelationItem, Qt::CheckState> &pair : mCache )
  {
    const bool isChecked = checked.contains( pair.first.key.toString() );
    pair.second = isChecked ? Qt::Checked : Qt::Unchecked;
  }

  filterStringChanged( mSearchWidget->text() );
}

void QgsFilteredTableWidget::populate( QgsValueRelationFieldFormatter::ValueRelationCache cache )
{
  mCache.clear();
  for ( const QgsValueRelationFieldFormatter::ValueRelationItem &element : std::as_const( cache ) )
  {
    mCache.append( qMakePair( element, Qt::Unchecked ) );
  }
  filterStringChanged( mSearchWidget->text() );
}

void QgsFilteredTableWidget::setIndeterminateState()
{
  for ( int rowIndex = 0; rowIndex < mTableWidget->rowCount(); rowIndex++ )
  {
    for ( int columnIndex = 0; columnIndex < mColumnCount; ++columnIndex )
    {
      if ( item( rowIndex, columnIndex ) )
      {
        whileBlocking( mTableWidget )->item( rowIndex, columnIndex )->setCheckState( Qt::PartiallyChecked );
      }
      else
      {
        break;
      }
    }
  }
}

void QgsFilteredTableWidget::setEnabledTable( const bool enabled )
{
  if ( mEnabledTable == enabled )
    return;

  mEnabledTable = enabled;
  if ( !enabled )
    mSearchWidget->clear();

  filterStringChanged( mSearchWidget->text() );
}

void QgsFilteredTableWidget::setColumnCount( const int count )
{
  mColumnCount = count;
  mTableWidget->setColumnCount( count );
}

void QgsFilteredTableWidget::itemChanged_p( QTableWidgetItem *item )
{
  for ( QPair<QgsValueRelationFieldFormatter::ValueRelationItem, Qt::CheckState> &pair : mCache )
  {
    if ( pair.first.key == item->data( Qt::UserRole ) )
      pair.second = item->checkState();
  }
  emit itemChanged( item );
}
///@endcond


QgsValueRelationWidgetWrapper::QgsValueRelationWidgetWrapper( QgsVectorLayer *layer, int fieldIdx, QWidget *editor, QWidget *parent )
  : QgsEditorWidgetWrapper( layer, fieldIdx, editor, parent )
{
}

QVariant QgsValueRelationWidgetWrapper::value() const
{
  QVariant v;

  if ( mComboBox )
  {
    int cbxIdx = mComboBox->currentIndex();
    if ( cbxIdx > -1 )
    {
      v = mComboBox->currentData();
      if ( QgsVariantUtils::isNull( v ) )
        v = QVariant( field().type() );
    }
  }
  else if ( mTableWidget )
  {
    QStringList selection = mTableWidget->selection();

    // If there is no selection and allow NULL is not checked return NULL.
    if ( selection.isEmpty() && ! config( QStringLiteral( "AllowNull" ) ).toBool( ) )
    {
      return QVariant( QVariant::Type::List );
    }

    QVariantList vl;
    //store as QVariantList because the field type supports data structure
    for ( const QString &s : std::as_const( selection ) )
    {
      // Convert to proper type
      const QVariant::Type type { fkType() };
      switch ( type )
      {
        case QVariant::Type::Int:
          vl.push_back( s.toInt() );
          break;
        case QVariant::Type::LongLong:
          vl.push_back( s.toLongLong() );
          break;
        default:
          vl.push_back( s );
          break;
      }
    }

    if ( layer()->fields().at( fieldIdx() ).type() == QVariant::Map ||
         layer()->fields().at( fieldIdx() ).type() == QVariant::List )
    {
      v = vl;
    }
    else
    {
      //make string
      v = QgsPostgresStringUtils::buildArray( vl );
    }
  }
  else if ( mLineEdit )
  {
    for ( const QgsValueRelationFieldFormatter::ValueRelationItem &item : std::as_const( mCache ) )
    {
      if ( item.value == mLineEdit->text() )
      {
        v = item.key;
        break;
      }
    }
  }

  return v;
}

QWidget *QgsValueRelationWidgetWrapper::createWidget( QWidget *parent )
{
  QgsAttributeForm *form = qobject_cast<QgsAttributeForm *>( parent );
  if ( form )
    connect( form, &QgsAttributeForm::widgetValueChanged, this, &QgsValueRelationWidgetWrapper::widgetValueChanged );

  mExpression = config().value( QStringLiteral( "FilterExpression" ) ).toString();

  const bool allowMulti = config( QStringLiteral( "AllowMulti" ) ).toBool();
  const bool useCompleter = config( QStringLiteral( "UseCompleter" ) ).toBool();
  if ( allowMulti )
  {
    const bool displayGroupName = config( QStringLiteral( "DisplayGroupName" ) ).toBool();
    return new QgsFilteredTableWidget( parent, useCompleter, displayGroupName );
  }
  else if ( useCompleter )
  {
    return new QgsFilterLineEdit( parent );
  }
  else
  {
    QgsToolTipComboBox *combo = new QgsToolTipComboBox( parent );
    combo->setMinimumContentsLength( 1 );
    combo->setSizeAdjustPolicy( QComboBox::SizeAdjustPolicy::AdjustToMinimumContentsLengthWithIcon );
    return combo;
  }
}

void QgsValueRelationWidgetWrapper::initWidget( QWidget *editor )
{
  mComboBox = qobject_cast<QComboBox *>( editor );
  mTableWidget = qobject_cast<QgsFilteredTableWidget *>( editor );
  mLineEdit = qobject_cast<QLineEdit *>( editor );

  // Read current initial form values from the editor context
  setFeature( context().formFeature() );

  if ( mComboBox )
  {
    mComboBox->view()->setVerticalScrollBarPolicy( Qt::ScrollBarAsNeeded );
    connect( mComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ),
             this, static_cast<void ( QgsEditorWidgetWrapper::* )()>( &QgsEditorWidgetWrapper::emitValueChanged ), Qt::UniqueConnection );
  }
  else if ( mTableWidget )
  {
    connect( mTableWidget, &QgsFilteredTableWidget::itemChanged, this, static_cast<void ( QgsEditorWidgetWrapper::* )()>( &QgsEditorWidgetWrapper::emitValueChanged ), Qt::UniqueConnection );
  }
  else if ( mLineEdit )
  {
    if ( QgsFilterLineEdit *filterLineEdit = qobject_cast<QgsFilterLineEdit *>( editor ) )
    {
      connect( filterLineEdit, &QgsFilterLineEdit::valueChanged, this, [ = ]( const QString & )
      {
        if ( mSubWidgetSignalBlocking == 0 )
          emitValueChanged();
      } );
    }
    else
    {
      connect( mLineEdit, &QLineEdit::textChanged, this, &QgsValueRelationWidgetWrapper::emitValueChangedInternal, Qt::UniqueConnection );
    }
  }
}

bool QgsValueRelationWidgetWrapper::valid() const
{
  return mTableWidget || mLineEdit || mComboBox;
}

void QgsValueRelationWidgetWrapper::updateValues( const QVariant &value, const QVariantList & )
{
  if ( mTableWidget )
  {
    QStringList checkList;

    if ( layer()->fields().at( fieldIdx() ).type() == QVariant::Map ||
         layer()->fields().at( fieldIdx() ).type() == QVariant::List )
    {
      checkList = value.toStringList();
    }
    else
    {
      checkList = QgsValueRelationFieldFormatter::valueToStringList( value );
    }

    mTableWidget->checkItems( checkList );
  }
  else if ( mComboBox )
  {
    // findData fails to tell a 0 from a NULL
    // See: "Value relation, value 0 = NULL" - https://github.com/qgis/QGIS/issues/27803
    int idx = -1; // default to not found
    for ( int i = 0; i < mComboBox->count(); i++ )
    {
      QVariant v( mComboBox->itemData( i ) );
      if ( qgsVariantEqual( v, value ) )
      {
        idx = i;
        break;
      }
    }

    if ( idx == -1 )
    {
      // if value doesn't exist, we show it in '(...)' (just like value map widget)
      if ( QgsVariantUtils::isNull( value ) )
      {
        mComboBox->setCurrentIndex( -1 );
      }
      else
      {
        mComboBox->addItem( value.toString().prepend( '(' ).append( ')' ), value );
        mComboBox->setCurrentIndex( mComboBox->findData( value ) );
      }
    }
    else
    {
      mComboBox->setCurrentIndex( idx );
    }
  }
  else if ( mLineEdit )
  {
    mSubWidgetSignalBlocking ++;
    mLineEdit->clear();
    bool wasFound { false };
    for ( const QgsValueRelationFieldFormatter::ValueRelationItem &i : std::as_const( mCache ) )
    {
      if ( i.key == value )
      {
        mLineEdit->setText( i.value );
        wasFound = true;
        break;
      }
    }
    // Value could not be found
    if ( ! wasFound )
    {
      mLineEdit->setText( tr( "(no selection)" ) );
    }
    mSubWidgetSignalBlocking --;
  }
}

void QgsValueRelationWidgetWrapper::widgetValueChanged( const QString &attribute, const QVariant &newValue, bool attributeChanged )
{

  // Do nothing if the value has not changed
  if ( attributeChanged )
  {
    QVariant oldValue( value( ) );
    setFormFeatureAttribute( attribute, newValue );
    // Update combos if the value used in the filter expression has changed
    if ( QgsValueRelationFieldFormatter::expressionRequiresFormScope( mExpression )
         && QgsValueRelationFieldFormatter::expressionFormAttributes( mExpression ).contains( attribute ) )
    {
      populate();
      // Restore value
      updateValues( value( ) );
      // If the value has changed as a result of another widget's value change,
      // we need to emit the signal to make sure other dependent widgets are
      // updated.
      QgsFields formFields( formFeature().fields() );

      // Also check for fields in the layer in case this is a multi-edit form
      // and there is not form feature set
      if ( formFields.count() == 0 && layer() )
      {
        formFields = layer()->fields();
      }

      if ( oldValue != value() && fieldIdx() < formFields.count() )
      {
        QString attributeName( formFields.names().at( fieldIdx() ) );
        setFormFeatureAttribute( attributeName, value( ) );
        emitValueChanged();
      }
    }
  }
}


void QgsValueRelationWidgetWrapper::setFeature( const QgsFeature &feature )
{
  setFormFeature( feature );
  whileBlocking( this )->populate();
  whileBlocking( this )->setValue( feature.attribute( fieldIdx() ) );

  // As we block any signals, possible depending widgets will not being updated
  // so we force emit signal once and for all
  emitValueChanged();

  // A bit of logic to set the default value if AllowNull is false and this is a new feature
  // Note that this needs to be here after the cache has been created/updated by populate()
  // and signals unblocked (we want this to propagate to the feature itself)
  if ( context().attributeFormMode() != QgsAttributeEditorContext::Mode::MultiEditMode
       && ! formFeature().attribute( fieldIdx() ).isValid()
       && ! mCache.isEmpty()
       && ! config( QStringLiteral( "AllowNull" ) ).toBool( ) )
  {
    // This is deferred because at the time the feature is set in one widget it is not
    // set in the next, which is typically the "down" in a drill-down
    QTimer::singleShot( 0, this, [ this ]
    {
      if ( ! mCache.isEmpty() )
      {
        updateValues( formFeature().attribute( fieldIdx() ).isValid() ? formFeature().attribute( fieldIdx() ) : mCache.at( 0 ).key );
      }
    } );
  }
}

int QgsValueRelationWidgetWrapper::columnCount() const
{
  return std::max( 1, config( QStringLiteral( "NofColumns" ) ).toInt() );
}


QVariant::Type QgsValueRelationWidgetWrapper::fkType() const
{
  const QgsVectorLayer *layer = QgsValueRelationFieldFormatter::resolveLayer( config(), QgsProject::instance() );
  if ( layer )
  {
    QgsFields fields = layer->fields();
    int idx { fields.lookupField( config().value( QStringLiteral( "Key" ) ).toString() )  };
    if ( idx >= 0 )
    {
      return fields.at( idx ).type();
    }
  }
  return QVariant::Type::Invalid;
}

void QgsValueRelationWidgetWrapper::populate()
{
  // Initialize, note that signals are blocked, to avoid double signals on new features
  if ( QgsValueRelationFieldFormatter::expressionRequiresFormScope( mExpression ) ||
       QgsValueRelationFieldFormatter::expressionRequiresParentFormScope( mExpression ) )
  {
    if ( context().parentFormFeature().isValid() )
    {
      mCache = QgsValueRelationFieldFormatter::createCache( config(), formFeature(), context().parentFormFeature() );
    }
    else
    {
      mCache = QgsValueRelationFieldFormatter::createCache( config(), formFeature() );
    }
  }
  else if ( mCache.empty() )
  {
    mCache = QgsValueRelationFieldFormatter::createCache( config() );
  }

  if ( mComboBox )
  {
    mComboBox->blockSignals( true );
    mComboBox->clear();
    const bool allowNull = config( QStringLiteral( "AllowNull" ) ).toBool();
    if ( allowNull )
    {
      mComboBox->addItem( tr( "(no selection)" ), QVariant( field().type( ) ) );
    }

    if ( !mCache.isEmpty() )
    {
      QVariant currentGroup;
      QStandardItemModel *model = qobject_cast<QStandardItemModel *>( mComboBox->model() );
      const bool displayGroupName = config( QStringLiteral( "DisplayGroupName" ) ).toBool();
      for ( const QgsValueRelationFieldFormatter::ValueRelationItem &element : std::as_const( mCache ) )
      {
        if ( currentGroup != element.group )
        {
          if ( mComboBox->count() > ( allowNull ? 1 : 0 ) )
          {
            mComboBox->insertSeparator( mComboBox->count() );
          }
          if ( displayGroupName )
          {
            mComboBox->addItem( element.group.toString() );
            QStandardItem *item = model->item( mComboBox->count() - 1 );
            item->setFlags( item->flags() & ~Qt::ItemIsEnabled );
          }
          currentGroup = element.group;
        }

        mComboBox->addItem( element.value, element.key );

        if ( !element.description.isEmpty() )
        {
          mComboBox->setItemData( mComboBox->count() - 1, element.description, Qt::ToolTipRole );
        }
      }
    }
    mComboBox->blockSignals( false );
  }
  else if ( mTableWidget )
  {
    mTableWidget->setColumnCount( columnCount() );
    mTableWidget->populate( mCache );
  }
  else if ( mLineEdit )
  {
    QStringList values;
    values.reserve( mCache.size() );
    for ( const QgsValueRelationFieldFormatter::ValueRelationItem &i : std::as_const( mCache ) )
    {
      values << i.value;
    }
    QStringListModel *m = new QStringListModel( values, mLineEdit );
    QCompleter *completer = new QCompleter( m, mLineEdit );

    const Qt::MatchFlags completerMatchFlags { config().contains( QStringLiteral( "CompleterMatchFlags" ) ) ? static_cast<Qt::MatchFlags>( config().value( QStringLiteral( "CompleterMatchFlags" ), Qt::MatchFlag::MatchStartsWith ).toInt( ) ) :  Qt::MatchFlag::MatchStartsWith };

    if ( completerMatchFlags.testFlag( Qt::MatchFlag::MatchContains ) )
    {
      completer->setFilterMode( Qt::MatchFlag::MatchContains );
    }
    else
    {
      completer->setFilterMode( Qt::MatchFlag::MatchStartsWith );
    }
    completer->setCaseSensitivity( Qt::CaseInsensitive );
    mLineEdit->setCompleter( completer );
  }
}

void QgsValueRelationWidgetWrapper::showIndeterminateState()
{
  if ( mTableWidget )
  {
    mTableWidget->setIndeterminateState();
  }
  else if ( mComboBox )
  {
    whileBlocking( mComboBox )->setCurrentIndex( -1 );
  }
  else if ( mLineEdit )
  {
    whileBlocking( mLineEdit )->clear();
  }
}

void QgsValueRelationWidgetWrapper::setEnabled( bool enabled )
{
  if ( mEnabled == enabled )
    return;

  mEnabled = enabled;

  if ( mTableWidget )
  {
    mTableWidget->setEnabledTable( enabled );
  }
  else
    QgsEditorWidgetWrapper::setEnabled( enabled );
}

void QgsValueRelationWidgetWrapper::parentFormValueChanged( const QString &attribute, const QVariant &value )
{

  // Update the parent feature in the context ( which means to replace the whole context :/ )
  QgsAttributeEditorContext ctx { context() };
  QgsFeature feature { context().parentFormFeature() };
  feature.setAttribute( attribute, value );
  ctx.setParentFormFeature( feature );
  setContext( ctx );

  // Check if the change might affect the filter expression and the cache needs updates
  if ( QgsValueRelationFieldFormatter::expressionRequiresParentFormScope( mExpression )
       && ( config( QStringLiteral( "Value" ) ).toString() == attribute ||
            config( QStringLiteral( "Key" ) ).toString() == attribute ||
            ! QgsValueRelationFieldFormatter::expressionParentFormVariables( mExpression ).isEmpty() ||
            QgsValueRelationFieldFormatter::expressionParentFormAttributes( mExpression ).contains( attribute ) ) )
  {
    populate();
  }

}

void QgsValueRelationWidgetWrapper::emitValueChangedInternal( const QString &value )
{
  Q_NOWARN_DEPRECATED_PUSH
  emit valueChanged( value );
  Q_NOWARN_DEPRECATED_POP
  emit valuesChanged( value );
}
