/***************************************************************************
                         qgsgrassmoduleparam.cpp
                             -------------------
    begin                : August, 2015
    copyright            : (C) 2015 by Radim Blazek
    email                : radim.blazek@gmail.com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgrassmoduleparam.h"

#include "qgis.h"
#include "qgsdatasourceuri.h"
#include "qgsgrass.h"
#include "qgsgrassfeatureiterator.h"
#include "qgsgrassmodule.h"
#include "qgsgrassmoduleinput.h"
#include "qgsgrassplugin.h"
#include "qgsgrassprovider.h"
#include "qgslogger.h"
#include "qgsmaplayer.h"
#include "qgsproject.h"
#include "qgsrasterlayer.h"
#include "qgssettings.h"
#include "qgsvectorlayer.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionValidator>

#include "moc_qgsgrassmoduleparam.cpp"

#if 0
extern "C"
{
#include <grass/vector.h>
}
#endif

/********************** QgsGrassModuleParam *************************/
QgsGrassModuleParam::QgsGrassModuleParam( QgsGrassModule *module, QString key, QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode, bool direct )
  : mModule( module )
  , mKey( key )
  , mDirect( direct )
{
  Q_UNUSED( gdesc )
  //mAnswer = qdesc.attribute("answer", "");

  if ( !qdesc.attribute( u"answer"_s ).isNull() )
  {
    mAnswer = qdesc.attribute( u"answer"_s ).trimmed();
  }
  else
  {
    QDomNode n = gnode.namedItem( u"default"_s );
    if ( !n.isNull() )
    {
      QDomElement e = n.toElement();
      mAnswer = e.text().trimmed();
    }
  }

  if ( qdesc.attribute( u"hidden"_s ) == "yes"_L1 )
  {
    mHidden = true;
  }

  QString label, description;
  if ( !qdesc.attribute( u"label"_s ).isEmpty() )
  {
    label = QApplication::translate( "grasslabel", qdesc.attribute( u"label"_s ).trimmed().toUtf8() );
  }
  if ( label.isEmpty() )
  {
    QDomNode n = gnode.namedItem( u"label"_s );
    if ( !n.isNull() )
    {
      QDomElement e = n.toElement();
      label = module->translate( e.text() );
    }
  }
  QDomNode n = gnode.namedItem( u"description"_s );
  if ( !n.isNull() )
  {
    QDomElement e = n.toElement();
    description = module->translate( e.text() );
  }

  if ( !label.isEmpty() )
  {
    mTitle = label;
    mToolTip = description;
  }
  else
  {
    mTitle = description;
  }

  mRequired = gnode.toElement().attribute( u"required"_s ) == "yes"_L1;

  mMultiple = gnode.toElement().attribute( u"multiple"_s ) == "yes"_L1;

  mId = qdesc.attribute( u"id"_s );
}

bool QgsGrassModuleParam::hidden() const
{
  return mHidden;
}

QStringList QgsGrassModuleParam::options()
{
  return QStringList();
}

QString QgsGrassModuleParam::getDescPrompt( QDomElement descDomElement, const QString &name )
{
  QDomNode gispromptNode = descDomElement.namedItem( u"gisprompt"_s );

  if ( !gispromptNode.isNull() )
  {
    QDomElement gispromptElement = gispromptNode.toElement();
    if ( !gispromptElement.isNull() )
    {
      return gispromptElement.attribute( name );
    }
  }
  return QString();
}

QDomNode QgsGrassModuleParam::nodeByKey( QDomElement descDomElement, QString key )
{
  QgsDebugMsgLevel( "called with key=" + key, 3 );
  QDomNode n = descDomElement.firstChild();

  while ( !n.isNull() )
  {
    QDomElement e = n.toElement();

    if ( !e.isNull() )
    {
      if ( e.tagName() == "parameter"_L1 || e.tagName() == "flag"_L1 )
      {
        if ( e.attribute( u"name"_s ) == key )
        {
          return n;
        }
      }
    }
    n = n.nextSibling();
  }

  return QDomNode();
}

QList<QDomNode> QgsGrassModuleParam::nodesByType( QDomElement descDomElement, STD_OPT optionType, const QString &age )
{
  // TODO: never tested
  QList<QDomNode> nodes;

  // Not all options have prompt set, for example G_OPT_V_TYPE and G_OPT_V_FIELD, which would be useful, don't have prompt
  QMap<QString, STD_OPT> typeMap;
  typeMap.insert( u"dbtable"_s, G_OPT_DB_TABLE );
  typeMap.insert( u"dbdriver"_s, G_OPT_DB_DRIVER );
  typeMap.insert( u"dbname"_s, G_OPT_DB_DATABASE );
  typeMap.insert( u"dbcolumn"_s, G_OPT_DB_COLUMN );
  typeMap.insert( u"vector"_s, G_OPT_V_INPUT );

  QDomNode n = descDomElement.firstChild();

  while ( !n.isNull() )
  {
    QString prompt = getDescPrompt( n.toElement(), u"prompt"_s );
    if ( typeMap.value( prompt ) == optionType )
    {
      if ( age.isEmpty() || getDescPrompt( n.toElement(), u"age"_s ) == age )
      {
        nodes << n;
      }
    }

    n = n.nextSibling();
  }

  return nodes;
}

/***************** QgsGrassModuleGroupBoxItem *********************/

QgsGrassModuleGroupBoxItem::QgsGrassModuleGroupBoxItem( QgsGrassModule *module, QString key, QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode, bool direct, QWidget *parent )
  : QGroupBox( parent )
  , QgsGrassModuleParam( module, key, qdesc, gdesc, gnode, direct )
{
  adjustTitle();
  setToolTip( mToolTip );
}

void QgsGrassModuleGroupBoxItem::resizeEvent( QResizeEvent *event )
{
  Q_UNUSED( event )
  adjustTitle();
  setToolTip( mToolTip );
}

