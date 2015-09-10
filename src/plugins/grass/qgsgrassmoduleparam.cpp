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

#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>

#include "qgis.h"
#include "qgsdatasourceuri.h"
#include "qgslogger.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerregistry.h"
#include "qgsrasterlayer.h"
#include "qgsvectorlayer.h"

#include "qgsgrass.h"
#include "qgsgrassmodule.h"
#include "qgsgrassmoduleinput.h"
#include "qgsgrassmoduleparam.h"
#include "qgsgrassplugin.h"
#include "qgsgrassprovider.h"

#if 0
extern "C"
{
#if GRASS_VERSION_MAJOR < 7
#include <grass/Vect.h>
#else
#include <grass/vector.h>
#endif
}
#endif

/********************** QgsGrassModuleParam *************************/
QgsGrassModuleParam::QgsGrassModuleParam( QgsGrassModule *module, QString key,
    QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode, bool direct )
    : mModule( module )
    , mKey( key )
    , mMultiple( false )
    , mHidden( false )
    , mRequired( false )
    , mDirect( direct )
{
  Q_UNUSED( gdesc );
  //mAnswer = qdesc.attribute("answer", "");

  if ( !qdesc.attribute( "answer" ).isNull() )
  {
    mAnswer = qdesc.attribute( "answer" ).trimmed();
  }
  else
  {
    QDomNode n = gnode.namedItem( "default" );
    if ( !n.isNull() )
    {
      QDomElement e = n.toElement();
      mAnswer = e.text().trimmed();
    }
  }

  if ( qdesc.attribute( "hidden" ) == "yes" )
  {
    mHidden = true;
  }

  QString label, description;
  if ( !qdesc.attribute( "label" ).isEmpty() )
  {
    label = QApplication::translate( "grasslabel", qdesc.attribute( "label" ).trimmed().toUtf8() );
  }
  if ( label.isEmpty() )
  {
    QDomNode n = gnode.namedItem( "label" );
    if ( !n.isNull() )
    {
      QDomElement e = n.toElement();
      label = module->translate( e.text() );
    }
  }
  QDomNode n = gnode.namedItem( "description" );
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

  mRequired = gnode.toElement().attribute( "required" ) == "yes";

  mMultiple = gnode.toElement().attribute( "multiple" ) == "yes";

  mId = qdesc.attribute( "id" );
}

QgsGrassModuleParam::~QgsGrassModuleParam() {}

bool QgsGrassModuleParam::hidden()
{
  return mHidden;
}

QStringList QgsGrassModuleParam::options()
{
  return QStringList();
}

QString QgsGrassModuleParam::getDescPrompt( QDomElement descDomElement, const QString & name )
{
  QDomNode gispromptNode = descDomElement.namedItem( "gisprompt" );

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
  QgsDebugMsg( "called with key=" + key );
  QDomNode n = descDomElement.firstChild();

  while ( !n.isNull() )
  {
    QDomElement e = n.toElement();

    if ( !e.isNull() )
    {
      if ( e.tagName() == "parameter" || e.tagName() == "flag" )
      {
        if ( e.attribute( "name" ) == key )
        {
          return n;
        }
      }
    }
    n = n.nextSibling();
  }

  return QDomNode();
}

QList<QDomNode> QgsGrassModuleParam::nodesByType( QDomElement descDomElement, STD_OPT optionType, const QString & age )
{
  // TODO: never tested
  QList<QDomNode> nodes;

  // Not all options have prompt set, for example G_OPT_V_TYPE and G_OPT_V_FIELD, which would be useful, don't have prompt
  QMap<QString, STD_OPT> typeMap;
#if GRASS_VERSION_MAJOR < 7
  typeMap.insert( "dbtable", G_OPT_TABLE );
  typeMap.insert( "dbdriver", G_OPT_DRIVER );
  typeMap.insert( "dbname", G_OPT_DATABASE );
  typeMap.insert( "dbcolumn", G_OPT_COLUMN );
#else
  typeMap.insert( "dbtable", G_OPT_DB_TABLE );
  typeMap.insert( "dbdriver", G_OPT_DB_DRIVER );
  typeMap.insert( "dbname", G_OPT_DB_DATABASE );
  typeMap.insert( "dbcolumn", G_OPT_DB_COLUMN );
#endif
  typeMap.insert( "vector", G_OPT_V_INPUT );

  QDomNode n = descDomElement.firstChild();

  while ( !n.isNull() )
  {
    QString prompt = getDescPrompt( n.toElement(), "prompt" );
    if ( typeMap.value( prompt ) == optionType )
    {
      if ( age.isEmpty() || getDescPrompt( n.toElement(), "age" ) == age )
      {
        nodes << n;
      }
    }

    n = n.nextSibling();
  }

  return nodes;
}

