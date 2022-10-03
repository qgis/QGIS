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
#include "qgsgui.h"
#include "qgscodeeditorcolorschemeregistry.h"

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
    {QgsCodeEditorColorScheme::ColorRole::Default, mColorDefault },
    {QgsCodeEditorColorScheme::ColorRole::Keyword, mColorKeyword },
    {QgsCodeEditorColorScheme::ColorRole::Class, mColorClass },
    {QgsCodeEditorColorScheme::ColorRole::Method, mColorFunction },
    {QgsCodeEditorColorScheme::ColorRole::Decoration, mColorDecorator },
    {QgsCodeEditorColorScheme::ColorRole::Number, mColorNumber },
    {QgsCodeEditorColorScheme::ColorRole::Comment, mColorComment },
    {QgsCodeEditorColorScheme::ColorRole::CommentLine, mColorCommentLine },
    {QgsCodeEditorColorScheme::ColorRole::CommentBlock, mColorCommentBlock },
    {QgsCodeEditorColorScheme::ColorRole::Background, mColorBackground },
    {QgsCodeEditorColorScheme::ColorRole::Cursor, mColorCursor },
    {QgsCodeEditorColorScheme::ColorRole::CaretLine, mColorCaretLine },
    {QgsCodeEditorColorScheme::ColorRole::Operator, mColorOperator },
    {QgsCodeEditorColorScheme::ColorRole::QuotedOperator, mColorQuotedOperator },
    {QgsCodeEditorColorScheme::ColorRole::Identifier, mColorIdentifier },
    {QgsCodeEditorColorScheme::ColorRole::QuotedIdentifier, mColorQuotedIdentifier },
    {QgsCodeEditorColorScheme::ColorRole::Tag, mColorTag },
    {QgsCodeEditorColorScheme::ColorRole::UnknownTag, mColorUnknownTag },
    {QgsCodeEditorColorScheme::ColorRole::SingleQuote, mColorSingleQuote },
    {QgsCodeEditorColorScheme::ColorRole::DoubleQuote, mColorDoubleQuote },
    {QgsCodeEditorColorScheme::ColorRole::TripleSingleQuote, mColorTripleSingleQuote },
    {QgsCodeEditorColorScheme::ColorRole::TripleDoubleQuote, mColorTripleDoubleQuote },
    {QgsCodeEditorColorScheme::ColorRole::MarginBackground, mColorMarginBackground },
    {QgsCodeEditorColorScheme::ColorRole::MarginForeground, mColorMarginForeground },
    {QgsCodeEditorColorScheme::ColorRole::SelectionBackground, mColorSelectionBackground },
    {QgsCodeEditorColorScheme::ColorRole::SelectionForeground, mColorSelectionForeground },
    {QgsCodeEditorColorScheme::ColorRole::MatchedBraceBackground, mColorBraceBackground },
    {QgsCodeEditorColorScheme::ColorRole::MatchedBraceForeground, mColorBraceForeground },
    {QgsCodeEditorColorScheme::ColorRole::Edge, mColorEdge },
    {QgsCodeEditorColorScheme::ColorRole::Fold, mColorFold },
    {QgsCodeEditorColorScheme::ColorRole::Error, mColorError },
    {QgsCodeEditorColorScheme::ColorRole::ErrorBackground, mColorErrorBackground },
    {QgsCodeEditorColorScheme::ColorRole::FoldIconForeground, mColorFoldIcon },
    {QgsCodeEditorColorScheme::ColorRole::FoldIconHalo, mColorFoldIconHalo },
    {QgsCodeEditorColorScheme::ColorRole::IndentationGuide, mColorIndentation },
  };

  for ( auto it = mColorButtonMap.constBegin(); it != mColorButtonMap.constEnd(); ++it )
  {
    it.value()->setColor( QgsCodeEditor::color( it.key() ) );
    it.value()->setDefaultColor( it.value()->color() );
  }

  mColorSchemeComboBox->addItem( tr( "Default" ), QString() );

  QMap< QString, QString> themeNameToId;
  QStringList names;
  const QStringList ids = QgsGui::codeEditorColorSchemeRegistry()->schemes();
  for ( const QString &id : ids )
  {
    if ( id == QLatin1String( "default" ) )
      continue;

    const QString name = QgsGui::codeEditorColorSchemeRegistry()->scheme( id ).name();
    names << name;
    themeNameToId.insert( name, id );
  }

  std::sort( names.begin(), names.end() );
  for ( const QString &name : std::as_const( names ) )
  {
    mColorSchemeComboBox->addItem( name, themeNameToId.value( name ) );
  }

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

  connect( mColorSchemeComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, [ = ]
  {
    const QString theme = mColorSchemeComboBox->currentData().toString();
    if ( theme != QLatin1String( "custom" ) )
    {
      mBlockCustomColorChange = true;
      for ( auto it = mColorButtonMap.constBegin(); it != mColorButtonMap.constEnd(); ++it )
      {
        it.value()->setColor( QgsCodeEditor::defaultColor( it.key(), theme ) );
        it.value()->setDefaultColor( it.value()->color() );
      }
      mBlockCustomColorChange = false;
    }

    updatePreview();
  } );

  for ( auto it = mColorButtonMap.constBegin(); it != mColorButtonMap.constEnd(); ++it )
  {
    connect( it.value(), &QgsColorButton::colorChanged, this, [ = ]
    {
      if ( mBlockCustomColorChange )
        return;

      mColorSchemeComboBox->setCurrentIndex( mColorSchemeComboBox->findData( QStringLiteral( "custom" ) ) );
      updatePreview();
    } );
  }

  // font
  const QFont font = QgsCodeEditor::getMonospaceFont();
  mFontComboBox->setCurrentFont( font );
  mSizeSpin->setValue( font.pointSize() );
  mOverrideFontGroupBox->setChecked( !settings.value( QStringLiteral( "codeEditor/fontfamily" ), QString(), QgsSettings::Gui ).toString().isEmpty() );

  connect( mFontComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, [ = ]
  {
    updatePreview();
  } );
  connect( mSizeSpin, qOverload<int>( &QSpinBox::valueChanged ), this, [ = ]
  {
    updatePreview();
  } );
  connect( mOverrideFontGroupBox, &QGroupBox::toggled, this, [ = ]
  {
    updatePreview();
  } );

  mListLanguage->addItem( tr( "Python" ) );
  mListLanguage->addItem( tr( "QGIS Expression" ) );
  mListLanguage->addItem( tr( "SQL" ) );
  mListLanguage->addItem( tr( "HTML" ) );
  mListLanguage->addItem( tr( "CSS" ) );
  mListLanguage->addItem( tr( "JavaScript" ) );
  mListLanguage->addItem( tr( "R" ) );

  connect( mListLanguage, &QListWidget::currentRowChanged, this, [ = ]
  {
    mPreviewStackedWidget->setCurrentIndex( mListLanguage->currentRow() );
  } );

  mPythonPreview->setText( R"""(def simple_function(x,y,z):
    """
    Function docstring
    """
    return [1, 1.2, "val", 'a string', {'a': True, 'b': False}]

@my_decorator
def somefunc(param1: str='', param2=0):
    '''A docstring'''
    if param1 > param2: # interesting
        print('Gre\'ater'.lower())
    return (param2 - param1 + 1 + 0b10l) or None

class SomeClass:
    """
    My class docstring
    """
    pass
)""" );

  mExpressionPreview->setText( R"""(aggregate(layer:='rail_stations',
    aggregate:='collect', -- a comment
    expression:=centroid($geometry), /* a comment */
    filter:="region_name" = attribute(@parent,'name') + 55
)
)""");

  mSQLPreview->setText( R"""(CREATE TABLE "my_table" (
    "pk" serial NOT NULL PRIMARY KEY,
    "a_field" integer,
    "another_field" varchar(255)
);

-- Retrieve values
SELECT count(*) FROM "my_table" WHERE "a_field" > 'a value';
)""");

  mHtmlPreview->setText(R"""(<html>
  <head>
    <title>QGIS</title>
  </head>
  <body>
    <h1>QGIS Rocks!</h1>
    <img src="qgis.png" style="width: 100px" />
    <!--Sample comment-->
    <p>Sample paragraph</p>
  </body>
</html>
)""");

  mCssPreview->setText( R"""(@import url(print.css);

@font-face {
 font-family: DroidSans; /* A comment */
 src: url('DroidSans.ttf');
}

p.style_name:lang(en) {
 color: #F0F0F0;
 background: #600;
}

ul > li, a:hover {
 line-height: 11px;
 text-decoration: underline;
}

@media print {
  a[href^=http]::after {
    content: attr(href)
  }
}
)""" );

  mJsPreview->setText( R"""(// my sample JavaScript function

window.onAction(function update() {
    /* Do some work */
    var prevPos = closure.pos;

    element.width = 100;
    element.height = 2500;
    element.name = 'a string';
    element.title= "another string";

    if (prevPos.x > 100) {
        element.x += max(100*2, 100);
    }
});)""" );

  mRPreview->setText( R"""(# a comment
x <- 1:12
sample(x)
sample(x, replace = TRUE)

resample <- function(x, ...) x[sample.int(length(x), ...)]
resample(x[x >  8]) # length 2

a_variable <- "My string"

`%func_name%` <- function(arg_1,arg_2) {
  # function body
}

`%pwr%` <- function(x,y)
{
 return(x^y)
}
)""");

  mListLanguage->setCurrentRow( 0 );
  mPreviewStackedWidget->setCurrentIndex( 0 );

  mSplitter->restoreState( settings.value( QStringLiteral( "Windows/CodeEditorOptions/splitterState" ) ).toByteArray() );
}