void QgsGrassModuleGroupBoxItem::adjustTitle()
{
  QString tit = fontMetrics().elidedText( mTitle, Qt::ElideRight, width() - 20 );

  setTitle( tit );
}

/***************** QgsGrassModuleMultiParam *********************/

QgsGrassModuleMultiParam::QgsGrassModuleMultiParam( QgsGrassModule *module, QString key, QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode, bool direct, QWidget *parent )
  : QgsGrassModuleGroupBoxItem( module, key, qdesc, gdesc, gnode, direct, parent )
{
  adjustTitle();
  setToolTip( mToolTip );

  // variable number of line edits
  // add/delete buttons for multiple options
  mLayout = new QHBoxLayout( this );
  mParamsLayout = new QVBoxLayout();

  mLayout->insertLayout( -1, mParamsLayout );
}

void QgsGrassModuleMultiParam::showAddRemoveButtons()
{
  mButtonsLayout = new QVBoxLayout();
  mLayout->insertLayout( -1, mButtonsLayout );

  // TODO: how to keep both buttons on the top?
  QPushButton *addButton = new QPushButton( u"+"_s, this );
  connect( addButton, &QAbstractButton::clicked, this, &QgsGrassModuleMultiParam::addRow );
  mButtonsLayout->addWidget( addButton, 0, Qt::AlignTop );

  QPushButton *removeButton = new QPushButton( u"-"_s, this );
  connect( removeButton, &QAbstractButton::clicked, this, &QgsGrassModuleMultiParam::removeRow );
  mButtonsLayout->addWidget( removeButton, 0, Qt::AlignTop );

  // Don't enable this, it makes the group box expanding
  // mButtonsLayout->addStretch();
}

/********************** QgsGrassModuleOption *************************/

QgsGrassModuleOption::QgsGrassModuleOption( QgsGrassModule *module, QString key, QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode, bool direct, QWidget *parent )
  : QgsGrassModuleMultiParam( module, key, qdesc, gdesc, gnode, direct, parent )
  , mMin( std::numeric_limits<int>::max() )
  , mMax( std::numeric_limits<int>::min() )
{
  setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum );

  if ( mHidden )
  {
    hide();
  }

  // Is it output?
  QDomNode promptNode = gnode.namedItem( u"gisprompt"_s );
  if ( !promptNode.isNull() )
  {
    QDomElement promptElem = promptNode.toElement();
    QString element = promptElem.attribute( u"element"_s );
    QString age = promptElem.attribute( u"age"_s );

    if ( age == "new"_L1 )
    {
      mOutputElement = element;
      mIsOutput = true;

      if ( element == "vector"_L1 )
      {
        mOutputType = Vector;
      }
      else if ( element == "cell"_L1 )
      {
        mOutputType = Raster;
      }
    }
  }

  // String without options
  if ( !mHidden )
  {
    QDomElement gelem = gnode.toElement();

    // Output option may have missing gisprompt if output may be both vector and raster according to other options (e.g. v.kernel)
    // outputType qgm attribute allows forcing an output type

    // Predefined values ?
    QDomNode valuesNode = gnode.namedItem( u"values"_s );
    QDomElement valuesElem = valuesNode.toElement(); // null if valuesNode is null

    if ( !valuesNode.isNull() && valuesNode.childNodes().count() > 1 )
    {
      // predefined values -> ComboBox or CheckBox

      // TODO: add add/removeRow support for ComboBox?

      // one or many?
      if ( gelem.attribute( u"multiple"_s ) == "yes"_L1 )
      {
        mControlType = CheckBoxes;
      }
      else
      {
        mControlType = ComboBox;
        mComboBox = new QComboBox( this );
        paramsLayout()->addWidget( mComboBox );
      }

      // List of values to be excluded
      QStringList exclude = qdesc.attribute( u"exclude"_s ).split( ',', Qt::SkipEmptyParts );

      QDomNode valueNode = valuesElem.firstChild();

      while ( !valueNode.isNull() )
      {
        QDomElement valueElem = valueNode.toElement();

        if ( !valueElem.isNull() && valueElem.tagName() == "value"_L1 )
        {
          QDomNode n = valueNode.namedItem( u"name"_s );
          if ( !n.isNull() )
          {
            QDomElement e = n.toElement();
            QString val = e.text().trimmed();

            if ( exclude.contains( val ) == 0 )
            {
              n = valueNode.namedItem( u"description"_s );
              QString desc;
              if ( !n.isNull() )
              {
                e = n.toElement();
                desc = e.text().trimmed();
              }
              else
              {
                desc = val;
              }
              desc.replace( 0, 1, desc.at( 0 ).toUpper() );

              if ( mControlType == ComboBox )
              {
                mComboBox->addItem( desc );
                if ( mAnswer.length() > 0 && val == mAnswer )
                {
                  mComboBox->setCurrentIndex( mComboBox->count() - 1 );
                }
              }
              else
              {
                QgsGrassModuleCheckBox *cb = new QgsGrassModuleCheckBox( desc, this );
                mCheckBoxes.push_back( cb );
                paramsLayout()->addWidget( cb );
              }

              mValues.push_back( val );
            }
          }
        }

        valueNode = valueNode.nextSibling();
      }
    }
    else // No values
    {
      // Line edit
      mControlType = LineEdit;

      // Output option may have missing gisprompt if output may be both vector and raster according to other options (e.g. v.kernel)
      // outputType qgm attribute allow forcing an output type
      QgsDebugMsgLevel( "outputType = " + qdesc.attribute( "outputType" ), 3 );
      if ( qdesc.hasAttribute( u"outputType"_s ) )
      {
        QString outputType = qdesc.attribute( u"outputType"_s );
        mIsOutput = true;
        if ( outputType == "vector"_L1 )
        {
          mOutputElement = u"vector"_s;
          mOutputType = Vector;
        }
        else if ( outputType == "raster"_L1 )
        {
          mOutputElement = u"cell"_s;
          mOutputType = Raster;
        }
        else
        {
          mErrors << tr( "Unknown outputType" ) + " : " + outputType;
        }
      }

      if ( gelem.attribute( u"type"_s ) == "integer"_L1 )
      {
        mValueType = Integer;
      }
      else if ( gelem.attribute( u"type"_s ) == "float"_L1 )
      {
        mValueType = Double;
      }

      QStringList minMax;
      if ( valuesNode.childNodes().count() == 1 )
      {
        QDomNode valueNode = valuesElem.firstChild();

        QDomNode n = valueNode.namedItem( u"name"_s );
        if ( !n.isNull() )
        {
          QDomElement e = n.toElement();
          QString val = e.text().trimmed();
          minMax = val.split( '-' );
          if ( minMax.size() == 2 )
          {
            mHaveLimits = true;
            mMin = minMax.at( 0 ).toDouble();
            mMax = minMax.at( 1 ).toDouble();
          }
        }
      }

      QDomNode keydescNode = gnode.namedItem( u"keydesc"_s );
      if ( !keydescNode.isNull() )
      {
        // fixed number of line edits
        // Example:
        // <keydesc>
        //    <item order="1">rows</item>
        //    <item order="2">columns</item>
        // </keydesc>

        QDomNodeList keydescs = keydescNode.childNodes();
        for ( int k = 0; k < keydescs.count(); k++ )
        {
          QDomNode nodeItem = keydescs.at( k );
          QString itemDesc = nodeItem.toElement().text().trimmed();
          //QString itemDesc = nodeItem.firstChild().toText().data();
          QgsDebugMsgLevel( "keydesc item = " + itemDesc, 3 );

          addRow();
        }
      }
      else
      {
        addRow();
        if ( gelem.attribute( u"multiple"_s ) == "yes"_L1 )
        {
          showAddRemoveButtons();
        }
      }
    }
  }

  mUsesRegion = false;
  QString region = qdesc.attribute( u"region"_s );
  if ( region.length() > 0 )
  {
    if ( region == "yes"_L1 )
      mUsesRegion = true;
  }
  else
  {
    QgsDebugMsgLevel( "\n\n\n\n**************************", 3 );
    QgsDebugMsgLevel( QString( "isOutput = %1" ).arg( isOutput() ), 3 );
    QgsDebugMsgLevel( QString( "mOutputType = %1" ).arg( mOutputType ), 3 );
    if ( isOutput() && mOutputType == Raster )
      mUsesRegion = true;
  }
  QgsDebugMsgLevel( QString( "mUsesRegion = %1" ).arg( mUsesRegion ), 3 );
}

