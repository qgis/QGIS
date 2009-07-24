
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
  
  QDomNode node = docElem.firstChild();
  QString name;
  
  while (!node.isNull())
  {
    QDomElement e = node.toElement();
    if (!e.isNull()) // comments are null elements
    {
      if (e.tagName() == "symbol")
      {
        name = e.attribute("name");
        QgsSymbolV2* symbol = loadSymbol(e);
        if (symbol != NULL)
          addSymbol(name, symbol);
      }
      else if (e.tagName() == "colorramp")
      {
        name = e.attribute("name");
        QgsVectorColorRampV2* ramp = loadColorRamp(e);
        if (ramp != NULL)
          addColorRamp(name, ramp);
      }
      else
      {
        QgsDebugMsg("unknown tag: " + e.tagName());
      }
    } 
    node = node.nextSibling();
  }
  
  // now walk through the list of symbols and find those prefixed with @
  // these symbols are sub-symbols of some other symbol layers
  // e.g. symbol named "@foo@1" is sub-symbol of layer 1 in symbol "foo"
  QStringList subsymbols;
  
  for (QMap<QString, QgsSymbolV2*>::iterator it = mSymbols.begin(); it != mSymbols.end(); ++it)
  {
    if (it.key()[0] != '@')
      continue;
    
    // add to array (for deletion)
    subsymbols.append(it.key());
    
    QStringList parts = it.key().split("@");
    if (parts.count() < 3)
    {
      QgsDebugMsg("found subsymbol with invalid name: "+it.key());
      delete it.value(); // we must delete it
      continue; // some invalid syntax
    }
    QString symname = parts[1];
    int symlayer = parts[2].toInt();
    
    if (!mSymbols.contains(symname))
    {
      QgsDebugMsg("subsymbol references invalid symbol: " + symname);
      delete it.value(); // we must delete it
      continue;
    }
    
    QgsSymbolV2* sym = mSymbols[symname];
    if (symlayer < 0 || symlayer >= sym->symbolLayerCount())
    {
      QgsDebugMsg("subsymbol references invalid symbol layer: "+ QString::number(symlayer));
      delete it.value(); // we must delete it
      continue;
    }
    
    // set subsymbol takes ownership
    bool res = sym->symbolLayer(symlayer)->setSubSymbol( it.value() );
    if (!res)
    {
      QgsDebugMsg("symbol layer refused subsymbol: " + it.key());
    }
    
    
  }
  
  // now safely remove sub-symbol entries (they have been already deleted or the ownership was taken away)
  for (int i = 0; i < subsymbols.count(); i++)
    mSymbols.take(subsymbols[i]);
  
  return true;
}


QgsSymbolV2* QgsStyleV2::loadSymbol(QDomElement& element)
{
  QgsSymbolLayerV2List layers;
  QDomNode layerNode = element.firstChild();
  
  while (!layerNode.isNull())
  {
    QDomElement e = layerNode.toElement();
    if (!e.isNull())
    {
      if (e.tagName() != "layer")
      {
        QgsDebugMsg("unknown tag " + e.tagName());
      }
      else
      {
        QgsSymbolLayerV2* layer = loadSymbolLayer(e);
        if (layer != NULL)
          layers.append(layer);
      }
    }
    layerNode = layerNode.nextSibling();
  }
  
  if (layers.count() == 0)
  {
    QgsDebugMsg("no layers for symbol");
    return NULL;
  }
  
  QString symbolType = element.attribute("type");
  if (symbolType == "line")
    return new QgsLineSymbolV2(layers);
  else if (symbolType == "fill")
    return new QgsFillSymbolV2(layers);
  else if (symbolType == "marker")
    return new QgsMarkerSymbolV2(layers);
  else
  {
    QgsDebugMsg("unknown symbol type " + symbolType);
    return NULL;
  }  
}

QgsSymbolLayerV2* QgsStyleV2::loadSymbolLayer(QDomElement& element)
{
  QString layerClass = element.attribute("class");
  bool locked = element.attribute("locked").toInt();
  
  // parse properties
  QgsStringMap props = parseProperties(element);
  
  QgsSymbolLayerV2* layer;
  layer = QgsSymbolLayerV2Registry::instance()->createSymbolLayer(layerClass, props);
  if (layer)
  {
    layer->setLocked(locked);
    return layer;
  }
  else
  {
    QgsDebugMsg("unknown class " + layerClass);
    return NULL;
  }
}

