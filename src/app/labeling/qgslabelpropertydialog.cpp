/***************************************************************************
                          qgslabelpropertydialog.cpp
                          --------------------------
    begin                : 2010-11-12
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslabelpropertydialog.h"
#include "qgscallout.h"
#include "qgsfontutils.h"
#include "qgslogger.h"
#include "qgsfeatureiterator.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgisapp.h"
#include "qgsmapcanvas.h"
#include "qgsvectorlayerlabeling.h"
#include "qgsproperty.h"
#include "qgssettings.h"
#include "qgsexpressioncontextutils.h"
#include "qgsgui.h"
#include "qgshelp.h"
#include "qgsexpressionnodeimpl.h"

#include <QColorDialog>
#include <QFontDatabase>
#include <QDialogButtonBox>


QgsLabelPropertyDialog::QgsLabelPropertyDialog( const QString &layerId, const QString &providerId, QgsFeatureId featureId, const QFont &labelFont, const QString &labelText, bool isPinned, const QgsPalLayerSettings &layerSettings, QgsMapCanvas *canvas, QWidget *parent, Qt::WindowFlags f )
  : QDialog( parent, f )
  , mCanvas( canvas )
  , mLabelFont( labelFont )
  , mIsPinned( isPinned )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  // set defaults to layer defaults
  mLabelAllPartsCheckBox->setChecked( layerSettings.labelPerPart );

  connect( buttonBox, &QDialogButtonBox::clicked, this, &QgsLabelPropertyDialog::buttonBox_clicked );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsLabelPropertyDialog::showHelp );
  connect( mShowLabelChkbx, &QCheckBox::toggled, this, &QgsLabelPropertyDialog::mShowLabelChkbx_toggled );
  connect( mAlwaysShowChkbx, &QCheckBox::toggled, this, &QgsLabelPropertyDialog::mAlwaysShowChkbx_toggled );
  connect( mShowCalloutChkbx, &QCheckBox::toggled, this, &QgsLabelPropertyDialog::showCalloutToggled );
  connect( mBufferDrawChkbx, &QCheckBox::toggled, this, &QgsLabelPropertyDialog::bufferDrawToggled );
  connect( mLabelDistanceSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLabelPropertyDialog::mLabelDistanceSpinBox_valueChanged );
  connect( mXCoordSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLabelPropertyDialog::mXCoordSpinBox_valueChanged );
  connect( mYCoordSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLabelPropertyDialog::mYCoordSpinBox_valueChanged );
  connect( mFontFamilyCmbBx, &QFontComboBox::currentFontChanged, this, &QgsLabelPropertyDialog::mFontFamilyCmbBx_currentFontChanged );
  connect( mFontStyleCmbBx, &QComboBox::currentTextChanged, this, &QgsLabelPropertyDialog::mFontStyleCmbBx_currentIndexChanged );
  connect( mFontUnderlineBtn, &QToolButton::toggled, this, &QgsLabelPropertyDialog::mFontUnderlineBtn_toggled );
  connect( mFontStrikethroughBtn, &QToolButton::toggled, this, &QgsLabelPropertyDialog::mFontStrikethroughBtn_toggled );
  connect( mFontBoldBtn, &QToolButton::toggled, this, &QgsLabelPropertyDialog::mFontBoldBtn_toggled );
  connect( mFontItalicBtn, &QToolButton::toggled, this, &QgsLabelPropertyDialog::mFontItalicBtn_toggled );
  connect( mFontSizeSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLabelPropertyDialog::mFontSizeSpinBox_valueChanged );
  connect( mBufferSizeSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLabelPropertyDialog::mBufferSizeSpinBox_valueChanged );
  connect( mRotationSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLabelPropertyDialog::mRotationSpinBox_valueChanged );
  connect( mFontColorButton, &QgsColorButton::colorChanged, this, &QgsLabelPropertyDialog::mFontColorButton_colorChanged );
  connect( mBufferColorButton, &QgsColorButton::colorChanged, this, &QgsLabelPropertyDialog::mBufferColorButton_colorChanged );
  connect( mMultiLineAlignComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLabelPropertyDialog::mMultiLineAlignComboBox_currentIndexChanged );
  connect( mHaliComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLabelPropertyDialog::mHaliComboBox_currentIndexChanged );
  connect( mValiComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLabelPropertyDialog::mValiComboBox_currentIndexChanged );
  connect( mLabelTextLineEdit, &QLineEdit::textChanged, this, &QgsLabelPropertyDialog::mLabelTextLineEdit_textChanged );
  connect( mLabelAllPartsCheckBox, &QCheckBox::toggled, this, &QgsLabelPropertyDialog::labelAllPartsToggled );

  mRotationSpinBox->setClearValue( 0 );
  fillMultiLineAlignComboBox();
  fillHaliComboBox();
  fillValiComboBox();

  init( layerId, providerId, featureId, labelText );

  connect( mMinScaleWidget, &QgsScaleWidget::scaleChanged, this, &QgsLabelPropertyDialog::minScaleChanged );
  connect( mMaxScaleWidget, &QgsScaleWidget::scaleChanged, this, &QgsLabelPropertyDialog::maxScaleChanged );
}

void QgsLabelPropertyDialog::setMapCanvas( QgsMapCanvas *canvas )
{
  mMinScaleWidget->setMapCanvas( canvas );
  mMinScaleWidget->setShowCurrentScaleButton( true );
  mMaxScaleWidget->setMapCanvas( canvas );
  mMaxScaleWidget->setShowCurrentScaleButton( true );
}

void QgsLabelPropertyDialog::buttonBox_clicked( QAbstractButton *button )
{
  if ( buttonBox->buttonRole( button ) == QDialogButtonBox::ApplyRole )
  {
    emit applied();
  }
}

void QgsLabelPropertyDialog::init( const QString &layerId, const QString &providerId, QgsFeatureId featureId, const QString &labelText )
{
  //get feature attributes
  QgsVectorLayer *vlayer = qobject_cast< QgsVectorLayer * >( mCanvas->layer( layerId ) );
  if ( !vlayer )
  {
    return;
  }
  if ( !vlayer->labeling() )
  {
    return;
  }

  if ( !vlayer->getFeatures( QgsFeatureRequest().setFilterFid( featureId ).setFlags( QgsFeatureRequest::NoGeometry ) ).nextFeature( mCurLabelFeat ) )
  {
    return;
  }
  const QgsAttributes attributeValues = mCurLabelFeat.attributes();

  //get layerproperties. Problem: only for pallabeling...

  blockElementSignals( true );

  QgsPalLayerSettings layerSettings = vlayer->labeling()->settings( providerId );

  //get label field and fill line edit
  if ( layerSettings.isExpression && !labelText.isNull() )
  {
    mLabelTextLineEdit->setText( labelText );
    mLabelTextLineEdit->setEnabled( false );
    mLabelTextLabel->setText( tr( "Expression result" ) );
  }
  else
  {
    const QString labelFieldName = layerSettings.fieldName;
    if ( !labelFieldName.isEmpty() )
    {
      mCurLabelField = vlayer->fields().lookupField( labelFieldName );
      if ( mCurLabelField >= 0 )
      {
        mLabelTextLineEdit->setText( attributeValues.at( mCurLabelField ).toString() );

        if ( vlayer->isEditable() )
          mLabelTextLineEdit->setEnabled( true );
        else
          mLabelTextLineEdit->setEnabled( false );

        const QgsFields &layerFields = vlayer->fields();
        switch ( layerFields.at( mCurLabelField ).type() )
        {
          case QVariant::Double:
            mLabelTextLineEdit->setValidator( new QDoubleValidator( this ) );
            break;
          case QVariant::Int:
          case QVariant::UInt:
          case QVariant::LongLong:
            mLabelTextLineEdit->setValidator( new QIntValidator( this ) );
            break;
          default:
            break;
        }
      }
      else
      {
        mLabelTextLineEdit->setEnabled( false );
      }
    }
  }

  //get attributes of the feature and fill data defined values

  // font is set directly from QgsLabelPosition
  updateFont( mLabelFont, false );

  QgsTextFormat format = layerSettings.format();
  const QgsTextBufferSettings buffer = format.buffer();

  //set all the gui elements to the default layer-level values
  mLabelDistanceSpinBox->clear();
  mLabelDistanceSpinBox->setSpecialValueText( tr( "Layer default (%1)" ).arg( QString::number( layerSettings.dist, 'f', mLabelDistanceSpinBox->decimals() ) ) );
  mBufferSizeSpinBox->clear();
  mBufferSizeSpinBox->setSpecialValueText( tr( "Layer default (%1)" ).arg( QString::number( buffer.size(), 'f', mBufferSizeSpinBox->decimals() ) ) );
  mRotationSpinBox->clear();
  mXCoordSpinBox->clear();
  mYCoordSpinBox->clear();

  mShowLabelChkbx->setChecked( true );
  mBufferDrawChkbx->setChecked( buffer.enabled() );
  mShowCalloutChkbx->setChecked( layerSettings.callout() ? layerSettings.callout()->enabled() : false );
  mFontColorButton->setColor( format.color() );
  mBufferColorButton->setColor( buffer.color() );
  mMinScaleWidget->setScale( layerSettings.minimumScale );
  mMaxScaleWidget->setScale( layerSettings.maximumScale );

  QString defaultMultilineAlign;
  switch ( layerSettings.multilineAlign )
  {
    case Qgis::LabelMultiLineAlignment::Left:
      defaultMultilineAlign = QStringLiteral( "left" );
      break;
    case Qgis::LabelMultiLineAlignment::Center:
      defaultMultilineAlign = QStringLiteral( "center" );
      break;
    case Qgis::LabelMultiLineAlignment::Right:
      defaultMultilineAlign = QStringLiteral( "right" );
      break;
    case Qgis::LabelMultiLineAlignment::Justify:
      defaultMultilineAlign = QStringLiteral( "justify" );
      break;
    case Qgis::LabelMultiLineAlignment::FollowPlacement:
      defaultMultilineAlign = QStringLiteral( "follow label placement" );
      break;
  }
  mMultiLineAlignComboBox->setItemText( mMultiLineAlignComboBox->findData( "" ), tr( "Layer default (%1)" ).arg( defaultMultilineAlign ) );
  mMultiLineAlignComboBox->setCurrentIndex( mMultiLineAlignComboBox->findData( "" ) );

  mHaliComboBox->setCurrentIndex( mHaliComboBox->findData( "left", Qt::UserRole, Qt::MatchFixedString ) );
  mValiComboBox->setCurrentIndex( mValiComboBox->findData( "bottom", Qt::UserRole, Qt::MatchFixedString ) );
  mFontColorButton->setColorDialogTitle( tr( "Font Color" ) );
  mBufferColorButton->setColorDialogTitle( tr( "Buffer Color" ) );

  disableGuiElements();

  mDataDefinedProperties = layerSettings.dataDefinedProperties();

  //set widget values from data defined results
  setDataDefinedValues( vlayer );
  //enable widgets connected to data defined fields
  enableDataDefinedWidgets( vlayer );

  blockElementSignals( false );
}

void QgsLabelPropertyDialog::disableGuiElements()
{
  mShowLabelChkbx->setEnabled( false );
  mAlwaysShowChkbx->setEnabled( false );
  mShowCalloutChkbx->setEnabled( false );
  mMinScaleWidget->setEnabled( false );
  mMaxScaleWidget->setEnabled( false );
  mFontFamilyCmbBx->setEnabled( false );
  mFontStyleCmbBx->setEnabled( false );
  mFontUnderlineBtn->setEnabled( false );
  mFontStrikethroughBtn->setEnabled( false );
  mFontBoldBtn->setEnabled( false );
  mFontItalicBtn->setEnabled( false );
  mFontSizeSpinBox->setEnabled( false );
  mBufferSizeSpinBox->setEnabled( false );
  mFontColorButton->setEnabled( false );
  mBufferColorButton->setEnabled( false );
  mBufferDrawChkbx->setEnabled( false );
  mLabelDistanceSpinBox->setEnabled( false );
  mXCoordSpinBox->setEnabled( false );
  mYCoordSpinBox->setEnabled( false );
  mMultiLineAlignComboBox->setEnabled( false );
  mHaliComboBox->setEnabled( false );
  mValiComboBox->setEnabled( false );
  mRotationSpinBox->setEnabled( false );
  mLabelAllPartsCheckBox->setEnabled( false );
}

void QgsLabelPropertyDialog::blockElementSignals( bool block )
{
  mShowLabelChkbx->blockSignals( block );
  mAlwaysShowChkbx->blockSignals( block );
  mShowCalloutChkbx->blockSignals( block );
  mMinScaleWidget->blockSignals( block );
  mMaxScaleWidget->blockSignals( block );
  mFontFamilyCmbBx->blockSignals( block );
  mFontStyleCmbBx->blockSignals( block );
  mFontUnderlineBtn->blockSignals( block );
  mFontStrikethroughBtn->blockSignals( block );
  mFontBoldBtn->blockSignals( block );
  mFontItalicBtn->blockSignals( block );
  mFontSizeSpinBox->blockSignals( block );
  mBufferSizeSpinBox->blockSignals( block );
  mFontColorButton->blockSignals( block );
  mBufferColorButton->blockSignals( block );
  mBufferDrawChkbx->blockSignals( block );
  mLabelDistanceSpinBox->blockSignals( block );
  mXCoordSpinBox->blockSignals( block );
  mYCoordSpinBox->blockSignals( block );
  mMultiLineAlignComboBox->blockSignals( block );
  mHaliComboBox->blockSignals( block );
  mValiComboBox->blockSignals( block );
  mRotationSpinBox->blockSignals( block );
  mLabelAllPartsCheckBox->blockSignals( block );
}

int QgsLabelPropertyDialog::dataDefinedColumnIndex( QgsPalLayerSettings::Property p, const QgsVectorLayer *vlayer, const QgsExpressionContext &context ) const
{
  if ( !mDataDefinedProperties.isActive( p ) )
    return -1;

  const QgsProperty property = mDataDefinedProperties.property( p );

  QString fieldName;
  switch ( property.propertyType() )
  {
    case QgsProperty::InvalidProperty:
    case QgsProperty::StaticProperty:
      break;

    case QgsProperty::FieldBasedProperty:
      fieldName = property.field();
      break;

    case QgsProperty::ExpressionBasedProperty:
    {
      // an expression based property may still be a effectively a single field reference in the map canvas context.
      // e.g. if it is a expression like '"some_field"', or 'case when @some_project_var = 'a' then "field_a" else "field_b" end'
      QgsExpression expression( property.expressionString() );
      if ( expression.prepare( &context ) )
      {
        const QgsExpressionNode *node = expression.rootNode()->effectiveNode();
        if ( node->nodeType() == QgsExpressionNode::ntColumnRef )
        {
          const QgsExpressionNodeColumnRef *columnRef = qgis::down_cast<const QgsExpressionNodeColumnRef *>( node );
          fieldName = columnRef->name();
        }
        // ok, it's not. But let's be super smart and helpful for users!
        // maybe it's a COALESCE("some field", 'some' || 'fallback' || 'expression') type expression, where the user wants to override
        // some labels with a value stored in a field but all others use some expression
        else if ( node->nodeType() == QgsExpressionNode::ntFunction )
        {
          const QgsExpressionNodeFunction *functionNode = qgis::down_cast<const QgsExpressionNodeFunction *>( node );
          if ( const QgsExpressionFunction *function = QgsExpression::QgsExpression::Functions()[functionNode->fnIndex()] )
          {
            if ( function->name() == QLatin1String( "coalesce" ) )
            {
              if ( const QgsExpressionNode *firstArg = functionNode->args()->list().value( 0 ) )
              {
                const QgsExpressionNode *firstArgNode = firstArg->effectiveNode();
                if ( firstArgNode->nodeType() == QgsExpressionNode::ntColumnRef )
                {
                  const QgsExpressionNodeColumnRef *columnRef = qgis::down_cast<const QgsExpressionNodeColumnRef *>( firstArgNode );
                  fieldName = columnRef->name();
                }
              }
            }
          }
        }
      }
      break;
    }
  }

  if ( !fieldName.isEmpty() )
    return vlayer->fields().lookupField( fieldName );
  return -1;
}

void QgsLabelPropertyDialog::setDataDefinedValues( QgsVectorLayer *vlayer )
{
  //loop through data defined properties and set all the GUI widget values. We can do this
  //even if the data defined property is set to an expression, as it's useful to show
  //users what the evaluated property is...

  QgsExpressionContext context;
  context << QgsExpressionContextUtils::globalScope()
          << QgsExpressionContextUtils::projectScope( QgsProject::instance() )
          << QgsExpressionContextUtils::atlasScope( nullptr )
          << QgsExpressionContextUtils::mapSettingsScope( QgisApp::instance()->mapCanvas()->mapSettings() )
          << QgsExpressionContextUtils::layerScope( vlayer );
  context.setFeature( mCurLabelFeat );

  const auto constPropertyKeys = mDataDefinedProperties.propertyKeys();
  for ( const int key : constPropertyKeys )
  {
    if ( !mDataDefinedProperties.isActive( key ) )
      continue;

    //TODO - pass expression context
    const QVariant result = mDataDefinedProperties.value( key, context );
    if ( !result.isValid() || result.isNull() )
    {
      //could not evaluate data defined value
      continue;
    }

    bool ok = false;
    switch ( key )
    {
      case QgsPalLayerSettings::Show:
      {
        const int showLabel = result.toInt( &ok );
        mShowLabelChkbx->setChecked( !ok || showLabel != 0 );
        break;
      }
      case QgsPalLayerSettings::AlwaysShow:
        mAlwaysShowChkbx->setChecked( result.toBool() );
        break;

      case QgsPalLayerSettings::CalloutDraw:
        mShowCalloutChkbx->setChecked( result.toBool() );
        break;

      case QgsPalLayerSettings::LabelAllParts:
        mLabelAllPartsCheckBox->setChecked( result.toBool() );
        break;

      case QgsPalLayerSettings::MinimumScale:
      {
        const double minScale = result.toDouble( &ok );
        if ( ok )
        {
          mMinScaleWidget->setScale( minScale );
        }
        break;
      }
      case QgsPalLayerSettings::MaximumScale:
      {
        const double maxScale = result.toDouble( &ok );
        if ( ok )
        {
          mMaxScaleWidget->setScale( maxScale );
        }
        break;
      }
      case QgsPalLayerSettings::BufferSize:
      {
        const double bufferSize = result.toDouble( &ok );
        if ( ok )
        {
          mBufferSizeSpinBox->setValue( bufferSize );
        }
        break;
      }
      case QgsPalLayerSettings::PositionX:
      {
        const double posX = result.toDouble( &ok );
        if ( ok )
        {
          mXCoordSpinBox->setValue( posX );
        }
        break;
      }
      case QgsPalLayerSettings::PositionY:
      {
        const double posY = result.toDouble( &ok );
        if ( ok )
        {
          mYCoordSpinBox->setValue( posY );
        }
        break;
      }
      case QgsPalLayerSettings::LabelDistance:
      {
        const double labelDist = result.toDouble( &ok );
        if ( ok )
        {
          mLabelDistanceSpinBox->setValue( labelDist );
        }
        break;
      }
      case QgsPalLayerSettings::MultiLineAlignment:
        mMultiLineAlignComboBox->setCurrentIndex( mMultiLineAlignComboBox->findData( result.toString(), Qt::UserRole, Qt::MatchFixedString ) );
        break;
      case QgsPalLayerSettings::Hali:
        mHaliComboBox->setCurrentIndex( mHaliComboBox->findData( result.toString(), Qt::UserRole, Qt::MatchFixedString ) );
        break;
      case QgsPalLayerSettings::Vali:
        mValiComboBox->setCurrentIndex( mValiComboBox->findData( result.toString(), Qt::UserRole, Qt::MatchFixedString ) );
        break;
      case QgsPalLayerSettings::BufferColor:
        mBufferColorButton->setColor( QColor( result.toString() ) );
        break;

      case QgsPalLayerSettings::BufferDraw:
        mBufferDrawChkbx->setChecked( result.toBool() );
        break;

      case QgsPalLayerSettings::Color:
        mFontColorButton->setColor( QColor( result.toString() ) );
        break;
      case QgsPalLayerSettings::LabelRotation:
      {
        const double rot = result.toDouble( &ok );
        if ( ok )
        {
          mRotationSpinBox->setValue( rot );
        }
        break;
      }

      case QgsPalLayerSettings::Size:
      {
        const double size = result.toDouble( &ok );
        if ( ok )
        {
          mFontSizeSpinBox->setValue( size );
        }
        else
        {
          mFontSizeSpinBox->setValue( 0 );
        }
        break;
      }
      default:
        break;
    }
  }
  enableWidgetsForPinnedLabels();
}

void QgsLabelPropertyDialog::enableDataDefinedWidgets( QgsVectorLayer *vlayer )
{
  QgsExpressionContext context = mCanvas->createExpressionContext();
  context.appendScope( vlayer->createExpressionContextScope() );

  //loop through data defined properties, this time setting whether or not the widgets are enabled
  //this can only be done for properties which are assigned to fields
  const auto constPropertyKeys = mDataDefinedProperties.propertyKeys();
  for ( const int key : constPropertyKeys )
  {
    const QgsProperty prop = mDataDefinedProperties.property( key );
    if ( !prop || !prop.isActive() )
    {
      continue;
    }

    const int ddIndex = dataDefinedColumnIndex( static_cast< QgsPalLayerSettings::Property >( key ), vlayer, context );
    mPropertyToFieldMap[ key ] = ddIndex;
    if ( ddIndex < 0 )
      continue; // can only modify attributes with an active data definition of a mapped field

    switch ( key )
    {
      case QgsPalLayerSettings::Show:
        mShowLabelChkbx->setEnabled( true );
        break;
      case QgsPalLayerSettings::AlwaysShow:
        mAlwaysShowChkbx->setEnabled( true );
        break;
      case QgsPalLayerSettings::MinimumScale:
        mMinScaleWidget->setEnabled( true );
        break;
      case QgsPalLayerSettings::MaximumScale:
        mMaxScaleWidget->setEnabled( true );
        break;
      case QgsPalLayerSettings::BufferSize:
        mBufferSizeSpinBox->setEnabled( true );
        break;
      case QgsPalLayerSettings::PositionX:
        mXCoordSpinBox->setEnabled( true );
        break;
      case QgsPalLayerSettings::PositionY:
        mYCoordSpinBox->setEnabled( true );
        break;
      case QgsPalLayerSettings::LabelDistance:
        mLabelDistanceSpinBox->setEnabled( true );
        break;
      case QgsPalLayerSettings::MultiLineAlignment:
        mMultiLineAlignComboBox->setEnabled( true );
        break;
      case QgsPalLayerSettings::Hali:
        mCanSetHAlignment = true;
        mHaliComboBox->setEnabled( mIsPinned );
        break;
      case QgsPalLayerSettings::Vali:
        mCanSetVAlignment = true;
        mValiComboBox->setEnabled( mIsPinned );
        break;
      case QgsPalLayerSettings::BufferColor:
        mBufferColorButton->setEnabled( true );
        break;
      case QgsPalLayerSettings::BufferDraw:
        mBufferDrawChkbx->setEnabled( true );
        break;
      case QgsPalLayerSettings::Color:
        mFontColorButton->setEnabled( true );
        break;
      case QgsPalLayerSettings::LabelRotation:
        mRotationSpinBox->setEnabled( true );
        break;
      //font related properties
      case QgsPalLayerSettings::Family:
        mFontFamilyCmbBx->setEnabled( true );
        break;
      case QgsPalLayerSettings::FontStyle:
        mFontStyleCmbBx->setEnabled( true );
        break;
      case QgsPalLayerSettings::Underline:
        mFontUnderlineBtn->setEnabled( true );
        break;
      case QgsPalLayerSettings::Strikeout:
        mFontStrikethroughBtn->setEnabled( true );
        break;
      case QgsPalLayerSettings::Bold:
        mFontBoldBtn->setEnabled( true );
        break;
      case QgsPalLayerSettings::Italic:
        mFontItalicBtn->setEnabled( true );
        break;
      case QgsPalLayerSettings::Size:
        mFontSizeSpinBox->setEnabled( true );
        break;
      case QgsPalLayerSettings::CalloutDraw:
        mShowCalloutChkbx->setEnabled( true );
        break;
      case QgsPalLayerSettings::LabelAllParts:
        mLabelAllPartsCheckBox->setEnabled( true );
        break;
      default:
        break;
    }
  }
}

void QgsLabelPropertyDialog::updateFont( const QFont &font, bool block )
{
  // update background reference font
  if ( font != mLabelFont )
  {
    mLabelFont = font;
  }

  if ( block )
    blockElementSignals( true );

  mFontFamilyCmbBx->setCurrentFont( mLabelFont );
  populateFontStyleComboBox();
  mFontUnderlineBtn->setChecked( mLabelFont.underline() );
  mFontStrikethroughBtn->setChecked( mLabelFont.strikeOut() );
  mFontBoldBtn->setChecked( mLabelFont.bold() );
  mFontItalicBtn->setChecked( mLabelFont.italic() );
  if ( block )
    blockElementSignals( false );
}

void QgsLabelPropertyDialog::populateFontStyleComboBox()
{
  mFontStyleCmbBx->clear();
  const auto constFamily = mFontDB.styles( mLabelFont.family() );
  for ( const QString &style : constFamily )
  {
    mFontStyleCmbBx->addItem( style );
  }

  int curIndx = 0;
  const int stylIndx = mFontStyleCmbBx->findText( mFontDB.styleString( mLabelFont ) );
  if ( stylIndx > -1 )
  {
    curIndx = stylIndx;
  }

  mFontStyleCmbBx->setCurrentIndex( curIndx );
}

void QgsLabelPropertyDialog::fillMultiLineAlignComboBox()
{
  mMultiLineAlignComboBox->addItem( tr( "Layer Default" ), "" );
  mMultiLineAlignComboBox->addItem( tr( "Left" ), "Left" );
  mMultiLineAlignComboBox->addItem( tr( "Center" ), "Center" );
  mMultiLineAlignComboBox->addItem( tr( "Right" ), "Right" );
  mMultiLineAlignComboBox->addItem( tr( "Justify" ), "Justify" );
}

void QgsLabelPropertyDialog::fillHaliComboBox()
{
  mHaliComboBox->addItem( tr( "Left" ), "Left" );
  mHaliComboBox->addItem( tr( "Center" ), "Center" );
  mHaliComboBox->addItem( tr( "Right" ), "Right" );
}

void QgsLabelPropertyDialog::fillValiComboBox()
{
  mValiComboBox->addItem( tr( "Bottom" ), "Bottom" );
  mValiComboBox->addItem( tr( "Base" ), "Base" );
  mValiComboBox->addItem( tr( "Half" ), "Half" );
  mValiComboBox->addItem( tr( "Cap" ), "Cap" );
  mValiComboBox->addItem( tr( "Top" ), "Top" );
}

void QgsLabelPropertyDialog::mShowLabelChkbx_toggled( bool chkd )
{
  insertChangedValue( QgsPalLayerSettings::Show, ( chkd ? 1 : 0 ) );
}

void QgsLabelPropertyDialog::mAlwaysShowChkbx_toggled( bool chkd )
{
  insertChangedValue( QgsPalLayerSettings::AlwaysShow, ( chkd ? 1 : 0 ) );
}

void QgsLabelPropertyDialog::labelAllPartsToggled( bool checked )
{
  insertChangedValue( QgsPalLayerSettings::LabelAllParts, ( checked ? 1 : 0 ) );
}

void QgsLabelPropertyDialog::showCalloutToggled( bool chkd )
{
  insertChangedValue( QgsPalLayerSettings::CalloutDraw, ( chkd ? 1 : 0 ) );
}

void QgsLabelPropertyDialog::bufferDrawToggled( bool chkd )
{
  insertChangedValue( QgsPalLayerSettings::BufferDraw, ( chkd ? 1 : 0 ) );
}

void QgsLabelPropertyDialog::minScaleChanged( double scale )
{
  insertChangedValue( QgsPalLayerSettings::MinimumScale, scale );
}

void QgsLabelPropertyDialog::maxScaleChanged( double scale )
{
  insertChangedValue( QgsPalLayerSettings::MaximumScale, scale );
}

void QgsLabelPropertyDialog::mLabelDistanceSpinBox_valueChanged( double d )
{
  QVariant distance( d );
  if ( d < 0 )
  {
    //null value so that distance is reset to default
    distance.clear();
  }
  insertChangedValue( QgsPalLayerSettings::LabelDistance, distance );
}

void QgsLabelPropertyDialog::mXCoordSpinBox_valueChanged( double d )
{
  QVariant x( d );
  if ( d < mXCoordSpinBox->minimum() + mXCoordSpinBox->singleStep() )
  {
    //null value
    x.clear();
  }

  insertChangedValue( QgsPalLayerSettings::PositionX, x );
  enableWidgetsForPinnedLabels();
}

void QgsLabelPropertyDialog::mYCoordSpinBox_valueChanged( double d )
{
  QVariant y( d );
  if ( d < mYCoordSpinBox->minimum() + mYCoordSpinBox->singleStep() )
  {
    //null value
    y.clear();
  }
  insertChangedValue( QgsPalLayerSettings::PositionY, y );
  enableWidgetsForPinnedLabels();
}

void QgsLabelPropertyDialog::mFontFamilyCmbBx_currentFontChanged( const QFont &f )
{
  mLabelFont.setFamily( f.family() );
  updateFont( mLabelFont );
  insertChangedValue( QgsPalLayerSettings::Family, f.family() );
}

void QgsLabelPropertyDialog::mFontStyleCmbBx_currentIndexChanged( const QString &text )
{
  QgsFontUtils::updateFontViaStyle( mLabelFont, text );
  updateFont( mLabelFont );
  insertChangedValue( QgsPalLayerSettings::FontStyle, text );
}

void QgsLabelPropertyDialog::mFontUnderlineBtn_toggled( bool ckd )
{
  mLabelFont.setUnderline( ckd );
  updateFont( mLabelFont );
  insertChangedValue( QgsPalLayerSettings::Underline, ckd );
}

void QgsLabelPropertyDialog::mFontStrikethroughBtn_toggled( bool ckd )
{
  mLabelFont.setStrikeOut( ckd );
  updateFont( mLabelFont );
  insertChangedValue( QgsPalLayerSettings::Strikeout, ckd );
}

void QgsLabelPropertyDialog::mFontBoldBtn_toggled( bool ckd )
{
  mLabelFont.setBold( ckd );
  updateFont( mLabelFont );
  insertChangedValue( QgsPalLayerSettings::Bold, ckd );
}

void QgsLabelPropertyDialog::mFontItalicBtn_toggled( bool ckd )
{
  mLabelFont.setItalic( ckd );
  updateFont( mLabelFont );
  insertChangedValue( QgsPalLayerSettings::Italic, ckd );
}

void QgsLabelPropertyDialog::mFontSizeSpinBox_valueChanged( double d )
{
  QVariant size( d );
  if ( d <= 0 )
  {
    //null value so that font size is reset to default
    size.clear();
  }
  insertChangedValue( QgsPalLayerSettings::Size, size );
}

void QgsLabelPropertyDialog::mBufferSizeSpinBox_valueChanged( double d )
{
  QVariant size( d );
  if ( d < 0 )
  {
    //null value so that size is reset to default
    size.clear();
  }
  insertChangedValue( QgsPalLayerSettings::BufferSize, size );
}

void QgsLabelPropertyDialog::mRotationSpinBox_valueChanged( double d )
{
  QVariant rotation( d );
  if ( d < 0 )
  {
    //null value so that size is reset to default
    rotation.clear();
  }
  insertChangedValue( QgsPalLayerSettings::LabelRotation, rotation );
}

void QgsLabelPropertyDialog::mFontColorButton_colorChanged( const QColor &color )
{
  insertChangedValue( QgsPalLayerSettings::Color, color.name() );
}

void QgsLabelPropertyDialog::mBufferColorButton_colorChanged( const QColor &color )
{
  insertChangedValue( QgsPalLayerSettings::BufferColor, color.name() );
}

void QgsLabelPropertyDialog::mMultiLineAlignComboBox_currentIndexChanged( const int index )
{
  insertChangedValue( QgsPalLayerSettings::MultiLineAlignment, mMultiLineAlignComboBox->itemData( index ) );
}

void QgsLabelPropertyDialog::mHaliComboBox_currentIndexChanged( const int index )
{
  insertChangedValue( QgsPalLayerSettings::Hali, mHaliComboBox->itemData( index ) );
}

void QgsLabelPropertyDialog::mValiComboBox_currentIndexChanged( const int index )
{
  insertChangedValue( QgsPalLayerSettings::Vali, mValiComboBox->itemData( index ) );
}

void QgsLabelPropertyDialog::mLabelTextLineEdit_textChanged( const QString &text )
{
  if ( mCurLabelField != -1 )
  {
    mChangedProperties.insert( mCurLabelField, text );
  }
}

void QgsLabelPropertyDialog::insertChangedValue( QgsPalLayerSettings::Property p, const QVariant &value )
{
  if ( mDataDefinedProperties.isActive( p ) )
  {
    const QgsProperty prop = mDataDefinedProperties.property( p );
    if ( const int index = mPropertyToFieldMap.value( p ); index >= 0 )
    {
      mChangedProperties.insert( index, value );
    }
  }
}

void QgsLabelPropertyDialog::enableWidgetsForPinnedLabels()
{
  const bool pinned = mXCoordSpinBox->value() >= ( mXCoordSpinBox->minimum() + mXCoordSpinBox->singleStep() ) &&
                      mYCoordSpinBox->value() >= ( mYCoordSpinBox->minimum() + mYCoordSpinBox->singleStep() );

  mHaliComboBox->setEnabled( pinned && mCanSetHAlignment );
  mValiComboBox->setEnabled( pinned && mCanSetVAlignment );

  if ( !pinned )
  {
    mHaliComboBox->setToolTip( tr( "Alignment can only be set for pinned labels" ) );
    mValiComboBox->setToolTip( tr( "Alignment can only be set for pinned labels" ) );
  }
  else
  {
    mHaliComboBox->setToolTip( QString() );
    mValiComboBox->setToolTip( QString() );
  }
}

void QgsLabelPropertyDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "working_with_vector/vector_properties.html#the-label-toolbar" ) );
}