void QgsGrassModuleOption::addRow()
{
  // TODO make the widget growing with new lines. HOW???!!!
  QLineEdit *lineEdit = new QLineEdit( this );
  mLineEdits << lineEdit;
  lineEdit->setText( mAnswer );

  if ( mValueType == Integer )
  {
    if ( mHaveLimits )
    {
      mValidator = new QIntValidator( static_cast<int>( mMin ), static_cast<int>( mMax ), this );
    }
    else
    {
      mValidator = new QIntValidator( this );
    }
    lineEdit->setValidator( mValidator );
  }
  else if ( mValueType == Double )
  {
    if ( mHaveLimits )
    {
      mValidator = new QDoubleValidator( mMin, mMax, 10, this );
    }
    else
    {
      mValidator = new QDoubleValidator( this );
    }
    lineEdit->setValidator( mValidator );
  }
  else if ( mIsOutput )
  {
    QRegularExpression rx;
    if ( mOutputType == Vector )
    {
      rx.setPattern( u"[A-Za-z_][A-Za-z0-9_]+"_s );
    }
    else
    {
      rx.setPattern( u"[A-Za-z0-9_.]+"_s );
    }
    mValidator = new QRegularExpressionValidator( rx, this );

    lineEdit->setValidator( mValidator );
  }

  if ( mIsOutput && mDirect )
  {
    QHBoxLayout *l = new QHBoxLayout();
    l->addWidget( lineEdit );
    lineEdit->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
    QPushButton *button = new QPushButton( tr( "Browse" ) );
    l->addWidget( button );
    paramsLayout()->addItem( l );
    connect( button, &QAbstractButton::clicked, this, &QgsGrassModuleOption::browse );
  }
  else
  {
    paramsLayout()->addWidget( lineEdit );
  }
}

void QgsGrassModuleOption::removeRow()
{
  if ( mLineEdits.size() < 2 )
  {
    return;
  }
  delete mLineEdits.at( mLineEdits.size() - 1 );
  mLineEdits.removeLast();
}

void QgsGrassModuleOption::browse( bool checked )
{
  Q_UNUSED( checked )

  QgsSettings settings;
  QString lastDir = settings.value( u"GRASS/lastDirectOutputDir"_s, QString() ).toString();
  QString fileName = QFileDialog::getSaveFileName( this, tr( "Output file" ), lastDir, tr( "GeoTIFF" ) + " (*.tif)" );
  if ( !fileName.isEmpty() )
  {
    if ( !fileName.endsWith( ".tif"_L1, Qt::CaseInsensitive ) && !fileName.endsWith( ".tiff"_L1, Qt::CaseInsensitive ) )
    {
      fileName = fileName + ".tif";
    }
    mLineEdits.at( 0 )->setText( fileName );
    settings.setValue( u"GRASS/lastDirectOutputDir"_s, QFileInfo( fileName ).absolutePath() );
  }
}