QgsCodeEditorOptionsWidget::~QgsCodeEditorOptionsWidget()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/CodeEditorOptions/splitterState" ), mSplitter->saveState() );
}

QString QgsCodeEditorOptionsWidget::helpKey() const
{
  return QStringLiteral( "introduction/qgis_configuration.html#code-editor-settings" );
}

void QgsCodeEditorOptionsWidget::apply()
{
  const QString theme = mColorSchemeComboBox->currentData().toString();

  QgsSettings settings;
  const bool customTheme = theme == QLatin1String( "custom" );
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

void QgsCodeEditorOptionsWidget::updatePreview()
{
  QString theme = mColorSchemeComboBox->currentData().toString();


  QMap< QgsCodeEditorColorScheme::ColorRole, QColor> colors;
  if ( theme == QLatin1String( "custom" ) )
  {
    for ( auto it = mColorButtonMap.constBegin(); it != mColorButtonMap.constEnd(); ++it )
    {
      colors[ it.key() ] = it.value()->color();
    }
    theme.clear();
  }

  QString fontFamily;
  int fontSize = 0;
  if ( mOverrideFontGroupBox->isChecked() )
  {
    fontFamily = mFontComboBox->currentFont().family();
    fontSize = mSizeSpin->value();
  }

  mPythonPreview->setCustomAppearance( theme, colors, fontFamily, fontSize );
  mExpressionPreview->setCustomAppearance( theme, colors, fontFamily, fontSize );
  mSQLPreview->setCustomAppearance( theme, colors, fontFamily, fontSize );
  mHtmlPreview->setCustomAppearance( theme, colors, fontFamily, fontSize );
  mCssPreview->setCustomAppearance( theme, colors, fontFamily, fontSize );
  mJsPreview->setCustomAppearance( theme, colors, fontFamily, fontSize );
  mRPreview->setCustomAppearance( theme, colors, fontFamily, fontSize );
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

QStringList QgsCodeEditorOptionsFactory::path() const
{
  return {QStringLiteral( "ide" ) };
}

QString QgsCodeEditorOptionsFactory::pagePositionHint() const
{
  return QStringLiteral( "consoleOptions" );
}
