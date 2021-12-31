/***************************************************************************
                         qgslayoutlabelwidget.cpp
                         ------------------------
    begin                : October 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutlabelwidget.h"
#include "qgslayoutitemlabel.h"
#include "qgslayout.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgsguiutils.h"
#include "qgslayoutitemmap.h"
#include "qgsvectorlayer.h"
#include "qgsprojoperation.h"

#include <QColorDialog>
#include <QFontDialog>
#include <QWidget>
#include <QAction>
#include <QMenu>

QgsLayoutLabelWidget::QgsLayoutLabelWidget( QgsLayoutItemLabel *label )
  : QgsLayoutItemBaseWidget( nullptr, label )
  , mLabel( label )
{
  Q_ASSERT( mLabel );

  setupUi( this );
  connect( mHtmlCheckBox, &QCheckBox::stateChanged, this, &QgsLayoutLabelWidget::mHtmlCheckBox_stateChanged );
  connect( mTextEdit, &QPlainTextEdit::textChanged, this, &QgsLayoutLabelWidget::mTextEdit_textChanged );
  connect( mInsertExpressionButton, &QPushButton::clicked, this, &QgsLayoutLabelWidget::mInsertExpressionButton_clicked );
  connect( mMarginXDoubleSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutLabelWidget::mMarginXDoubleSpinBox_valueChanged );
  connect( mMarginYDoubleSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutLabelWidget::mMarginYDoubleSpinBox_valueChanged );
  connect( mCenterRadioButton, &QRadioButton::clicked, this, &QgsLayoutLabelWidget::mCenterRadioButton_clicked );
  connect( mLeftRadioButton, &QRadioButton::clicked, this, &QgsLayoutLabelWidget::mLeftRadioButton_clicked );
  connect( mRightRadioButton, &QRadioButton::clicked, this, &QgsLayoutLabelWidget::mRightRadioButton_clicked );
  connect( mTopRadioButton, &QRadioButton::clicked, this, &QgsLayoutLabelWidget::mTopRadioButton_clicked );
  connect( mBottomRadioButton, &QRadioButton::clicked, this, &QgsLayoutLabelWidget::mBottomRadioButton_clicked );
  connect( mMiddleRadioButton, &QRadioButton::clicked, this, &QgsLayoutLabelWidget::mMiddleRadioButton_clicked );
  setPanelTitle( tr( "Label Properties" ) );

  mFontButton->setMode( QgsFontButton::ModeTextRenderer );
  mFontButton->setDialogTitle( tr( "Label Font" ) );
  mFontButton->registerExpressionContextGenerator( this );

  //add widget for general composer item properties
  mItemPropertiesWidget = new QgsLayoutItemPropertiesWidget( this, label );
  mainLayout->addWidget( mItemPropertiesWidget );

  mMarginXDoubleSpinBox->setClearValue( 0.0 );
  mMarginYDoubleSpinBox->setClearValue( 0.0 );

  setGuiElementValues();
  connect( mLabel, &QgsLayoutObject::changed, this, &QgsLayoutLabelWidget::setGuiElementValues );

  connect( mFontButton, &QgsFontButton::changed, this, &QgsLayoutLabelWidget::fontChanged );
  connect( mJustifyRadioButton, &QRadioButton::clicked, this, &QgsLayoutLabelWidget::justifyClicked );

  mDynamicTextMenu = new QMenu( this );
  mDynamicTextButton->setMenu( mDynamicTextMenu );

  connect( mDynamicTextMenu, &QMenu::aboutToShow, this, [ = ]
  {
    mDynamicTextMenu->clear();
    if ( mLabel->layout() )
    {
      // we need to rebuild this on each show, as the content varies depending on other available items...
      buildInsertDynamicTextMenu( mLabel->layout(), mDynamicTextMenu, [ = ]( const QString & expression )
      {
        mLabel->beginCommand( tr( "Insert dynamic text" ) );
        mTextEdit->insertPlainText( "[%" + expression + "%]" );
        mLabel->endCommand();
      } );
    }
  } );

  QMenu *expressionMenu = new QMenu( this );
  QAction *convertToStaticAction = new QAction( tr( "Convert to Static Text" ), this );
  expressionMenu->addAction( convertToStaticAction );
  connect( convertToStaticAction, &QAction::triggered, mLabel, &QgsLayoutItemLabel::convertToStaticText );
  mInsertExpressionButton->setMenu( expressionMenu );

  mFontButton->setLayer( coverageLayer() );
  if ( mLabel->layout() )
  {
    connect( &mLabel->layout()->reportContext(), &QgsLayoutReportContext::layerChanged, mFontButton, &QgsFontButton::setLayer );
  }
}

void QgsLayoutLabelWidget::setMasterLayout( QgsMasterLayoutInterface *masterLayout )
{
  if ( mItemPropertiesWidget )
    mItemPropertiesWidget->setMasterLayout( masterLayout );
}

QgsExpressionContext QgsLayoutLabelWidget::createExpressionContext() const
{
  return mLabel->createExpressionContext();
}

void QgsLayoutLabelWidget::buildInsertDynamicTextMenu( QgsLayout *layout, QMenu *menu, const std::function<void ( const QString & )> &callback )
{
  Q_ASSERT( layout );
  auto addExpression = [&callback]( QMenu * menu, const QString & name, const QString & expression )
  {
    QAction *action = new QAction( name, menu );
    connect( action, &QAction::triggered, action, [callback, expression]
    {
      callback( expression );
    } );
    menu->addAction( action );
  };

  QMenu *dateMenu = new QMenu( tr( "Current Date" ), menu );
  for ( const std::pair< QString, QString > &expression :
        {
          std::make_pair( tr( "ISO Format (%1)" ).arg( QDateTime::currentDateTime().toString( QStringLiteral( "yyyy-MM-dd" ) ) ), QStringLiteral( "format_date(now(), 'yyyy-MM-dd')" ) ),
          std::make_pair( tr( "Day/Month/Year (%1)" ).arg( QDateTime::currentDateTime().toString( QStringLiteral( "dd/MM/yyyy" ) ) ), QStringLiteral( "format_date(now(), 'dd/MM/yyyy')" ) ),
          std::make_pair( tr( "Month/Day/Year (%1)" ).arg( QDateTime::currentDateTime().toString( QStringLiteral( "MM/dd/yyyy" ) ) ), QStringLiteral( "format_date(now(), 'MM/dd/yyyy')" ) ),
        } )
  {
    addExpression( dateMenu, expression.first, expression.second );
  }
  menu->addMenu( dateMenu );

  QMenu *mapsMenu = new QMenu( tr( "Map Properties" ), menu );
  QList< QgsLayoutItemMap * > maps;
  layout->layoutItems( maps );
  for ( QgsLayoutItemMap *map : std::as_const( maps ) )
  {
    // these expressions require the map to have a non-empty ID set
    if ( map->id().isEmpty() )
      continue;

    QMenu *mapMenu = new QMenu( map->displayName(), mapsMenu );
    for ( const std::pair< QString, QString > &expression :
          {
            std::make_pair( tr( "Scale (%1)" ).arg( map->scale() ), QStringLiteral( "item_variables('%1')['map_scale']" ).arg( map->id() ) ),
            std::make_pair( tr( "Rotation (%1)" ).arg( map->rotation() ), QStringLiteral( "item_variables('%1')['map_rotation']" ).arg( map->id() ) ),
          } )
    {
      addExpression( mapMenu, expression.first, expression.second );
    }
    mapMenu->addSeparator();
    for ( const std::pair< QString, QString > &expression :
          {
            std::make_pair( tr( "CRS Identifier (%1)" ).arg( map->crs().authid() ),  QStringLiteral( "item_variables('%1')['map_crs']" ).arg( map->id() ) ),
            std::make_pair( tr( "CRS Name (%1)" ).arg( map->crs().description() ),  QStringLiteral( "item_variables('%1')['map_crs_description']" ).arg( map->id() ) ),
            std::make_pair( tr( "Ellipsoid Name (%1)" ).arg( map->crs().ellipsoidAcronym() ),  QStringLiteral( "item_variables('%1')['map_crs_ellipsoid']" ).arg( map->id() ) ),
            std::make_pair( tr( "Units (%1)" ).arg( QgsUnitTypes::toString( map->crs().mapUnits() ) ),  QStringLiteral( "item_variables('%1')['map_units']" ).arg( map->id() ) ),
            std::make_pair( tr( "Projection (%1)" ).arg( map->crs().operation().description() ),  QStringLiteral( "item_variables('%1')['map_crs_projection']" ).arg( map->id() ) ),
          } )
    {
      addExpression( mapMenu, expression.first, expression.second );
    }
    mapMenu->addSeparator();

    const QgsRectangle mapExtent = map->extent();
    const QgsPointXY center = mapExtent.center();
    for ( const std::pair< QString, QString > &expression :
          {
            std::make_pair( tr( "Center (X) (%1)" ).arg( center.x() ),  QStringLiteral( "x(item_variables('%1')['map_extent_center'])" ).arg( map->id() ) ),
            std::make_pair( tr( "Center (Y) (%1)" ).arg( center.y() ),  QStringLiteral( "y(item_variables('%1')['map_extent_center'])" ).arg( map->id() ) ),
            std::make_pair( tr( "X Minimum (%1)" ).arg( mapExtent.xMinimum() ),  QStringLiteral( "x_min(item_variables('%1')['map_extent'])" ).arg( map->id() ) ),
            std::make_pair( tr( "Y Minimum (%1)" ).arg( mapExtent.yMinimum() ),  QStringLiteral( "y_min(item_variables('%1')['map_extent'])" ).arg( map->id() ) ),
            std::make_pair( tr( "X Maximum (%1)" ).arg( mapExtent.xMaximum() ),  QStringLiteral( "x_max(item_variables('%1')['map_extent'])" ).arg( map->id() ) ),
            std::make_pair( tr( "Y Maximum (%1)" ).arg( mapExtent.yMaximum() ),  QStringLiteral( "y_max(item_variables('%1')['map_extent'])" ).arg( map->id() ) ),
          } )
    {
      addExpression( mapMenu, expression.first, expression.second );
    }
    mapMenu->addSeparator();
    for ( const std::pair< QString, QString > &expression :
          {
            std::make_pair( tr( "Layer Credits" ),  QStringLiteral( "array_to_string(map_credits('%1'))" ).arg( map->id() ) ),
          } )
    {
      addExpression( mapMenu, expression.first, expression.second );
    }
    mapsMenu->addMenu( mapMenu );
  }
  menu->addMenu( mapsMenu );
  menu->addSeparator();

  if ( layout->reportContext().layer() )
  {
    const QgsFields fields = layout->reportContext().layer()->fields();

    QMenu *fieldsMenu = new QMenu( tr( "Field" ), menu );
    for ( const QgsField &field : fields )
    {
      addExpression( fieldsMenu, field.displayName(), QStringLiteral( "\"%1\"" ).arg( field.name() ) );
    }

    menu->addMenu( fieldsMenu );
    menu->addSeparator();
  }

  for ( const std::pair< QString, QString > &expression :
        {
          std::make_pair( tr( "Layout Name" ), QStringLiteral( "@layout_name" ) ),
          std::make_pair( tr( "Layout Page Number" ), QStringLiteral( "@layout_page" ) ),
          std::make_pair( tr( "Layout Page Count" ), QStringLiteral( "@layout_numpages" ) )
        } )
  {
    addExpression( menu, expression.first, expression.second );
  }
  menu->addSeparator();
  for ( const std::pair< QString, QString > &expression :
        {
          std::make_pair( tr( "Project Author" ), QStringLiteral( "@project_author" ) ),
          std::make_pair( tr( "Project Title" ), QStringLiteral( "@project_title" ) ),
          std::make_pair( tr( "Project Path" ), QStringLiteral( "@project_path" ) )
        } )
  {
    addExpression( menu, expression.first, expression.second );
  }
  menu->addSeparator();
  for ( const std::pair< QString, QString > &expression :
        {
          std::make_pair( tr( "Current User Name" ), QStringLiteral( "@user_full_name" ) ),
          std::make_pair( tr( "Current User Account" ), QStringLiteral( "@user_account_name" ) )
        } )
  {
    addExpression( menu, expression.first, expression.second );
  }
}

bool QgsLayoutLabelWidget::setNewItem( QgsLayoutItem *item )
{
  if ( item->type() != QgsLayoutItemRegistry::LayoutLabel )
    return false;

  if ( mLabel )
  {
    disconnect( mLabel, &QgsLayoutObject::changed, this, &QgsLayoutLabelWidget::setGuiElementValues );
  }

  mLabel = qobject_cast< QgsLayoutItemLabel * >( item );
  mItemPropertiesWidget->setItem( mLabel );

  if ( mLabel )
  {
    connect( mLabel, &QgsLayoutObject::changed, this, &QgsLayoutLabelWidget::setGuiElementValues );
  }

  setGuiElementValues();

  return true;
}

void QgsLayoutLabelWidget::mHtmlCheckBox_stateChanged( int state )
{
  if ( mLabel )
  {
    mVerticalAlignementLabel->setDisabled( state );
    mTopRadioButton->setDisabled( state );
    mMiddleRadioButton->setDisabled( state );
    mBottomRadioButton->setDisabled( state );

    mLabel->beginCommand( tr( "Change Label Mode" ) );
    mLabel->blockSignals( true );
    mLabel->setMode( state ? QgsLayoutItemLabel::ModeHtml : QgsLayoutItemLabel::ModeFont );
    mLabel->setText( mTextEdit->toPlainText() );
    mLabel->update();
    mLabel->blockSignals( false );
    mLabel->endCommand();
  }
}

void QgsLayoutLabelWidget::mTextEdit_textChanged()
{
  if ( mLabel )
  {
    mLabel->beginCommand( tr( "Change Label Text" ), QgsLayoutItem::UndoLabelText );
    mLabel->blockSignals( true );
    mLabel->setText( mTextEdit->toPlainText() );
    mLabel->update();
    mLabel->blockSignals( false );
    mLabel->endCommand();
  }
}

void QgsLayoutLabelWidget::fontChanged()
{
  if ( mLabel )
  {
    mLabel->beginCommand( tr( "Change Label Font" ), QgsLayoutItem::UndoLabelFont );
    mLabel->setTextFormat( mFontButton->textFormat() );
    mLabel->update();
    mLabel->endCommand();
  }
}

void QgsLayoutLabelWidget::justifyClicked()
{
  if ( mLabel )
  {
    mLabel->beginCommand( tr( "Change Label Alignment" ) );
    mLabel->setHAlign( Qt::AlignJustify );
    mLabel->update();
    mLabel->endCommand();
  }
}

void QgsLayoutLabelWidget::mMarginXDoubleSpinBox_valueChanged( double d )
{
  if ( mLabel )
  {
    mLabel->beginCommand( tr( "Change Label Margin" ), QgsLayoutItem::UndoLabelMargin );
    mLabel->setMarginX( d );
    mLabel->update();
    mLabel->endCommand();
  }
}

void QgsLayoutLabelWidget::mMarginYDoubleSpinBox_valueChanged( double d )
{
  if ( mLabel )
  {
    mLabel->beginCommand( tr( "Change Label Margin" ), QgsLayoutItem::UndoLabelMargin );
    mLabel->setMarginY( d );
    mLabel->update();
    mLabel->endCommand();
  }
}

void QgsLayoutLabelWidget::mInsertExpressionButton_clicked()
{
  if ( !mLabel )
  {
    return;
  }

  QString selText = mTextEdit->textCursor().selectedText();

  // html editor replaces newlines with Paragraph Separator characters - see https://github.com/qgis/QGIS/issues/27568
  selText = selText.replace( QChar( 0x2029 ), QChar( '\n' ) );

  // edit the selected expression if there's one
  if ( selText.startsWith( QLatin1String( "[%" ) ) && selText.endsWith( QLatin1String( "%]" ) ) )
    selText = selText.mid( 2, selText.size() - 4 );

  // use the atlas coverage layer, if any
  QgsVectorLayer *layer = coverageLayer();

  QgsExpressionContext context = mLabel->createExpressionContext();
  QgsExpressionBuilderDialog exprDlg( layer, selText, this, QStringLiteral( "generic" ), context );

  exprDlg.setWindowTitle( tr( "Insert Expression" ) );
  if ( exprDlg.exec() == QDialog::Accepted )
  {
    QString expression = exprDlg.expressionText();
    if ( !expression.isEmpty() )
    {
      mLabel->beginCommand( tr( "Insert expression" ) );
      mTextEdit->insertPlainText( "[%" + expression + "%]" );
      mLabel->endCommand();
    }
  }
}

void QgsLayoutLabelWidget::mCenterRadioButton_clicked()
{
  if ( mLabel )
  {
    mLabel->beginCommand( tr( "Change Label Alignment" ) );
    mLabel->setHAlign( Qt::AlignHCenter );
    mLabel->update();
    mLabel->endCommand();
  }
}

void QgsLayoutLabelWidget::mRightRadioButton_clicked()
{
  if ( mLabel )
  {
    mLabel->beginCommand( tr( "Change Label Alignment" ) );
    mLabel->setHAlign( Qt::AlignRight );
    mLabel->update();
    mLabel->endCommand();
  }
}

void QgsLayoutLabelWidget::mLeftRadioButton_clicked()
{
  if ( mLabel )
  {
    mLabel->beginCommand( tr( "Change Label Alignment" ) );
    mLabel->setHAlign( Qt::AlignLeft );
    mLabel->update();
    mLabel->endCommand();
  }
}

void QgsLayoutLabelWidget::mTopRadioButton_clicked()
{
  if ( mLabel )
  {
    mLabel->beginCommand( tr( "Change Label Alignment" ) );
    mLabel->setVAlign( Qt::AlignTop );
    mLabel->update();
    mLabel->endCommand();
  }
}

void QgsLayoutLabelWidget::mBottomRadioButton_clicked()
{
  if ( mLabel )
  {
    mLabel->beginCommand( tr( "Change Label Alignment" ) );
    mLabel->setVAlign( Qt::AlignBottom );
    mLabel->update();
    mLabel->endCommand();
  }
}

void QgsLayoutLabelWidget::mMiddleRadioButton_clicked()
{
  if ( mLabel )
  {
    mLabel->beginCommand( tr( "Change Label Alignment" ) );
    mLabel->setVAlign( Qt::AlignVCenter );
    mLabel->update();
    mLabel->endCommand();
  }
}

void QgsLayoutLabelWidget::setGuiElementValues()
{
  blockAllSignals( true );
  mTextEdit->setPlainText( mLabel->text() );
  mTextEdit->moveCursor( QTextCursor::End, QTextCursor::MoveAnchor );
  mMarginXDoubleSpinBox->setValue( mLabel->marginX() );
  mMarginYDoubleSpinBox->setValue( mLabel->marginY() );
  mHtmlCheckBox->setChecked( mLabel->mode() == QgsLayoutItemLabel::ModeHtml );
  mTopRadioButton->setChecked( mLabel->vAlign() == Qt::AlignTop );
  mMiddleRadioButton->setChecked( mLabel->vAlign() == Qt::AlignVCenter );
  mBottomRadioButton->setChecked( mLabel->vAlign() == Qt::AlignBottom );
  mLeftRadioButton->setChecked( mLabel->hAlign() == Qt::AlignLeft );
  mJustifyRadioButton->setChecked( mLabel->hAlign() == Qt::AlignJustify );
  mCenterRadioButton->setChecked( mLabel->hAlign() == Qt::AlignHCenter );
  mRightRadioButton->setChecked( mLabel->hAlign() == Qt::AlignRight );
  mFontButton->setTextFormat( mLabel->textFormat() );
  mVerticalAlignementLabel->setDisabled( mLabel->mode() == QgsLayoutItemLabel::ModeHtml );
  mTopRadioButton->setDisabled( mLabel->mode() == QgsLayoutItemLabel::ModeHtml );
  mMiddleRadioButton->setDisabled( mLabel->mode() == QgsLayoutItemLabel::ModeHtml );
  mBottomRadioButton->setDisabled( mLabel->mode() == QgsLayoutItemLabel::ModeHtml );

  blockAllSignals( false );
}

void QgsLayoutLabelWidget::blockAllSignals( bool block )
{
  mTextEdit->blockSignals( block );
  mHtmlCheckBox->blockSignals( block );
  mMarginXDoubleSpinBox->blockSignals( block );
  mMarginYDoubleSpinBox->blockSignals( block );
  mTopRadioButton->blockSignals( block );
  mMiddleRadioButton->blockSignals( block );
  mBottomRadioButton->blockSignals( block );
  mLeftRadioButton->blockSignals( block );
  mCenterRadioButton->blockSignals( block );
  mRightRadioButton->blockSignals( block );
  mJustifyRadioButton->blockSignals( block );
  mFontButton->blockSignals( block );
}