QString QgsGrassModuleOption::outputExists()
{
  if ( !mIsOutput )
    return QString();

  QLineEdit *lineEdit = mLineEdits.at( 0 );
  QString value = lineEdit->text().trimmed();
  QgsDebugMsgLevel( "mKey = " + mKey, 3 );
  QgsDebugMsgLevel( "value = " + value, 3 );
  QgsDebugMsgLevel( "mOutputElement = " + mOutputElement, 3 );

  if ( value.length() == 0 )
    return QString();

  QString path = QgsGrass::getDefaultGisdbase() + "/"
                 + QgsGrass::getDefaultLocation() + "/"
                 + QgsGrass::getDefaultMapset() + "/"
                 + mOutputElement + "/" + value;

  QFileInfo fi( path );

  if ( fi.exists() )
  {
    return ( lineEdit->text() );
  }

  return QString();
}

QString QgsGrassModuleOption::value()
{
  QString value;

  if ( mHidden )
  {
    return mAnswer;
  }
  else if ( mControlType == LineEdit )
  {
    for ( int i = 0; i < mLineEdits.size(); i++ )
    {
      QLineEdit *lineEdit = mLineEdits.at( i );
      if ( lineEdit->text().trimmed().length() > 0 )
      {
        if ( value.length() > 0 )
          value.append( "," );
        value.append( lineEdit->text().trimmed() );
      }
    }
  }
  else if ( mControlType == ComboBox )
  {
    value = mValues[mComboBox->currentIndex()];
  }
  else if ( mControlType == CheckBoxes )
  {
    QStringList values;
    for ( int i = 0; i < mCheckBoxes.size(); ++i )
    {
      if ( mCheckBoxes[i]->isChecked() )
      {
        values.append( mValues[i] );
      }
    }
    value = values.join( ','_L1 );
  }
  return value;
}

bool QgsGrassModuleOption::checkVersion( const QString &version_min, const QString &version_max, QStringList &errors )
{
  QgsDebugMsgLevel( "version_min = " + version_min, 3 );
  QgsDebugMsgLevel( "version_max = " + version_max, 3 );

  bool minOk = true;
  bool maxOk = true;
  const thread_local QRegularExpression rxVersionMajor( "^(\\d+)$" );
  const thread_local QRegularExpression rxVersion( "^(\\d+)\\.(\\d+)$" );
  if ( !version_min.isEmpty() )
  {
    const QRegularExpressionMatch versionMatch = rxVersion.match( version_min );
    const QRegularExpressionMatch versionMajorMatch = rxVersionMajor.match( version_min );
    if ( versionMatch.hasMatch() )
    {
      int versionMajorMin = versionMatch.captured( 1 ).toInt();
      int versionMinorMin = versionMatch.captured( 2 ).toInt();
      if ( QgsGrass::versionMajor() < versionMajorMin || ( QgsGrass::versionMajor() == versionMajorMin && QgsGrass::versionMinor() < versionMinorMin ) )
      {
        minOk = false;
      }
    }
    else if ( versionMajorMatch.hasMatch() )
    {
      int versionMajorMin = versionMajorMatch.captured( 1 ).toInt();
      if ( QgsGrass::versionMajor() < versionMajorMin )
      {
        minOk = false;
      }
    }
    else
    {
      errors << tr( "Cannot parse version_min %1" ).arg( version_min );
    }
  }

  if ( !version_max.isEmpty() )
  {
    const QRegularExpressionMatch versionMatch = rxVersion.match( version_max );
    const QRegularExpressionMatch versionMajorMatch = rxVersionMajor.match( version_max );

    if ( versionMatch.hasMatch() )
    {
      int versionMajorMax = versionMatch.captured( 1 ).toInt();
      int versionMinorMax = versionMatch.captured( 2 ).toInt();
      if ( QgsGrass::versionMajor() > versionMajorMax || ( QgsGrass::versionMajor() == versionMajorMax && QgsGrass::versionMinor() > versionMinorMax ) )
      {
        maxOk = false;
      }
    }
    else if ( versionMajorMatch.hasMatch() )
    {
      int versionMajorMax = versionMajorMatch.captured( 1 ).toInt();
      if ( QgsGrass::versionMajor() > versionMajorMax )
      {
        maxOk = false;
      }
    }
    else
    {
      errors << tr( "Cannot parse version_max %1" ).arg( version_max );
    }
  }
  return errors.isEmpty() && minOk && maxOk;
}

QStringList QgsGrassModuleOption::options()
{
  QStringList list;

  QString val = value();
  if ( !val.isEmpty() )
  {
    list.push_back( mKey + "=" + val );
  }

  return list;
}

QString QgsGrassModuleOption::ready()
{
  QgsDebugMsgLevel( "key = " + key(), 3 );

  QString error;

  if ( value().isEmpty() && mRequired )
  {
    error.append( tr( "%1:&nbsp;missing value" ).arg( title() ) );
  }

  return error;
}

/***************** QgsGrassModuleFlag *********************/
QgsGrassModuleFlag::QgsGrassModuleFlag( QgsGrassModule *module, QString key, QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode, bool direct, QWidget *parent )
  : QgsGrassModuleCheckBox( QString(), parent ), QgsGrassModuleParam( module, key, qdesc, gdesc, gnode, direct )
{
  if ( mHidden )
    hide();

  if ( mAnswer == "on"_L1 )
    setChecked( true );
  else
    setChecked( false );

  setText( mTitle );
  setToolTip( mToolTip );
}

QStringList QgsGrassModuleFlag::options()
{
  QStringList list;
  if ( isChecked() )
  {
    list.push_back( "-" + mKey );
  }
  return list;
}

/***************** QgsGrassModuleGdalInput *********************/