QgsVectorColorRampV2* QgsStyleV2::loadColorRamp(QDomElement& element)
{
  QString rampType = element.attribute("type");
  
  // parse properties
  QgsStringMap props = parseProperties(element);
  
  if (rampType == "gradient")
    return QgsVectorGradientColorRampV2::create(props);
  else if (rampType == "random")
    return QgsVectorRandomColorRampV2::create(props);
  else
  {
    QgsDebugMsg("unknown colorramp type " + rampType);
    return NULL;
  }
}


QgsStringMap QgsStyleV2::parseProperties(QDomElement& element)
{
  QgsStringMap props;
  QDomNode propNode = element.firstChild();
  while (!propNode.isNull())
  {
    QDomElement e = propNode.toElement();
    if (!e.isNull())
    {
      if (e.tagName() != "prop")
      {
        QgsDebugMsg("unknown tag " + e.tagName());
      }
      else
      {
        QString propKey = e.attribute("k");
        QString propValue = e.attribute("v");
        props[propKey] = propValue;
      }
    }
    propNode = propNode.nextSibling();
  }
  return props;
}

void QgsStyleV2::saveProperties(QgsStringMap props, QDomDocument& doc, QDomElement& element)
{
  for (QgsStringMap::iterator it = props.begin(); it != props.end(); ++it)
  {
    QDomElement propEl = doc.createElement("prop");
    propEl.setAttribute("k", it.key());
    propEl.setAttribute("v", it.value());
    element.appendChild(propEl);
  }
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
  
  QMap<QString, QgsSymbolV2*> subSymbols;
  
  // save symbols
  for (QMap<QString, QgsSymbolV2*>::iterator its = mSymbols.begin(); its != mSymbols.end(); ++its)
  {
    QDomElement symEl = saveSymbol(its.key(), its.value(), doc, &subSymbols);
    root.appendChild(symEl);
  }
  
  // add subsymbols, don't allow subsymbols for them (to keep things simple)
  for (QMap<QString, QgsSymbolV2*>::iterator itsub = subSymbols.begin(); itsub != subSymbols.end(); ++itsub)
  {
    QDomElement subsymEl = saveSymbol(itsub.key(), itsub.value(), doc);
    root.appendChild(subsymEl);
  }
  
  // save color ramps
  for (QMap<QString, QgsVectorColorRampV2*>::iterator itr = mColorRamps.begin(); itr != mColorRamps.end(); ++itr)
  {
    QDomElement rampEl = saveColorRamp(itr.key(), itr.value(), doc);
    root.appendChild(rampEl);
  }
  
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

static QString _nameForSymbolType(QgsSymbolV2::SymbolType type)
{
  switch (type)
  {
    case QgsSymbolV2::Line: return "line";
    case QgsSymbolV2::Marker: return "marker";
    case QgsSymbolV2::Fill: return "fill";
  }
}

QDomElement QgsStyleV2::saveSymbol(QString name, QgsSymbolV2* symbol, QDomDocument& doc, QgsSymbolV2Map* subSymbols)
{
  QDomElement symEl = doc.createElement("symbol");
  symEl.setAttribute("type", _nameForSymbolType(symbol->type()) );
  symEl.setAttribute("name", name);
  
  for (int i = 0; i < symbol->symbolLayerCount(); i++)
  {
    QgsSymbolLayerV2* layer = symbol->symbolLayer(i);
    
    QDomElement layerEl = doc.createElement("layer");
    layerEl.setAttribute("class", layer->layerType());
    layerEl.setAttribute("locked", layer->isLocked());
    
    if (subSymbols != NULL && layer->subSymbol() != NULL)
    {
      QString subname = QString("@%1@%2").arg(name).arg(i);
      subSymbols->insert(subname, layer->subSymbol());
    }
    
    saveProperties(layer->properties(), doc, layerEl);
    symEl.appendChild(layerEl);
  }

  return symEl;
}

QDomElement QgsStyleV2::saveColorRamp(QString name, QgsVectorColorRampV2* ramp, QDomDocument& doc)
{
  QDomElement rampEl = doc.createElement("colorramp");
  rampEl.setAttribute("type", ramp->type());
  rampEl.setAttribute("name", name);
  
  saveProperties(ramp->properties(), doc, rampEl);
  return rampEl;
}