/********************** QgsGrassModuleOption *************************/

QgsGrassModuleOption::QgsGrassModuleOption( QgsGrassModule *module, QString key,
    QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode,
    bool direct, QWidget * parent )
    : QgsGrassModuleGroupBoxItem( module, key, qdesc, gdesc, gnode, direct, parent )
    , mControlType( NoControl )
    , mValueType( String )
    , mOutputType( None )
    , mHaveLimits( false )
    , mMin( INT_MAX )
    , mMax( INT_MIN )
    , mComboBox( 0 )
    , mIsOutput( false )
    , mValidator( 0 )
    , mLayout( 0 )
    , mUsesRegion( false )
{
  QgsDebugMsg( "entered" );
  setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum );

  if ( mHidden )
    hide();

  mLayout = new QVBoxLayout();

  // Is it output?
  QDomNode promptNode = gnode.namedItem( "gisprompt" );
  if ( !promptNode.isNull() )
  {
    QDomElement promptElem = promptNode.toElement();
    QString element = promptElem.attribute( "element" );
    QString age = promptElem.attribute( "age" );

    if ( age == "new" )
    {
      mOutputElement = element;
      mIsOutput = true;

      if ( element == "vector" )
      {
        mOutputType = Vector;
      }
      else if ( element == "cell" )
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
    // outputType qgm attribute allows to force output type

    // Predefined values ?
    QDomNode valuesNode = gnode.namedItem( "values" );
    QDomElement valuesElem = valuesNode.toElement(); // null if valuesNode is null

    if ( !valuesNode.isNull() && valuesNode.childNodes().count() > 1 )
    {
      setLayout( mLayout );
      // predefined values -> ComboBox or CheckBox

      // one or many?
      if ( gelem.attribute( "multiple" ) == "yes" )
      {
        mControlType = CheckBoxes;
      }
      else
      {
        mControlType = ComboBox;
        mComboBox = new QComboBox( this );
        mLayout->addWidget( mComboBox );
      }

      // List of values to be excluded
      QStringList exclude = qdesc.attribute( "exclude" ).split( ',', QString::SkipEmptyParts );

      QDomNode valueNode = valuesElem.firstChild();

      while ( !valueNode.isNull() )
      {
        QDomElement valueElem = valueNode.toElement();

        if ( !valueElem.isNull() && valueElem.tagName() == "value" )
        {

          QDomNode n = valueNode.namedItem( "name" );
          if ( !n.isNull() )
          {
            QDomElement e = n.toElement();
            QString val = e.text().trimmed();

            if ( exclude.contains( val ) == 0 )
            {
              n = valueNode.namedItem( "description" );
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
              desc.replace( 0, 1, desc.left( 1 ).toUpper() );

              if ( mControlType == ComboBox )
              {
                mComboBox->addItem( desc );
                if ( mAnswer.length() > 0 && desc == mAnswer )
                {
                  mComboBox->setCurrentIndex( mComboBox->count() - 1 );
                }
              }
              else
              {
                QgsGrassModuleCheckBox *cb = new QgsGrassModuleCheckBox( desc, this );
                mCheckBoxes.push_back( cb );
                mLayout->addWidget( cb );
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
      // outputType qgm attribute allows to force output type
      QgsDebugMsg( "outputType = " + qdesc.attribute( "outputType" ) );
      if ( qdesc.hasAttribute( "outputType" ) )
      {
        QString outputType = qdesc.attribute( "outputType" );
        mIsOutput = true;
        if ( outputType == "vector" )
        {
          mOutputElement = "vector";
          mOutputType = Vector;
        }
        else if ( outputType == "raster" )
        {
          mOutputElement = "cell";
          mOutputType = Raster;
        }
        else
        {
          mErrors << tr( "Unknown outputType" ) + " : " + outputType;
        }
      }

      if ( gelem.attribute( "type" ) == "integer" )
      {
        mValueType = Integer;
      }
      else if ( gelem.attribute( "type" ) == "float" )
      {
        mValueType = Double;
      }

      QStringList minMax;
      if ( valuesNode.childNodes().count() == 1 )
      {
        QDomNode valueNode = valuesElem.firstChild();

        QDomNode n = valueNode.namedItem( "name" );
        if ( !n.isNull() )
        {
          QDomElement e = n.toElement();
          QString val = e.text().trimmed();
          minMax = val.split( "-" );
          if ( minMax.size() == 2 )
          {
            mHaveLimits = true;
            mMin = minMax.at( 0 ).toDouble();
            mMax = minMax.at( 1 ).toDouble();
          }
        }
      }

      QDomNode keydescNode = gnode.namedItem( "keydesc" );
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
          QgsDebugMsg( "keydesc item = " + itemDesc );

          addLineEdit();
        }

        setLayout( mLayout );
      }
      else if ( gelem.attribute( "multiple" ) == "yes" )
      {
        // variable number of line edits
        // add/delete buttons for multiple options
        QHBoxLayout *l = new QHBoxLayout( this );
        QVBoxLayout *vl = new QVBoxLayout();
        l->insertLayout( -1, mLayout );
        l->insertLayout( -1, vl );

        // TODO: how to keep both buttons on the top?
        QPushButton *b = new QPushButton( "+", this );
        connect( b, SIGNAL( clicked() ), this, SLOT( addLineEdit() ) );
        vl->addWidget( b, 0, Qt::AlignTop );

        b = new QPushButton( "-", this );
        connect( b, SIGNAL( clicked() ), this, SLOT( removeLineEdit() ) );
        vl->addWidget( b, 0, Qt::AlignTop );

        // Don't enable this, it makes the group box expanding
        // vl->addStretch();
      }
      else
      {
        // only one line edit
        addLineEdit();
        setLayout( mLayout );
      }
    }
  }

  mUsesRegion = false;
  QString region = qdesc.attribute( "region" );
  if ( region.length() > 0 )
  {
    if ( region == "yes" )
      mUsesRegion = true;
  }
  else
  {
    QgsDebugMsg( "\n\n\n\n**************************" );
    QgsDebugMsg( QString( "isOutput = %1" ).arg( isOutput() ) );
    QgsDebugMsg( QString( "mOutputType = %1" ).arg( mOutputType ) );
    if ( isOutput() && mOutputType == Raster )
      mUsesRegion = true;
  }
  QgsDebugMsg( QString( "mUsesRegion = %1" ).arg( mUsesRegion ) );
}

void QgsGrassModuleOption::addLineEdit()
{
  QgsDebugMsg( "entered" );

  // TODO make the widget growing with new lines. HOW???!!!
  QLineEdit *lineEdit = new QLineEdit( this );
  mLineEdits.push_back( lineEdit );
  lineEdit->setText( mAnswer );

  if ( mValueType == Integer )
  {
    if ( mHaveLimits )
    {
      mValidator = new QIntValidator(( int )mMin, ( int )mMax, this );
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
    QRegExp rx;
    if ( mOutputType == Vector )
    {
      rx.setPattern( "[A-Za-z_][A-Za-z0-9_]+" );
    }
    else
    {
      rx.setPattern( "[A-Za-z0-9_.]+" );
    }
    mValidator = new QRegExpValidator( rx, this );

    lineEdit->setValidator( mValidator );
  }

  if ( mIsOutput && mDirect )
  {
    QHBoxLayout *l = new QHBoxLayout();
    l->addWidget( lineEdit );
    lineEdit->setSizePolicy( QSizePolicy::Expanding, QSizePolicy:: Preferred );
    QPushButton *button = new QPushButton( tr( "Browse" ) );
    l->addWidget( button );
    mLayout->addItem( l );
    connect( button, SIGNAL( clicked( bool ) ), this, SLOT( browse( bool ) ) );
  }
  else
  {
    mLayout->addWidget( lineEdit );
  }
}

void QgsGrassModuleOption::browse( bool checked )
{
  Q_UNUSED( checked );
  QgsDebugMsg( "entered" );

  QSettings settings;
  QString lastDir = settings.value( "/GRASS/lastDirectOutputDir", "" ).toString();
  QString fileName = QFileDialog::getSaveFileName( this, tr( "Output file" ), lastDir, tr( "GeoTIFF" ) + " (*.tif)" );
  if ( !fileName.isEmpty() )
  {
    if ( !fileName.endsWith( ".tif", Qt::CaseInsensitive ) && !fileName.endsWith( ".tiff", Qt::CaseInsensitive ) )
    {
      fileName = fileName + ".tif";
    }
    mLineEdits.at( 0 )->setText( fileName );
    settings.setValue( "/GRASS/lastDirectOutputDir",  QFileInfo( fileName ).absolutePath() );
  }
}

void QgsGrassModuleOption::removeLineEdit()
{
  QgsDebugMsg( "entered" );

  if ( mLineEdits.size() < 2 )
    return;
  delete mLineEdits.at( mLineEdits.size() - 1 );
  mLineEdits.pop_back();
}

QString QgsGrassModuleOption::outputExists()
{
  QgsDebugMsg( "entered" );

  if ( !mIsOutput )
    return QString();

  QLineEdit *lineEdit = mLineEdits.at( 0 );
  QString value = lineEdit->text().trimmed();
  QgsDebugMsg( "mKey = " + mKey );
  QgsDebugMsg( "value = " + value );
  QgsDebugMsg( "mOutputElement = " + mOutputElement );

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
    value = values.join( "," );
  }
  return value;
}

bool QgsGrassModuleOption::checkVersion( const QString& version_min, const QString& version_max, QStringList& errors )
{
  QgsDebugMsg( "version_min = " + version_min );
  QgsDebugMsg( "version_max = " + version_max );

  bool minOk = true;
  bool maxOk = true;
  QRegExp rxVersionMajor( "(\\d+)" );
  QRegExp rxVersion( "(\\d+)\\.(\\d+)" );
  if ( !version_min.isEmpty() )
  {
    if ( rxVersion.exactMatch( version_min ) )
    {
      int versionMajorMin = rxVersion.cap( 1 ).toInt();
      int versionMinorMin = rxVersion.cap( 2 ).toInt();
      if ( QgsGrass::versionMajor() < versionMajorMin || ( QgsGrass::versionMajor() == versionMajorMin && QgsGrass::versionMinor() < versionMinorMin ) )
      {
        minOk = false;
      }
    }
    else if ( rxVersionMajor.exactMatch( version_min ) )
    {
      int versionMajorMin = rxVersionMajor.cap( 1 ).toInt();
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
    if ( rxVersion.exactMatch( version_max ) )
    {
      int versionMajorMax = rxVersion.cap( 1 ).toInt();
      int versionMinorMax = rxVersion.cap( 2 ).toInt();
      if ( QgsGrass::versionMajor() > versionMajorMax || ( QgsGrass::versionMajor() == versionMajorMax && QgsGrass::versionMinor() > versionMinorMax ) )
      {
        maxOk = false;
      }
    }
    else if ( rxVersionMajor.exactMatch( version_max ) )
    {
      int versionMajorMax = rxVersionMajor.cap( 1 ).toInt();
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
  QgsDebugMsg( "key = " + key() );

  QString error;

  if ( value().isEmpty() && mRequired )
  {
    error.append( tr( "%1:&nbsp;missing value" ).arg( title() ) );
  }

  return error;
}

QgsGrassModuleOption::~QgsGrassModuleOption()
{
}

/***************** QgsGrassModuleFlag *********************/
QgsGrassModuleFlag::QgsGrassModuleFlag( QgsGrassModule *module, QString key,
                                        QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode,
                                        bool direct, QWidget * parent )
    : QgsGrassModuleCheckBox( "", parent ), QgsGrassModuleParam( module, key, qdesc, gdesc, gnode, direct )
{
  QgsDebugMsg( "entered" );

  if ( mHidden )
    hide();

  if ( mAnswer == "on" )
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

QgsGrassModuleFlag::~QgsGrassModuleFlag()
{
}

/***************** QgsGrassModuleGroupBoxItem *********************/

QgsGrassModuleGroupBoxItem::QgsGrassModuleGroupBoxItem( QgsGrassModule *module, QString key,
    QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode,
    bool direct, QWidget * parent )
    : QGroupBox( parent )
    , QgsGrassModuleParam( module, key, qdesc, gdesc, gnode, direct )
{
  adjustTitle();

  setToolTip( mToolTip );
}

QgsGrassModuleGroupBoxItem::~QgsGrassModuleGroupBoxItem() {}

void QgsGrassModuleGroupBoxItem::resizeEvent( QResizeEvent * event )
{
  Q_UNUSED( event );
  adjustTitle();
  setToolTip( mToolTip );
}

void QgsGrassModuleGroupBoxItem::adjustTitle()
{
  QString tit = fontMetrics().elidedText( mTitle, Qt::ElideRight, width() - 20 );

  setTitle( tit );
}

/***************** QgsGrassModuleGdalInput *********************/

QgsGrassModuleGdalInput::QgsGrassModuleGdalInput(
  QgsGrassModule *module, Type type, QString key, QDomElement &qdesc,
  QDomElement &gdesc, QDomNode &gnode, bool direct, QWidget * parent )
    : QgsGrassModuleGroupBoxItem( module, key, qdesc, gdesc, gnode, direct, parent )
    , mType( type )
    , mOgrLayerOption( "" )
    , mOgrWhereOption( "" )
{
  if ( mTitle.isEmpty() )
  {
    mTitle = tr( "OGR/PostGIS/GDAL Input" );
  }
  adjustTitle();

  // Check if this parameter is required
  mRequired = gnode.toElement().attribute( "required" ) == "yes";

  QDomNode promptNode = gnode.namedItem( "gisprompt" );
  QDomElement promptElem = promptNode.toElement();
  QString element = promptElem.attribute( "element" );

  // Read "layeroption" is defined
  QString opt = qdesc.attribute( "layeroption" );
  if ( ! opt.isNull() )
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
  opt = qdesc.attribute( "whereoption" );
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
  mLayerComboBox->setSizePolicy( QSizePolicy::Expanding, QSizePolicy:: Preferred );
  l->addWidget( mLayerComboBox );

  QLabel *lbl = new QLabel( tr( "Password" ) );
  l->addWidget( lbl );

  mLayerPassword = new QLineEdit();
  mLayerPassword->setEchoMode( QLineEdit::Password );
  mLayerPassword->setEnabled( false );
  l->addWidget( mLayerPassword );

  lbl->setBuddy( mLayerPassword );

  connect( QgsMapLayerRegistry::instance(), SIGNAL( layersAdded( QList<QgsMapLayer *> ) ),
           this, SLOT( updateQgisLayers() ) );
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layersRemoved( QStringList ) ),
           this, SLOT( updateQgisLayers() ) );

  // Fill in QGIS layers
  updateQgisLayers();
}

void QgsGrassModuleGdalInput::updateQgisLayers()
{
  QgsDebugMsg( "entered" );

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

  foreach ( QgsMapLayer *layer, QgsMapLayerRegistry::instance()->mapLayers().values() )
  {
    if ( !layer ) continue;

    if ( mType == Ogr && layer->type() == QgsMapLayer::VectorLayer )
    {
      QgsVectorLayer *vector = qobject_cast<QgsVectorLayer *>( layer );
      if ( !vector ||
           ( vector->providerType() != "ogr" && vector->providerType() != "postgres" )
         )
        continue;

      QgsDataProvider *provider = vector->dataProvider();

      QString uri;
      QString ogrLayer;
      QString ogrWhere;
      if ( vector->providerType() == "postgres" )
      {
        // Construct OGR DSN
        QgsDataSourceURI dsUri( provider->dataSourceUri() );
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
      else if ( vector->providerType() == "ogr" )
      {
        QStringList items = provider->dataSourceUri().split( "|" );

        if ( items.size() > 1 )
        {
          uri = items[0];

          ogrLayer = "";
          ogrWhere = "";

          for ( int i = 1; i < items.size(); i++ )
          {
            QStringList args = items[i].split( "=" );

            if ( args.size() != 2 )
              continue;

            if ( args[0] == "layername" && args[0] == "layerid" )
            {
              ogrLayer = args[1];
            }
            else if ( args[0] == "subset" )
            {
              ogrWhere = args[1];
            }
          }

          if ( uri.endsWith( ".shp", Qt::CaseInsensitive ) )
          {
            ogrLayer = "";
          }
        }
        else
        {
          uri = items[0];
          ogrLayer = "";
          ogrWhere = "";
        }
      }

      QgsDebugMsg( "uri = " + uri );
      QgsDebugMsg( "ogrLayer = " + ogrLayer );

      mLayerComboBox->addItem( layer->name() );
      if ( layer->name() == current )
        mLayerComboBox->setItemText( mLayerComboBox->currentIndex(), current );

      mUri.push_back( uri );
      mOgrLayers.push_back( ogrLayer );
      mOgrWheres.push_back( ogrWhere );
    }
    else if ( mType == Gdal && layer->type() == QgsMapLayer::RasterLayer )
    {
      QString uri = layer->source();
      mLayerComboBox->addItem( layer->name() );
      if ( layer->name() == current )
        mLayerComboBox->setItemText( mLayerComboBox->currentIndex(), current );
      mUri.push_back( uri );
      mOgrLayers.push_back( "" );
      mOgrWheres.push_back( "" );
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

    if ( uri.startsWith( "PG:" ) && uri.contains( "password=" ) && !mLayerPassword->text().isEmpty() )
    {
      uri += " password=" + mLayerPassword->text();
    }

    opt.append( uri );
  }

  list.push_back( opt );

  if ( !mOgrLayerOption.isEmpty() && mOgrLayers[current].size() > 0 )
  {
    opt = mOgrLayerOption + "=";
    // GDAL 1.4.0 supports schemas (r9998)
#if GDAL_VERSION_NUM >= 1400
    opt += mOgrLayers[current];
#else
    // Handle older versions of gdal gracefully
    // OGR does not support schemas !!!
    if ( current >= 0 && current <  mUri.size() )
    {
      QStringList l = mOgrLayers[current].split( "." );
      opt += l.at( 1 );

      // Currently only PostGIS is using layer
      //  -> layer -> PostGIS -> warning
      if ( mOgrLayers[current].length() > 0 )
      {
        QMessageBox::warning( 0, tr( "Warning" ),
                              tr( "PostGIS driver in OGR does not support schemas!<br>"
                                  "Only the table name will be used.<br>"
                                  "It can result in wrong input if more tables of the same name<br>"
                                  "are present in the database." ) );
      }
    }
#endif //GDAL_VERSION_NUM
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
  QgsDebugMsg( "entered" );

  QString error;

  QgsDebugMsg( QString( "count = %1" ).arg( mLayerComboBox->count() ) );
  if ( mLayerComboBox->count() == 0 )
  {
    error.append( tr( "%1:&nbsp;no input" ).arg( title() ) );
  }
  return error;
}

void QgsGrassModuleGdalInput::changed( int i )
{
  mLayerPassword->setEnabled( i < mUri.size() && mUri.value( i ).startsWith( "PG:" ) && !mUri.value( i ).contains( "password=" ) );
}

QgsGrassModuleGdalInput::~QgsGrassModuleGdalInput()
{
}

/***************** QgsGrassModuleField *********************/
QgsGrassModuleField::QgsGrassModuleField( QgsGrassModule *module, QString key,
    QDomElement &qdesc, QDomElement &gdesc, QDomNode &gnode, bool direct, QWidget * parent )
    : QgsGrassModuleOption( module, key, qdesc, gdesc, gnode, direct, parent )
{
  QRegExp rx( "^[a-zA-Z_][a-zA-Z0-9_]*$" );
  foreach ( QLineEdit *lineEdit, mLineEdits )
  {
    lineEdit->setValidator( new QRegExpValidator( rx, this ) );
  }
}

QgsGrassModuleField::~QgsGrassModuleField()
{
}

/***************** QgsGrassModuleVectorField *********************/

QgsGrassModuleVectorField::QgsGrassModuleVectorField(
  QgsGrassModule *module, QgsGrassModuleStandardOptions *options,
  QString key, QDomElement &qdesc,
  QDomElement &gdesc, QDomNode &gnode, bool direct, QWidget * parent )
    : QgsGrassModuleGroupBoxItem( module, key, qdesc, gdesc, gnode, direct, parent )
    , mModuleStandardOptions( options ), mLayerInput( 0 )
{
  if ( mTitle.isEmpty() )
  {
    mTitle = tr( "Attribute field" );
  }
  adjustTitle();

  QDomNode promptNode = gnode.namedItem( "gisprompt" );
  QDomElement promptElem = promptNode.toElement();
  QString element = promptElem.attribute( "element" );

  mType = qdesc.attribute( "type" );

  mLayerKey = qdesc.attribute( "layer" );
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
      connect( mLayerInput, SIGNAL( valueChanged() ), this, SLOT( updateFields() ) );
    }
  }

  QHBoxLayout *l = new QHBoxLayout( this );
  mFieldComboBox = new QComboBox();
  l->addWidget( mFieldComboBox );

  // Fill in layer current fields
  updateFields();
}

void QgsGrassModuleVectorField::updateFields()
{
  QgsDebugMsg( "entered" );

  QString current = mFieldComboBox->currentText();
  mFieldComboBox->clear();

  //QgsMapCanvas *canvas = mModule->qgisIface()->mapCanvas();

  if ( mLayerInput == 0 )
    return;

  QgsFields fields = mLayerInput->currentFields();

  for ( int i = 0; i < fields.size(); i++ )
  {
    if ( mType.contains( fields[i].typeName() ) )
    {
      mFieldComboBox->addItem( fields[i].name() );
      if ( fields[i].name() == current )
      {
        mFieldComboBox->setItemText( mFieldComboBox->currentIndex(), current );
      }
    }
  }
}

QStringList QgsGrassModuleVectorField::options()
{
  QStringList list;

  if ( !mFieldComboBox->currentText().isEmpty() )
  {
    QString opt( mKey + "=" + mFieldComboBox->currentText() );
    list.push_back( opt );
  }

  return list;
}

QgsGrassModuleVectorField::~QgsGrassModuleVectorField()
{
}

/***************** QgsGrassModuleSelection *********************/

QgsGrassModuleSelection::QgsGrassModuleSelection(
  QgsGrassModule *module, QgsGrassModuleStandardOptions *options,
  QString key, QDomElement &qdesc,
  QDomElement &gdesc, QDomNode &gnode, bool direct, QWidget * parent )
    : QgsGrassModuleGroupBoxItem( module, key, qdesc, gdesc, gnode, direct, parent )
    , mModuleStandardOptions( options )
    , mLayerInput( 0 )
    , mVectorLayer( 0 )
{
  if ( mTitle.isEmpty() )
  {
    mTitle = tr( "Selected categories" );
  }
  adjustTitle();

  QDomNode promptNode = gnode.namedItem( "gisprompt" );
  QDomElement promptElem = promptNode.toElement();
  QString element = promptElem.attribute( "element" );

  mLayerId = qdesc.attribute( "layerid" );

  mType = qdesc.attribute( "type" );

  QgsGrassModuleParam *item = mModuleStandardOptions->item( mLayerId );
  // TODO check type
  if ( item )
  {
    mLayerInput = dynamic_cast<QgsGrassModuleInput *>( item );
    connect( mLayerInput, SIGNAL( valueChanged() ), this, SLOT( updateSelection() ) );
  }

  QHBoxLayout *l = new QHBoxLayout( this );
  mLineEdit = new QLineEdit( this );
  l->addWidget( mLineEdit );

  // Fill in layer current fields
  updateSelection();
}

void QgsGrassModuleSelection::updateSelection()
{
  QgsDebugMsg( "entered" );

  mLineEdit->setText( "" );
  //QgsMapCanvas *canvas = mModule->qgisIface()->mapCanvas();
  if ( mLayerInput == 0 )
    return;

  // TODO update to new input
#if 0
  QgsMapLayer *layer = mLayerInput->currentLayer();
  if ( !layer )
    return;
  QgsVectorLayer *vector = qobject_cast<QgsVectorLayer *>( layer );

  QgsGrassProvider *provider = ( QgsGrassProvider * ) vector->dataProvider();
  QgsAttributeList allAttributes = provider->attributeIndexes();
  const QgsFeatureIds& selected = vector->selectedFeaturesIds();
  int keyField = provider->keyField();

  if ( keyField < 0 )
    return;

  QString cats;
  QgsFeatureIterator fi = provider->getFeatures( QgsFeatureRequest() );
  QgsFeature feature;

  int i = 0;
  while ( fi.nextFeature( feature ) )
  {
    if ( !selected.contains( feature.id() ) )
      continue;

    QgsAttributes attr = feature.attributes();
    if ( attr.size() > keyField )
    {
      if ( i > 0 )
        cats.append( "," );
      cats.append( attr[keyField].toString() );
      i++;
    }
  }
  if ( mVectorLayer != vector )
  {
    if ( mVectorLayer )
    {
      disconnect( mVectorLayer, SIGNAL( selectionChanged() ), this, SLOT( updateSelection() ) );
    }

    connect( vector, SIGNAL( selectionChanged() ), this, SLOT( updateSelection() ) );
    mVectorLayer = vector;
  }

  mLineEdit->setText( cats );
#endif
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

QgsGrassModuleSelection::~QgsGrassModuleSelection()
{
}

/***************** QgsGrassModuleFile *********************/

QgsGrassModuleFile::QgsGrassModuleFile(
  QgsGrassModule *module,
  QString key, QDomElement &qdesc,
  QDomElement &gdesc, QDomNode &gnode, bool direct, QWidget * parent )
    : QgsGrassModuleGroupBoxItem( module, key, qdesc, gdesc, gnode, direct, parent )
    , mType( Old )
{
  if ( mTitle.isEmpty() )
  {
    mTitle = tr( "File" );
  }
  adjustTitle();

  QDomNode promptNode = gnode.namedItem( "gisprompt" );
  QDomElement promptElem = promptNode.toElement();
  QString element = promptElem.attribute( "element" );

  if ( qdesc.attribute( "type" ).toLower() == "new" )
  {
    mType = New;
  }
  if ( qdesc.attribute( "type" ).toLower() == "multiple" )
  {
    mType = Multiple;
  }

  if ( qdesc.attribute( "type" ).toLower() == "directory" )
  {
    mType = Directory;
  }

  mFilters = qdesc.attribute( "filters" );

  mFileOption = qdesc.attribute( "fileoption" );

  QHBoxLayout *l = new QHBoxLayout( this );
  mLineEdit = new QLineEdit();
  mBrowseButton = new QPushButton( "..." );
  l->addWidget( mLineEdit );
  l->addWidget( mBrowseButton );

  connect( mBrowseButton, SIGNAL( clicked() ),
           this, SLOT( browse() ) );
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
    QString path = mLineEdit->text().split( "," ).first();
    if ( path.isEmpty() )
      path = lastDir;
    else
      path = QFileInfo( path ).absolutePath();

    QStringList files = QFileDialog::getOpenFileNames( this, 0, path, mFilters );
    if ( files.isEmpty() )
      return;

    lastDir = QFileInfo( files[0] ).absolutePath();

    mLineEdit->setText( files.join( "," ) );
  }
  else
  {
    QString selectedFile = mLineEdit->text();
    if ( selectedFile.isEmpty() )
      selectedFile = lastDir;

    if ( mType == New )
      selectedFile = QFileDialog::getSaveFileName( this, 0, selectedFile );
    else if ( mType == Directory )
      selectedFile = QFileDialog::getExistingDirectory( this, 0, selectedFile );
    else
      selectedFile = QFileDialog::getOpenFileName( this, 0, selectedFile, mFilters );

    lastDir = QFileInfo( selectedFile ).absolutePath();

    mLineEdit->setText( selectedFile );
  }
}

QString QgsGrassModuleFile::ready()
{
  QgsDebugMsg( "key = " + key() );

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

QgsGrassModuleFile::~QgsGrassModuleFile()
{
}

/***************************** QgsGrassModuleCheckBox *********************************/

QgsGrassModuleCheckBox::QgsGrassModuleCheckBox( const QString & text, QWidget * parent )
    : QCheckBox( text, parent ), mText( text )
{
  QgsDebugMsg( "entered" );
  adjustText();
}

QgsGrassModuleCheckBox::~QgsGrassModuleCheckBox()
{
}

void QgsGrassModuleCheckBox::resizeEvent( QResizeEvent * event )
{
  Q_UNUSED( event );
  adjustText();
}
void QgsGrassModuleCheckBox::setText( const QString & text )
{
  mText = text;
  adjustText();
}
void QgsGrassModuleCheckBox::setToolTip( const QString & text )
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