QgsGrassModuleGdalInput::QgsGrassModuleGdalInput(
  QgsGrassModule *module, Type type, QString key, QDomElement &qdesc,
  QDomElement &gdesc, QDomNode &gnode, bool direct, QWidget *parent
)
  : QgsGrassModuleGroupBoxItem( module, key, qdesc, gdesc, gnode, direct, parent )
  , mType( type )
{
  if ( mTitle.isEmpty() )
  {
    mTitle = tr( "OGR/PostGIS/GDAL Input" );
  }
  adjustTitle();

  // Read "layeroption" is defined
  QString opt = qdesc.attribute( u"layeroption"_s );
  if ( !opt.isNull() )
  {
    QDomNode optNode = nodeByKey( gdesc, opt );

    if ( optNode.isNull() )
    {
      mErrors << tr( "Cannot find layeroption %1" ).arg( opt );
    }
    else
    {
      mOgrLayerOption = opt;
    }
  }

  // Read "whereoption" if defined
  opt = qdesc.attribute( u"whereoption"_s );
  if ( !opt.isNull() )
  {
    QDomNode optNode = nodeByKey( gdesc, opt );
    if ( optNode.isNull() )
    {
      mErrors << tr( "Cannot find whereoption %1" ).arg( opt );
    }
    else
    {
      mOgrWhereOption = opt;
    }
  }

  QVBoxLayout *l = new QVBoxLayout( this );
  mLayerComboBox = new QComboBox();
  mLayerComboBox->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
  l->addWidget( mLayerComboBox );

  QLabel *lbl = new QLabel( tr( "Password" ) );
  l->addWidget( lbl );

  mLayerPassword = new QLineEdit();
  mLayerPassword->setEchoMode( QLineEdit::Password );
  mLayerPassword->setEnabled( false );
  l->addWidget( mLayerPassword );

  lbl->setBuddy( mLayerPassword );

  connect( QgsProject::instance(), &QgsProject::layersAdded, this, &QgsGrassModuleGdalInput::updateQgisLayers );
  connect( QgsProject::instance(), &QgsProject::layersRemoved, this, &QgsGrassModuleGdalInput::updateQgisLayers );

  // Fill in QGIS layers
  updateQgisLayers();
}

void QgsGrassModuleGdalInput::updateQgisLayers()
{
  QString current = mLayerComboBox->currentText();
  mLayerComboBox->clear();
  mUri.clear();
  mOgrLayers.clear();

  // If not required, add an empty item to combobox and a padding item into
  // layer containers.
  if ( !mRequired )
  {
    mUri.push_back( QString() );
    mOgrLayers.push_back( QString() );
    mOgrWheres.push_back( QString() );
    mLayerComboBox->addItem( tr( "Select a layer" ), QVariant() );
  }

  for ( QgsMapLayer *layer : QgsProject::instance()->mapLayers().values() )
  {
    if ( !layer )
      continue;

    if ( mType == Ogr && layer->type() == Qgis::LayerType::Vector )
    {
      QgsVectorLayer *vector = qobject_cast<QgsVectorLayer *>( layer );
      if ( !vector || ( vector->providerType() != "ogr"_L1 && vector->providerType() != "postgres"_L1 ) )
        continue;

      QgsDataProvider *provider = vector->dataProvider();

      QString uri;
      QString ogrLayer;
      QString ogrWhere;
      if ( vector->providerType() == "postgres"_L1 )
      {
        // Construct OGR DSN
        QgsDataSourceUri dsUri( provider->dataSourceUri() );
        uri = "PG:" + dsUri.connectionInfo();

        // Starting with GDAL 1.7.0, it is possible to restrict the schemas
        // layer names are then listed without schema if only one schema is specified
        if ( !dsUri.schema().isEmpty() )
        {
          uri += " schemas=" + dsUri.schema();
        }

        ogrLayer += dsUri.table();
        ogrWhere = dsUri.sql();
      }
      else if ( vector->providerType() == "ogr"_L1 )
      {
        QStringList items = provider->dataSourceUri().split( '|' );

        if ( items.size() > 1 )
        {
          uri = items[0];

          ogrLayer.clear();
          ogrWhere.clear();

          for ( int i = 1; i < items.size(); i++ )
          {
            QStringList args = items[i].split( '=' );

            if ( args.size() != 2 )
              continue;

            if ( args[0] == "layername"_L1 && args[0] == "layerid"_L1 )
            {
              ogrLayer = args[1];
            }
            else if ( args[0] == "subset"_L1 )
            {
              ogrWhere = args[1];
            }
          }

          if ( uri.endsWith( ".shp"_L1, Qt::CaseInsensitive ) )
          {
            ogrLayer.clear();
          }
        }
        else
        {
          uri = items[0];
          ogrLayer.clear();
          ogrWhere.clear();
        }
      }

      QgsDebugMsgLevel( "uri = " + uri, 3 );
      QgsDebugMsgLevel( "ogrLayer = " + ogrLayer, 3 );

      mLayerComboBox->addItem( layer->name() );
      if ( layer->name() == current )
        mLayerComboBox->setItemText( mLayerComboBox->currentIndex(), current );

      mUri.push_back( uri );
      mOgrLayers.push_back( ogrLayer );
      mOgrWheres.push_back( ogrWhere );
    }
    else if ( mType == Gdal && layer->type() == Qgis::LayerType::Raster )
    {
      QString uri = layer->source();
      mLayerComboBox->addItem( layer->name() );
      if ( layer->name() == current )
        mLayerComboBox->setItemText( mLayerComboBox->currentIndex(), current );
      mUri.push_back( uri );
      mOgrLayers.push_back( QString() );
      mOgrWheres.push_back( QString() );
    }
  }
}

QStringList QgsGrassModuleGdalInput::options()
{
  QStringList list;

  int current = mLayerComboBox->currentIndex();
  if ( current < 0 )
    return list;

  QString opt( mKey + "=" );

  if ( current >= 0 && current < mUri.size() )
  {
    QString uri = mUri[current];

    if ( uri.startsWith( "PG:"_L1 ) && uri.contains( "password="_L1 ) && !mLayerPassword->text().isEmpty() )
    {
      uri += " password=" + mLayerPassword->text();
    }

    opt.append( uri );
  }

  list.push_back( opt );

  if ( !mOgrLayerOption.isEmpty() && mOgrLayers[current].size() > 0 )
  {
    opt = mOgrLayerOption + "=";
    opt += mOgrLayers[current];
    list.push_back( opt );
  }

  if ( !mOgrWhereOption.isEmpty() && mOgrWheres[current].length() > 0 )
  {
    list.push_back( mOgrWhereOption + "=" + mOgrWheres[current] );
  }

  return list;
}

