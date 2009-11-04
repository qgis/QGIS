
#include "qgsstylev2.h"

#include "qgssymbolv2.h"
#include "qgsvectorcolorrampv2.h"

#include "qgssymbollayerv2registry.h"

#include "qgsapplication.h"
#include "qgslogger.h"

#include <QDomDocument>
#include <QDomElement>
#include <QFile>
#include <QTextStream>

#define STYLE_CURRENT_VERSION  "0"

QgsStyleV2* QgsStyleV2::mDefaultStyle = NULL;


QgsStyleV2::QgsStyleV2()
{
}

QgsStyleV2::~QgsStyleV2()
{
  clear();
}

QgsStyleV2* QgsStyleV2::defaultStyle() // static
{
  if (mDefaultStyle == NULL)
  {
    QString styleFilename = QgsApplication::userStyleV2Path();

    // copy default style if user style doesn't exist
    if ( !QFile::exists( styleFilename ) )
    {
      QFile::copy( QgsApplication::defaultStyleV2Path(), styleFilename );
    }

    mDefaultStyle = new QgsStyleV2;
    mDefaultStyle->load( styleFilename );
  }
  return mDefaultStyle;
}


void QgsStyleV2::clear()
{
  for (QMap<QString, QgsSymbolV2*>::iterator its = mSymbols.begin(); its != mSymbols.end(); ++its)
    delete its.value();
  for (QMap<QString, QgsVectorColorRampV2*>::iterator itr = mColorRamps.begin(); itr != mColorRamps.end(); ++itr)
    delete itr.value();
  
  mSymbols.clear();
  mColorRamps.clear();
}

bool QgsStyleV2::addSymbol(QString name, QgsSymbolV2* symbol)
{
  if (!symbol || name.count() == 0)
    return false;
  
  // delete previous symbol (if any)
  if (mSymbols.contains(name))
    delete mSymbols.value(name);
  
  mSymbols.insert(name, symbol);
  return true;
}

bool QgsStyleV2::removeSymbol(QString name)
{
  if (!mSymbols.contains(name))
    return false;
  
  // remove from map and delete
  delete mSymbols.take(name);
  return true;
}

QgsSymbolV2* QgsStyleV2::symbol(QString name)
{
  if (!mSymbols.contains(name))
    return NULL;
  return mSymbols[name]->clone();
}

const QgsSymbolV2* QgsStyleV2::symbolRef(QString name) const
{
  if (!mSymbols.contains(name))
    return NULL;
  return mSymbols[name];
}

int QgsStyleV2::symbolCount()
{
  return mSymbols.count();
}

QStringList QgsStyleV2::symbolNames()
{
  return mSymbols.keys();
}


bool QgsStyleV2::addColorRamp(QString name, QgsVectorColorRampV2* colorRamp)
{
  if (!colorRamp || name.count() == 0)
    return false;
  
  // delete previous symbol (if any)
  if (mColorRamps.contains(name))
    delete mColorRamps.value(name);
  
  mColorRamps.insert(name, colorRamp);
  return true;
}

bool QgsStyleV2::removeColorRamp(QString name)
{
  if (!mColorRamps.contains(name))
    return false;
  
  // remove from map and delete
  delete mColorRamps.take(name);
  return true;
}

QgsVectorColorRampV2* QgsStyleV2::colorRamp(QString name)
{
  if (!mColorRamps.contains(name))
    return NULL;
  return mColorRamps[name]->clone();
}

const QgsVectorColorRampV2* QgsStyleV2::colorRampRef(QString name) const
{
  if (!mColorRamps.contains(name))
    return NULL;
  return mColorRamps[name];
}

int QgsStyleV2::colorRampCount()
{
  return mColorRamps.count();
}
  
QStringList QgsStyleV2::colorRampNames()
{
  return mColorRamps.keys();
}


bool QgsStyleV2::load(QString filename)
{
  mErrorString = QString();
  
  // import xml file
  QDomDocument doc("style");
  QFile f(filename);
  if (!f.open(QFile::ReadOnly))
  {
    mErrorString = "Couldn't open the style file: " + filename;
    return false;
  }
  
  // parse the document
  if (!doc.setContent(&f))
  {
    mErrorString = "Couldn't parse the style file: " + filename;
    f.close();
    return false;
  }
  f.close();
  
  QDomElement docElem = doc.documentElement();
  if (docElem.tagName() != "qgis_style")
  {
    mErrorString = "Incorrect root tag in style: " + docElem.tagName();
    return false;
  }

  // check for style version
  QString version = docElem.attribute("version");
  if (version != STYLE_CURRENT_VERSION)
  {
    mErrorString = "Unknown style file version: " + version;
    return false;
  }

  // load symbols
  QDomElement symbolsElement = docElem.firstChildElement("symbols");
  if (!symbolsElement.isNull())
  {
    mSymbols = QgsSymbolLayerV2Utils::loadSymbols(symbolsElement);
  }

  // load color ramps
  QDomElement rampsElement = docElem.firstChildElement("colorramps");
  QDomElement e = rampsElement.firstChildElement();
  while (!e.isNull())
  {
    if (e.tagName() == "colorramp")
    {
      QgsVectorColorRampV2* ramp = QgsSymbolLayerV2Utils::loadColorRamp(e);
      if (ramp != NULL)
        addColorRamp(e.attribute("name"), ramp);
    }
    else
    {
      QgsDebugMsg("unknown tag: " + e.tagName());
    }
    e = e.nextSiblingElement();
  }

  return true;
}



bool QgsStyleV2::save(QString filename)
{
  mErrorString = QString();
  //if (filename.isEmpty())
  //  filename = mFilename;
  
  QDomDocument doc("qgis_style");
  QDomElement root = doc.createElement("qgis_style");
  root.setAttribute("version", STYLE_CURRENT_VERSION);
  doc.appendChild(root);

  QDomElement symbolsElem = QgsSymbolLayerV2Utils::saveSymbols(mSymbols, "symbols", doc);

  QDomElement rampsElem = doc.createElement("colorramps");
  
  // save color ramps
  for (QMap<QString, QgsVectorColorRampV2*>::iterator itr = mColorRamps.begin(); itr != mColorRamps.end(); ++itr)
  {
    QDomElement rampEl = QgsSymbolLayerV2Utils::saveColorRamp(itr.key(), itr.value(), doc);
    rampsElem.appendChild(rampEl);
  }

  root.appendChild(symbolsElem);
  root.appendChild(rampsElem);
  
  // save
  QFile f(filename);
  if (!f.open(QFile::WriteOnly))
  {
    mErrorString = "Couldn't open file for writing: "+filename;
    return false;
  }
  QTextStream ts(&f);
  doc.save(ts, 2);
  f.close();
  
  return true;
}
