/***************************************************************************
    qgscodeeditoroptions.cpp
    -------------------------
    begin                : September 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscodeeditoroptions.h"
#include "qgsapplication.h"
#include "qgssettings.h"
#include "qgis.h"

//
// QgsCodeEditorOptionsWidget
//

QgsCodeEditorOptionsWidget::QgsCodeEditorOptionsWidget( QWidget *parent )
  : QgsOptionsPageWidget( parent )
{
  setupUi( this );

  layout()->setContentsMargins( 0, 0, 0, 0 );

  mSizeSpin->setClearValue( 10 );

  mColorButtonMap =
  {
    {QgsCodeEditor::ColorRole::Default, mColorDefault },
    {QgsCodeEditor::ColorRole::Keyword, mColorKeyword },
    {QgsCodeEditor::ColorRole::Class, mColorClass },
    {QgsCodeEditor::ColorRole::Method, mColorFunction },
    {QgsCodeEditor::ColorRole::Decoration, mColorDecorator },
    {QgsCodeEditor::ColorRole::Number, mColorNumber },
    {QgsCodeEditor::ColorRole::Comment, mColorComment },
    {QgsCodeEditor::ColorRole::CommentLine, mColorCommentLine },
    {QgsCodeEditor::ColorRole::CommentBlock, mColorCommentBlock },
    {QgsCodeEditor::ColorRole::Background, mColorBackground },
    {QgsCodeEditor::ColorRole::Cursor, mColorCursor },
    {QgsCodeEditor::ColorRole::CaretLine, mColorCaretLine },
    {QgsCodeEditor::ColorRole::Operator, mColorOperator },
    {QgsCodeEditor::ColorRole::QuotedOperator, mColorQuotedOperator },
    {QgsCodeEditor::ColorRole::Identifier, mColorIdentifier },
    {QgsCodeEditor::ColorRole::QuotedIdentifier, mColorQuotedIdentifier },
    {QgsCodeEditor::ColorRole::Tag, mColorTag },
    {QgsCodeEditor::ColorRole::UnknownTag, mColorUnknownTag },
    {QgsCodeEditor::ColorRole::SingleQuote, mColorSingleQuote },
    {QgsCodeEditor::ColorRole::DoubleQuote, mColorDoubleQuote },
    {QgsCodeEditor::ColorRole::TripleSingleQuote, mColorTripleSingleQuote },
    {QgsCodeEditor::ColorRole::TripleDoubleQuote, mColorTripleDoubleQuote },
    {QgsCodeEditor::ColorRole::MarginBackground, mColorMarginBackground },
    {QgsCodeEditor::ColorRole::MarginForeground, mColorMarginForeground },
    {QgsCodeEditor::ColorRole::SelectionBackground, mColorSelectionBackground },
    {QgsCodeEditor::ColorRole::SelectionForeground, mColorSelectionForeground },
    {QgsCodeEditor::ColorRole::MatchedBraceBackground, mColorBraceBackground },
    {QgsCodeEditor::ColorRole::MatchedBraceForeground, mColorBraceForeground },
    {QgsCodeEditor::ColorRole::Edge, mColorEdge },
    {QgsCodeEditor::ColorRole::Fold, mColorFold },
    {QgsCodeEditor::ColorRole::Error, mColorError },
  };

  for ( auto it = mColorButtonMap.constBegin(); it != mColorButtonMap.constEnd(); ++it )
  {
    it.value()->setColor( QgsCodeEditor::color( it.key() ) );
  }

  mColorSchemeComboBox->addItem( tr( "Default" ), QString() );
  mColorSchemeComboBox->addItem( tr( "Solarized (Light)" ), QStringLiteral( "solarized" ) );
  mColorSchemeComboBox->addItem( tr( "Solarized (Dark)" ),  QStringLiteral( "solarized_dark" ) );
  mColorSchemeComboBox->addItem( tr( "Custom" ), QStringLiteral( "custom" ) );

  QgsSettings settings;
  if ( !settings.value( QStringLiteral( "codeEditor/overrideColors" ), false, QgsSettings::Gui ).toBool() )
  {
    const QString theme = settings.value( QStringLiteral( "codeEditor/colorScheme" ), QString(), QgsSettings::Gui ).toString();
    mColorSchemeComboBox->setCurrentIndex( mColorSchemeComboBox->findData( theme ) );
  }
  else
  {
    mColorSchemeComboBox->setCurrentIndex( mColorSchemeComboBox->findData( QStringLiteral( "custom" ) ) );
  }

  connect( mColorSchemeComboBox, qgis::overload<int>::of( &QComboBox::currentIndexChanged ), this, [ = ]
  {
    const QString theme = mColorSchemeComboBox->currentData().toString();
    if ( theme != QStringLiteral( "custom" ) )
    {
      mBlockCustomColorChange = true;
      for ( auto it = mColorButtonMap.constBegin(); it != mColorButtonMap.constEnd(); ++it )
      {
        it.value()->setColor( QgsCodeEditor::defaultColor( it.key(), theme ) );
      }
      mBlockCustomColorChange = false;
    }
  } );

  for ( auto it = mColorButtonMap.constBegin(); it != mColorButtonMap.constEnd(); ++it )
  {
    connect( it.value(), &QgsColorButton::colorChanged, this, [ = ]
    {
      if ( mBlockCustomColorChange )
        return;

      mColorSchemeComboBox->setCurrentIndex( mColorSchemeComboBox->findData( QStringLiteral( "custom" ) ) );
    } );
  }

  // font
  const QFont font = QgsCodeEditor::getMonospaceFont();
  mFontComboBox->setCurrentFont( font );
  mSizeSpin->setValue( font.pointSize() );
  mOverrideFontGroupBox->setChecked( !settings.value( QStringLiteral( "codeEditor/fontfamily" ), QString(), QgsSettings::Gui ).toString().isEmpty() );
}

QString QgsCodeEditorOptionsWidget::helpKey() const
{
  return QStringLiteral( "introduction/qgis_configuration.html#code-editor-settings" );
}

void QgsCodeEditorOptionsWidget::apply()
{
  const QString theme = mColorSchemeComboBox->currentData().toString();

  QgsSettings settings;
  const bool customTheme = theme == QStringLiteral( "custom" );
  settings.setValue( QStringLiteral( "codeEditor/overrideColors" ), customTheme, QgsSettings::Gui );
  if ( !customTheme )
  {
    settings.setValue( QStringLiteral( "codeEditor/colorScheme" ), theme, QgsSettings::Gui );
  }
  for ( auto it = mColorButtonMap.constBegin(); it != mColorButtonMap.constEnd(); ++it )
  {
    QgsCodeEditor::setColor( it.key(), it.value()->color() );
  }

  if ( mOverrideFontGroupBox->isChecked() )
  {
    settings.setValue( QStringLiteral( "codeEditor/fontfamily" ), mFontComboBox->currentFont().family(), QgsSettings::Gui );
    settings.setValue( QStringLiteral( "codeEditor/fontsize" ), mSizeSpin->value(), QgsSettings::Gui );
  }
  else
  {
    settings.remove( QStringLiteral( "codeEditor/fontfamily" ), QgsSettings::Gui );
    settings.remove( QStringLiteral( "codeEditor/fontsize" ), QgsSettings::Gui );
  }
}

//
// QgsCodeEditorOptionsFactory
//
QgsCodeEditorOptionsFactory::QgsCodeEditorOptionsFactory()
  : QgsOptionsWidgetFactory( tr( "Code Editor" ), QIcon() )
{

}

QIcon QgsCodeEditorOptionsFactory::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconCodeEditor.svg" ) );
}

QgsOptionsPageWidget *QgsCodeEditorOptionsFactory::createWidget( QWidget *parent ) const
{
  return new QgsCodeEditorOptionsWidget( parent );
}