QString QgsGrassModuleGdalInput::ready()
{
  QString error;

  QgsDebugMsgLevel( QString( "count = %1" ).arg( mLayerComboBox->count() ), 3 );
  if ( mLayerComboBox->count() == 0 )
  {
    error.append( tr( "%1:&nbsp;no input" ).arg( title() ) );
  }
  return error;
}

void QgsGrassModuleGdalInput::changed( int i )
{
  mLayerPassword->setEnabled( i < mUri.size() && mUri.value( i ).startsWith( "PG:"_L1 ) && !mUri.value( i ).contains( "password="_L1 ) );
}

/***************** QgsGrassModuleField *********************/
QgsGrassModuleField::QgsGrassModuleField( QgsGrassModule *module, QString key, QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode, bool direct, QWidget *parent )
  : QgsGrassModuleOption( module, key, qdesc, gdesc, gnode, direct, parent )
{
  // Validator is disabled to also allow entering of expressions
#if 0
  QRegExp rx( "^[a-zA-Z_][a-zA-Z0-9_]*$" );
  for ( QLineEdit *lineEdit : mLineEdits )
  {
    lineEdit->setValidator( new QRegExpValidator( rx, this ) );
  }
#endif
}

/***************** QgsGrassModuleVectorField *********************/

QgsGrassModuleVectorField::QgsGrassModuleVectorField(
  QgsGrassModule *module, QgsGrassModuleStandardOptions *options,
  QString key, QDomElement &qdesc,
  QDomElement &gdesc, QDomNode &gnode, bool direct, QWidget *parent
)
  : QgsGrassModuleMultiParam( module, key, qdesc, gdesc, gnode, direct, parent )
  , mModuleStandardOptions( options )
{
  if ( mTitle.isEmpty() )
  {
    mTitle = tr( "Attribute field" );
  }
  adjustTitle();

  QDomNode promptNode = gnode.namedItem( u"gisprompt"_s );
  QDomElement gelem = gnode.toElement();

  mType = qdesc.attribute( u"type"_s );

  mLayerKey = qdesc.attribute( u"layer"_s );
  if ( mLayerKey.isNull() || mLayerKey.length() == 0 )
  {
    mErrors << tr( "'layer' attribute in field tag with key= %1 is missing." ).arg( mKey );
  }
  else
  {
    QgsGrassModuleParam *item = mModuleStandardOptions->itemByKey( mLayerKey );
    // TODO check type
    if ( item )
    {
      mLayerInput = dynamic_cast<QgsGrassModuleInput *>( item );
      connect( mLayerInput, &QgsGrassModuleInput::valueChanged, this, &QgsGrassModuleVectorField::updateFields );
    }
  }

  addRow();
  if ( gelem.attribute( u"multiple"_s ) == "yes"_L1 )
  {
    showAddRemoveButtons();
  }

  // Fill in layer current fields
  updateFields();
}

void QgsGrassModuleVectorField::addRow()
{
  QComboBox *comboBox = new QComboBox();
  comboBox->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
  paramsLayout()->addWidget( comboBox );
  mComboBoxList << comboBox;
  updateFields();
}

void QgsGrassModuleVectorField::removeRow()
{
  if ( mComboBoxList.size() < 2 )
  {
    return;
  }
  delete mComboBoxList.at( mComboBoxList.size() - 1 );
  mComboBoxList.removeLast();
}

void QgsGrassModuleVectorField::updateFields()
{
  for ( QComboBox *comboBox : mComboBoxList )
  {
    QString current = comboBox->currentText();
    comboBox->clear();

    if ( !mLayerInput )
    {
      continue;
    }

    int index = 0;
    for ( const QgsField &field : mLayerInput->currentFields() )
    {
      if ( mType.contains( field.typeName() ) )
      {
        comboBox->addItem( field.name() );
        QgsDebugMsgLevel( "current = " + current + " field = " + field.name(), 3 );
        if ( field.name() == current )
        {
          comboBox->setCurrentIndex( index );
        }
        index++;
      }
    }
  }
}

QStringList QgsGrassModuleVectorField::options()
{
  QStringList list;

  QStringList valueList;
  for ( QComboBox *comboBox : mComboBoxList )
  {
    if ( !comboBox->currentText().isEmpty() )
    {
      valueList << comboBox->currentText();
    }
  }

  if ( !valueList.isEmpty() )
  {
    QString opt = mKey + "=" + valueList.join( ','_L1 );
    list << opt;
  }

  return list;
}

/***************** QgsGrassModuleSelection *********************/

QgsGrassModuleSelection::QgsGrassModuleSelection(
  QgsGrassModule *module, QgsGrassModuleStandardOptions *options,
  QString key, QDomElement &qdesc,
  QDomElement &gdesc, QDomNode &gnode, bool direct, QWidget *parent
)
  : QgsGrassModuleGroupBoxItem( module, key, qdesc, gdesc, gnode, direct, parent )
  , mModuleStandardOptions( options )
{
  if ( mTitle.isEmpty() )
  {
    mTitle = tr( "Selected categories" );
  }
  adjustTitle();

  QDomNode promptNode = gnode.namedItem( u"gisprompt"_s );
  QDomElement promptElem = promptNode.toElement();

  mLayerId = qdesc.attribute( u"layerid"_s );

  mType = qdesc.attribute( u"type"_s );

  QgsGrassModuleParam *item = mModuleStandardOptions->item( mLayerId );
  // TODO check type
  if ( item )
  {
    mLayerInput = dynamic_cast<QgsGrassModuleInput *>( item );
    connect( mLayerInput, &QgsGrassModuleInput::valueChanged, this, &QgsGrassModuleSelection::onLayerChanged );
  }

  QHBoxLayout *l = new QHBoxLayout( this );
  mLineEdit = new QLineEdit( this );
  l->addWidget( mLineEdit );

  mModeComboBox = new QComboBox( this );
  mModeComboBox->setSizeAdjustPolicy( QComboBox::AdjustToContents );
  mModeComboBox->addItem( tr( "Manual entry" ), Manual );
  connect( mModeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsGrassModuleSelection::onModeChanged );
  l->addWidget( mModeComboBox );

  connect( QgsProject::instance(), &QgsProject::layersAdded, this, &QgsGrassModuleSelection::onLayerChanged );
  connect( QgsProject::instance(), &QgsProject::layersRemoved, this, &QgsGrassModuleSelection::onLayerChanged );

  // Fill in layer current fields
  onLayerChanged();
}

void QgsGrassModuleSelection::onLayerChanged()
{
  if ( !mLayerInput )
  {
    return;
  }

  QStringList layerIds;
  // add new layers matching selected input layer if not yet present
  for ( QgsMapLayer *layer : QgsProject::instance()->mapLayers().values() )
  {
    QgsVectorLayer *vectorLayer = qobject_cast<QgsVectorLayer *>( layer );
    if ( vectorLayer && vectorLayer->providerType() == "grass"_L1 )
    {
      QString uri = vectorLayer->dataProvider()->dataSourceUri();
      QgsDebugMsgLevel( "uri = " + uri, 3 );
      QString layerCode = uri.split( '/' ).last();
      if ( mLayerInput->currentLayerCodes().contains( layerCode ) )
      {
        // Qt::UserRole+1 may be also uri (AddLayer) but hardly matching layer id
        if ( mModeComboBox->findData( vectorLayer->id(), Qt::UserRole + 1 ) == -1 )
        {
          mModeComboBox->addItem( vectorLayer->name() + " " + tr( "layer selection" ), Layer );
          mModeComboBox->setItemData( mModeComboBox->count() - 1, vectorLayer->id(), Qt::UserRole + 1 );
        }
        layerIds << vectorLayer->id();
      }
    }
  }
  // remove layers no more present
  for ( int i = mModeComboBox->count() - 1; i >= 0; i-- )
  {
    if ( mModeComboBox->itemData( i ).toInt() != Layer )
    {
      continue;
    }
    QString id = mModeComboBox->itemData( i, Qt::UserRole + 1 ).toString();
    if ( !layerIds.contains( id ) )
    {
      mModeComboBox->removeItem( i );
    }
  }

  // clear old AddLayer
  for ( int i = mModeComboBox->count() - 1; i >= 0; i-- )
  {
    if ( mModeComboBox->itemData( i ).toInt() == AddLayer )
    {
      mModeComboBox->removeItem( i );
    }
  }

  if ( layerIds.size() == 0 ) // non of selected layer is in canvas
  {
    for ( QString layerCode : mLayerInput->currentLayerCodes() )
    {
      if ( mLayerInput->currentLayer() )
      {
        mModeComboBox->addItem( tr( "Add to canvas layer" ) + " " + mLayerInput->currentMap() + " " + layerCode, AddLayer );
        QgsGrassObject grassObject = mLayerInput->currentLayer()->grassObject();
        QString uri = grassObject.mapsetPath() + "/" + grassObject.name() + "/" + layerCode;
        QgsDebugMsgLevel( "uri = " + uri, 3 );
        // Qt::UserRole+1 may be also layer id (Layer) but hardly matching layer uri
        if ( mModeComboBox->findData( uri, Qt::UserRole + 1 ) == -1 )
        {
          mModeComboBox->setItemData( mModeComboBox->count() - 1, uri, Qt::UserRole + 1 );
          QString name = grassObject.name() + " " + layerCode;
          mModeComboBox->setItemData( mModeComboBox->count() - 1, name, Qt::UserRole + 2 );
        }
      }
    }
  }
}

QString QgsGrassModuleSelection::currentSelectionLayerId()
{
  QString id;
  int index = mModeComboBox->currentIndex();
  if ( mModeComboBox->itemData( index ).toInt() == Layer )
  {
    id = mModeComboBox->itemData( index, Qt::UserRole + 1 ).toString();
  }
  return id;
}

QgsVectorLayer *QgsGrassModuleSelection::currentSelectionLayer()
{
  QString id = currentSelectionLayerId();
  if ( id.isEmpty() )
  {
    return nullptr;
  }
  QgsMapLayer *layer = QgsProject::instance()->mapLayer( id );
  return qobject_cast<QgsVectorLayer *>( layer );
}

void QgsGrassModuleSelection::onModeChanged()
{
  int index = mModeComboBox->currentIndex();
  if ( mModeComboBox->itemData( index ).toInt() == AddLayer )
  {
    QString uri = mModeComboBox->itemData( index, Qt::UserRole + 1 ).toString();
    QString name = mModeComboBox->itemData( index, Qt::UserRole + 2 ).toString();
    QgsDebugMsgLevel( "uri = " + uri, 3 );

    QgsVectorLayer *layer = new QgsVectorLayer( uri, name, u"grass"_s );
    QgsProject::instance()->addMapLayer( layer );
    onLayerChanged(); // update with added layer
  }
  else if ( mModeComboBox->itemData( index ).toInt() == Layer )
  {
    QString id = mModeComboBox->itemData( index, Qt::UserRole + 1 ).toString();
    QgsMapLayer *layer = QgsProject::instance()->mapLayer( id );
    QgsVectorLayer *vectorLayer = qobject_cast<QgsVectorLayer *>( layer );
    if ( vectorLayer )
    {
      onLayerSelectionChanged();
      connect( vectorLayer, &QgsVectorLayer::selectionChanged, this, &QgsGrassModuleSelection::onLayerSelectionChanged );
    }
  }
}

void QgsGrassModuleSelection::onLayerSelectionChanged()
{
  mLineEdit->clear();

  QgsVectorLayer *vectorLayer = currentSelectionLayer();
  if ( !vectorLayer )
  {
    return;
  }

  QList<int> cats;
  for ( QgsFeatureId fid : vectorLayer->selectedFeatureIds() )
  {
    cats << QgsGrassFeatureIterator::catFromFid( fid );
  }
  std::sort( cats.begin(), cats.end() );
  QString list;
  // make ranges of cats
  int last = -1;
  int range = false;
  for ( int cat : cats )
  {
    if ( cat == 0 )
    {
      continue;
    }
    if ( last == cat - 1 ) // begin or continue range
    {
      range = true;
    }
    else if ( range ) // close range and next  cat
    {
      list += u"-%1,%2"_s.arg( last ).arg( cat );
      range = false;
    }
    else // next cat
    {
      if ( !list.isEmpty() )
      {
        list += ','_L1;
      }
      list += QString::number( cat );
    }
    last = cat;
  }
  if ( range )
  {
    list += u"-%1"_s.arg( last );
  }

  mLineEdit->setText( list );
}

QStringList QgsGrassModuleSelection::options()
{
  QStringList list;

  if ( !mLineEdit->text().isEmpty() )
  {
    QString opt( mKey + "=" + mLineEdit->text() );
    list.push_back( opt );
  }

  return list;
}

/***************** QgsGrassModuleFile *********************/

QgsGrassModuleFile::QgsGrassModuleFile(
  QgsGrassModule *module,
  QString key, QDomElement &qdesc,
  QDomElement &gdesc, QDomNode &gnode, bool direct, QWidget *parent
)
  : QgsGrassModuleGroupBoxItem( module, key, qdesc, gdesc, gnode, direct, parent )
{
  if ( mTitle.isEmpty() )
  {
    mTitle = tr( "File" );
  }
  adjustTitle();

  if ( qdesc.attribute( u"type"_s ).toLower() == "new"_L1 )
  {
    mType = New;
  }
  if ( qdesc.attribute( u"type"_s ).toLower() == "multiple"_L1 )
  {
    mType = Multiple;
  }

  if ( qdesc.attribute( u"type"_s ).toLower() == "directory"_L1 )
  {
    mType = Directory;
  }

  mFilters = qdesc.attribute( u"filters"_s );

  mFileOption = qdesc.attribute( u"fileoption"_s );

  QHBoxLayout *l = new QHBoxLayout( this );
  mLineEdit = new QLineEdit();
  mBrowseButton = new QPushButton( u"…"_s );
  l->addWidget( mLineEdit );
  l->addWidget( mBrowseButton );

  connect( mBrowseButton, &QAbstractButton::clicked, this, &QgsGrassModuleFile::browse );
}

QStringList QgsGrassModuleFile::options()
{
  QStringList list;
  QString path = mLineEdit->text().trimmed();

  if ( mFileOption.isNull() )
  {
    QString opt( mKey + "=" + path );
    list.push_back( opt );
  }
  else
  {
    QFileInfo fi( path );

    QString opt( mKey + "=" + fi.path() );
    list.push_back( opt );

    opt = mFileOption + "=" + fi.baseName();
    list.push_back( opt );
  }

  return list;
}

void QgsGrassModuleFile::browse()
{
  static QString lastDir = QDir::currentPath();

  if ( mType == Multiple )
  {
    QString path = mLineEdit->text().split( ',' ).first();
    if ( path.isEmpty() )
      path = lastDir;
    else
      path = QFileInfo( path ).absolutePath();

    QStringList files = QFileDialog::getOpenFileNames( this, nullptr, path, mFilters );
    if ( files.isEmpty() )
      return;

    lastDir = QFileInfo( files[0] ).absolutePath();

    mLineEdit->setText( files.join( ','_L1 ) );
  }
  else
  {
    QString selectedFile = mLineEdit->text();
    if ( selectedFile.isEmpty() )
      selectedFile = lastDir;

    if ( mType == New )
      selectedFile = QFileDialog::getSaveFileName( this, nullptr, selectedFile, mFilters );
    else if ( mType == Directory )
      selectedFile = QFileDialog::getExistingDirectory( this, nullptr, selectedFile );
    else
      selectedFile = QFileDialog::getOpenFileName( this, nullptr, selectedFile, mFilters );

    lastDir = QFileInfo( selectedFile ).absolutePath();

    mLineEdit->setText( selectedFile );
  }
}

QString QgsGrassModuleFile::ready()
{
  QgsDebugMsgLevel( "key = " + key(), 3 );

  QString error;
  QString path = mLineEdit->text().trimmed();


  if ( path.length() == 0 && mRequired )
  {
    error.append( tr( "%1:&nbsp;missing value" ).arg( title() ) );
    return error;
  }

  QFileInfo fi( path );
  if ( !fi.dir().exists() )
  {
    error.append( tr( "%1:&nbsp;directory does not exist" ).arg( title() ) );
  }

  return error;
}

/***************************** QgsGrassModuleCheckBox *********************************/

QgsGrassModuleCheckBox::QgsGrassModuleCheckBox( const QString &text, QWidget *parent )
  : QCheckBox( text, parent )
  , mText( text )
{
  adjustText();
}

void QgsGrassModuleCheckBox::resizeEvent( QResizeEvent *event )
{
  Q_UNUSED( event )
  adjustText();
}
void QgsGrassModuleCheckBox::setText( const QString &text )
{
  mText = text;
  adjustText();
}
void QgsGrassModuleCheckBox::setToolTip( const QString &text )
{
  mTip = text;
  QWidget::setToolTip( text );
}
void QgsGrassModuleCheckBox::adjustText()
{
  QString t = fontMetrics().elidedText( mText, Qt::ElideRight, width() - iconSize().width() - 20 );
  QCheckBox::setText( t );

  if ( mTip.isEmpty() )
  {
    QString tt;
    if ( t != mText )
    {
      tt = mText;
    }
    QWidget::setToolTip( tt );
  }
}
