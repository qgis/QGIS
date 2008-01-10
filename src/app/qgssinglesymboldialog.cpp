/***************************************************************************
                         qgssinglesymboldialog.cpp  -  description
                             -------------------
    begin                : Oct 2003
    copyright            : (C) 2003 by Marco Hugentobler
    email                : mhugent@geo.unizh.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include "qgssinglesymboldialog.h"
#include "qgsmarkercatalogue.h"
#include "qgssinglesymbolrenderer.h"
#include "qgsfield.h"
#include "qgssymbol.h"
#include "qgssymbologyutils.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgslogger.h"

#include <QColorDialog>
#include <QPainter>
#include <QImage>
#include <QFileDialog>
#include <QListWidgetItem>

#define DO_NOT_USE_STR "<off>"

QgsSingleSymbolDialog::QgsSingleSymbolDialog(): QDialog(), mVectorLayer(0)
{
    setupUi(this);
#ifdef QGISDEBUG
    qWarning("constructor QgsSingleSymbolDialog called WITHOUT a layer");
#endif
}

QgsSingleSymbolDialog::QgsSingleSymbolDialog(QgsVectorLayer * layer): QDialog(), mVectorLayer(layer)
{
  setupUi(this);

#ifdef QGISDEBUG
  qWarning("constructor QgsSingleSymbolDialog called WITH a layer");
#endif

  //
  //set point symbol list
  //

  // If this layer doesn't have points, break out of the following
  // two loops after the first iteration. This gives one point
  // symbol in the dialog, etc so that other code can rely on such a
  // fact, but avoids the long time required to load all of the
  // available symbols when they are not needed.

  // NOTE BY Tim: I think the note above and the break out in the 
  // loops can be removed now with changes I have made to use combo
  // boxes for line style and fill style...test and remove if poss.


  QPen pen (QColor(0,0,255));
  QBrush brush ( QColor(220,220,220), Qt::SolidPattern );
  int size = 18;
  int myCounter = 0;
  QStringList ml = QgsMarkerCatalogue::instance()->list();
  for ( QStringList::iterator it = ml.begin(); it != ml.end(); ++it ) 
  {
    QPixmap myPixmap = QPixmap::fromImage(QgsMarkerCatalogue::instance()->imageMarker ( *it, size, pen, brush ));
    QListWidgetItem * mypItem = new QListWidgetItem(lstSymbols);
    QIcon myIcon;
    myIcon.addPixmap ( myPixmap );
    mypItem->setIcon ( myIcon );
    mypItem->setText ( "" );
    //store the symbol offset in the UserData role for later retrieval
    mypItem->setData ( Qt::UserRole, *it);
    if (layer->vectorType() != QGis::Point)
    {
      break;
    }
    ++myCounter;
  }

    // Find out the numerical fields of mVectorLayer, and populate the ComboBox
    QgsVectorDataProvider *provider = mVectorLayer->getDataProvider();
    if (provider)
    {
      const QgsFieldMap & fields = provider->fields();
      QString str;
      
      mRotationClassificationComboBox->insertItem(DO_NOT_USE_STR);
      mScaleClassificationComboBox->insertItem(DO_NOT_USE_STR);
      mFieldMap.insert(std::make_pair(DO_NOT_USE_STR, -1));
      for (QgsFieldMap::const_iterator it = fields.begin(); 
           it != fields.end(); 
           ++it)
      {
        QVariant::Type type = (*it).type();
        if (type == QVariant::Int || type == QVariant::Double)
        {
          mRotationClassificationComboBox->insertItem(it->name());
          mScaleClassificationComboBox->insertItem(it->name());
          mFieldMap.insert(std::make_pair(it->name(), it.key()));
        }
      }
    } 
    else
    {
      qWarning("Warning, data provider is null in QgsSingleSymbolDialog::QgsSingleSymbolDialog(...)");
      return;
    }
  //
  //set outline / line style
  //
  cboOutlineStyle->addItem(QIcon(QgsSymbologyUtils::char2LinePixmap("SolidLine")),tr("Solid Line"),"SolidLine");
  cboOutlineStyle->addItem(QIcon(QgsSymbologyUtils::char2LinePixmap("DashLine")),tr("Dash Line"),"DashLine");
  cboOutlineStyle->addItem(QIcon(QgsSymbologyUtils::char2LinePixmap("DotLine")),tr("Dot Line"),"DotLine");
  cboOutlineStyle->addItem(QIcon(QgsSymbologyUtils::char2LinePixmap("DashDotLine")),tr("Dash Dot Line"),"DashDotLine");
  cboOutlineStyle->addItem(QIcon(QgsSymbologyUtils::char2LinePixmap("DashDotDotLine")),tr("Dash Dot Dot Line"),"DashDotDotLine");
  cboOutlineStyle->addItem(QIcon(QgsSymbologyUtils::char2LinePixmap("NoPen")),tr("No Pen"),"NoPen");

  //
  //set pattern icons and state
  //
  cboFillStyle->addItem(QIcon(QgsSymbologyUtils::char2PatternPixmap("SolidPattern")),tr("Solid Pattern"),"SolidPattern");
  cboFillStyle->addItem(QIcon(QgsSymbologyUtils::char2PatternPixmap("HorPattern")),tr("Hor Pattern"),"HorPattern");
  cboFillStyle->addItem(QIcon(QgsSymbologyUtils::char2PatternPixmap("VerPattern")),tr("Ver Pattern"),"VerPattern");
  cboFillStyle->addItem(QIcon(QgsSymbologyUtils::char2PatternPixmap("CrossPattern")),tr("Cross Pattern"),"CrossPattern");
  cboFillStyle->addItem(QIcon(QgsSymbologyUtils::char2PatternPixmap("BDiagPattern")),tr("BDiag Pattern"),"BDiagPattern");
  cboFillStyle->addItem(QIcon(QgsSymbologyUtils::char2PatternPixmap("FDiagPattern")),tr("FDiag Pattern"),"FDiagPattern");
  cboFillStyle->addItem(QIcon(QgsSymbologyUtils::char2PatternPixmap("DiagCrossPattern")),tr("Diag Cross Pattern"),"DiagCrossPattern");
  cboFillStyle->addItem(QIcon(QgsSymbologyUtils::char2PatternPixmap("Dense1Pattern")),tr("Dense1 Pattern"),"Dense1Pattern");
  cboFillStyle->addItem(QIcon(QgsSymbologyUtils::char2PatternPixmap("Dense2Pattern")),tr("Dense2 Pattern"),"Dense2Pattern");
  cboFillStyle->addItem(QIcon(QgsSymbologyUtils::char2PatternPixmap("Dense3Pattern")),tr("Dense3 Pattern"),"Dense3Pattern");
  cboFillStyle->addItem(QIcon(QgsSymbologyUtils::char2PatternPixmap("Dense4Pattern")),tr("Dense4 Pattern"),"Dense4Pattern");
  cboFillStyle->addItem(QIcon(QgsSymbologyUtils::char2PatternPixmap("Dense5Pattern")),tr("Dense5 Pattern"),"Dense5Pattern");
  cboFillStyle->addItem(QIcon(QgsSymbologyUtils::char2PatternPixmap("Dense6Pattern")),tr("Dense6 Pattern"),"Dense6Pattern");
  cboFillStyle->addItem(QIcon(QgsSymbologyUtils::char2PatternPixmap("Dense7Pattern")),tr("Dense7 Pattern"),"Dense7Pattern");
  cboFillStyle->addItem(QIcon(QgsSymbologyUtils::char2PatternPixmap("NoBrush")),tr("No Brush"),"NoBrush");
  cboFillStyle->addItem(QIcon(QgsSymbologyUtils::char2PatternPixmap("TexturePattern")),tr("Texture Pattern"),"TexturePattern");

  if (mVectorLayer)
  {
    const QgsSingleSymbolRenderer *renderer=dynamic_cast<const QgsSingleSymbolRenderer*>(mVectorLayer->renderer());

    if (renderer)
    {
      // Set from the existing renderer
      set ( renderer->symbols().first() );
    }
    else
    {
      // Take values from an example instance
      QgsSingleSymbolRenderer exampleRenderer = QgsSingleSymbolRenderer( mVectorLayer->vectorType() );
      set ( exampleRenderer.symbols().first() );
    }

    if (mVectorLayer && mVectorLayer->vectorType() == QGis::Line)
    {
      btnFillColor->setEnabled(false);
      cboFillStyle->setEnabled(false);
      mGroupPoint->setEnabled(false);
      mGroupPoint->setVisible(false);
    }

    if (mVectorLayer && mVectorLayer->vectorType() == QGis::Polygon) 
    {
      mGroupPoint->setEnabled(false);
      mGroupPoint->setVisible(false);
    }

  }
  else
  {
    qWarning("Warning, layer is a null pointer in "
        "QgsSingleSymbolDialog::QgsSingleSymbolDialog(QgsVectorLayer)");
  }

  //do the signal/slot connections
  connect(btnOutlineColor, SIGNAL(clicked()), this, SLOT(selectOutlineColor()));
  connect(btnFillColor, SIGNAL(clicked()), this, SLOT(selectFillColor()));
  connect(outlinewidthspinbox, SIGNAL(valueChanged(int)), this, SLOT(resendSettingsChanged()));
  connect(mLabelEdit, SIGNAL(textChanged(const QString&)), this, SLOT(resendSettingsChanged()));
  connect (lstSymbols,SIGNAL(currentItemChanged ( QListWidgetItem * , QListWidgetItem * )),
        this, SLOT (symbolChanged (QListWidgetItem * , QListWidgetItem * )));
  connect(mPointSizeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(resendSettingsChanged()));
  connect(mRotationClassificationComboBox, SIGNAL(currentIndexChanged(const QString &)),
          this, SLOT(resendSettingsChanged()));
  connect(mScaleClassificationComboBox, SIGNAL(currentIndexChanged(const QString &)),
          this, SLOT(resendSettingsChanged()));
  connect(cboOutlineStyle, SIGNAL(
        currentIndexChanged ( const QString & )), this, SLOT(resendSettingsChanged()));
  connect(cboFillStyle, SIGNAL(
        currentIndexChanged ( const QString & )), this, SLOT(resendSettingsChanged()));
  //need this to deal with when texture fill is selected or deselected
  connect(cboFillStyle, SIGNAL(
        currentIndexChanged ( int )), this, SLOT(fillStyleChanged(int)));
  connect(toolSelectTexture, SIGNAL(clicked()), this, SLOT(selectTextureImage()));
}

QgsSingleSymbolDialog::~QgsSingleSymbolDialog()
{
#ifdef QGISDEBUG
    qWarning("destructor QgsSingleSymbolDialog");
#endif
}

void QgsSingleSymbolDialog::selectOutlineColor()
{
    QColor c = QColorDialog::getColor(btnOutlineColor->color(), this);
    
    if ( c.isValid() ) {
        btnOutlineColor->setColor(c);
        emit settingsChanged();
    }
    
    activateWindow();
}

void QgsSingleSymbolDialog::selectFillColor()
{
    QColor c = QColorDialog::getColor(btnFillColor->color(), this);

    if ( c.isValid() ) {
        btnFillColor->setColor(c);
        emit settingsChanged();
    }

    activateWindow();
}

//should this method have a different name?
void QgsSingleSymbolDialog::selectTextureImage()
{
  QString fileName = QFileDialog::getOpenFileName(this, "Open File",
           mTexturePath,
           "Images (*.png *.xpm *.jpg)"); //should we allow other types of images?

  if(fileName.isNull() == false)
  { //only process the string if the user clicked OK
    mTexturePath = fileName;
    resendSettingsChanged();
  }
}

void QgsSingleSymbolDialog::apply( QgsSymbol *sy )
{
    //query the values of the widgets and set the symbology of the vector layer
    sy->setFillColor(btnFillColor->color());
    sy->setLineWidth(outlinewidthspinbox->value());
    sy->setColor(btnOutlineColor->color());

    //
    // Apply point symbol
    // 
    if ( lstSymbols->currentItem() )
    {
      sy->setNamedPointSymbol( lstSymbols->currentItem()->data(Qt::UserRole).toString() ) ;
    }
    sy->setPointSize ( mPointSizeSpinBox->value() );

    sy->setRotationClassificationField(-1);
    sy->setScaleClassificationField(-1);

    std::map<QString,int>::iterator iter=mFieldMap.find(mRotationClassificationComboBox->currentText());
    if(iter!=mFieldMap.end())
    {
      sy->setRotationClassificationField(iter->second);
    }

    iter = mFieldMap.find(mScaleClassificationComboBox->currentText());
    if(iter!=mFieldMap.end())
    {
      sy->setScaleClassificationField(iter->second);
    }
    
    //
    // Apply the line style
    //
    QString myLineStyle = 
      cboOutlineStyle->itemData(cboOutlineStyle->currentIndex(),Qt::UserRole).toString();
     sy->setLineStyle(QgsSymbologyUtils::qString2PenStyle(myLineStyle));

    //
    // Apply the pattern
    //

    //Store the file path, and set the brush to TexturePattern.  If we have a different button selected,
    // the below code will override it, but leave the file path alone.
   
    sy->setCustomTexture(mTexturePath);

    QString myFillStyle = 
      cboFillStyle->itemData(cboFillStyle->currentIndex(),Qt::UserRole).toString();
    sy->setFillStyle(QgsSymbologyUtils::qString2BrushStyle(myFillStyle));

    sy->setLabel(mLabelEdit->text());
}

void QgsSingleSymbolDialog::apply()
{
  QgsSymbol* sy = new QgsSymbol(mVectorLayer->vectorType());
  apply(sy);
  
  QgsSingleSymbolRenderer *renderer = new QgsSingleSymbolRenderer(mVectorLayer->vectorType());
  renderer->addSymbol(sy);
  renderer->updateSymbolAttributes();

  mVectorLayer->setRenderer(renderer);
}

void QgsSingleSymbolDialog::set ( const QgsSymbol *sy ) 
{
  //set label

  mLabelEdit->setText(sy->label());

  // Set point symbol
  QString mySymbolName = sy->pointSymbolName();
  for ( int i = 0; i < lstSymbols->count(); ++i )
  {
    if (lstSymbols->item(i)->data( Qt::UserRole ).toString() == (mySymbolName))
    {
      lstSymbols->setCurrentItem ( lstSymbols->item(i) );
      lstSymbols->item(i)->setBackground( QBrush ( Qt::cyan ) );
      break;
    }
  }
  mPointSizeSpinBox->setValue ( sy->pointSize() );

  QString rotationclassfield = DO_NOT_USE_STR;
  QString scaleclassfield = DO_NOT_USE_STR;
  for(std::map<QString,int>::iterator it=mFieldMap.begin();it!=mFieldMap.end();++it)
  {
    if(it->second == sy->rotationClassificationField())
    {
      rotationclassfield=it->first;
      QgsDebugMsg(QString("Found rotation field " + rotationclassfield));
    }
    if(it->second == sy->scaleClassificationField())
    {
      scaleclassfield=it->first;
      QgsDebugMsg(QString("Found scale field " + scaleclassfield));
    }
  }
  mRotationClassificationComboBox->setCurrentText(rotationclassfield);
  mScaleClassificationComboBox->setCurrentText(scaleclassfield);


  outlinewidthspinbox->setValue(sy->pen().width());

  //set line width 1 as minimum to avoid confusion between line width 0 and no pen line style
  // ... but, drawLine is not correct with width > 0 -> until solved set to 0
  outlinewidthspinbox->setMinValue(0);

  btnFillColor->setColor( sy->brush().color() );

  btnOutlineColor->setColor( sy->pen().color() );

  //load the icons stored in QgsSymbologyUtils.cpp (to avoid redundancy)

  //
  // Set the line style combo
  //
  
  QPen myPen = sy->pen();
  QString myLineStyle = QgsSymbologyUtils::penStyle2QString(myPen.style());
  for ( int i = 0; i < cboOutlineStyle->count(); ++i )
  {
    if (cboOutlineStyle->itemData(i, Qt::UserRole ).toString() == myLineStyle)
    {
      cboOutlineStyle->setCurrentIndex( i );
      break;
    }
  }

  //
  // Set the brush combo
  //
  
  QBrush myBrush = sy->brush();
  QString myFillStyle =  QgsSymbologyUtils::brushStyle2QString(myBrush.style());
  for ( int i = 0; i < cboFillStyle->count(); ++i )
  {
    if (cboFillStyle->itemData(i, Qt::UserRole ).toString() == myFillStyle)
    {
      cboFillStyle->setCurrentIndex( i );
      break;
    }
  }
  
  //get and show the file path, even if we aren't using it.
  mTexturePath = sy->customTexture(); 
  //if the file path isn't empty, show the image on the button
  if(sy->customTexture().size() > 0)
  {
    //show the current texture image
   // texture->setPixmap(QPixmap(sy->customTexture())); 
  }
  else
  {
    //show the default question mark
    //texture->setPixmap(QgsSymbologyUtils::char2PatternPixmap("TexturePattern")); 
  }
}

void QgsSingleSymbolDialog::setOutlineColor(QColor& c)
{
    btnOutlineColor->setColor(c);
}

void QgsSingleSymbolDialog::setOutlineStyle(Qt::PenStyle pstyle)
{
  QString myLineStyle = QgsSymbologyUtils::penStyle2QString(pstyle);
  for ( int i = 0; i < cboOutlineStyle->count(); ++i )
  {
    if (cboOutlineStyle->itemData(i, Qt::UserRole ).toString() == myLineStyle)
    {
      cboOutlineStyle->setCurrentIndex( i );
      break;
    }
  }
}

void QgsSingleSymbolDialog::setFillColor(QColor& c)
{
    btnFillColor->setColor(c);
}

void QgsSingleSymbolDialog::setFillStyle(Qt::BrushStyle fstyle)
{
#ifdef QGISDEBUG
  qWarning(("Setting fill style: "+QgsSymbologyUtils::brushStyle2QString(fstyle)).toLocal8Bit().data());
#endif
  QString myFillStyle =  QgsSymbologyUtils::brushStyle2QString(fstyle);
  for ( int i = 0; i < cboFillStyle->count(); ++i )
  {
    if (cboFillStyle->itemData(i, Qt::UserRole ).toString() == myFillStyle)
    {
      cboFillStyle->setCurrentIndex( i );
      break;
    }
  }
}

void QgsSingleSymbolDialog::setOutlineWidth(int width)
{
    outlinewidthspinbox->setValue(width);
}

QColor QgsSingleSymbolDialog::getOutlineColor()
{
    return btnOutlineColor->color();
}

Qt::PenStyle QgsSingleSymbolDialog::getOutlineStyle()
{
    QString myLineStyle = 
      cboOutlineStyle->itemData(cboOutlineStyle->currentIndex(),Qt::UserRole).toString();
    return QgsSymbologyUtils::qString2PenStyle(myLineStyle);
}

int QgsSingleSymbolDialog::getOutlineWidth()
{
    return outlinewidthspinbox->value();
}

QColor QgsSingleSymbolDialog::getFillColor()
{
    return btnFillColor->color();
}

Qt::BrushStyle QgsSingleSymbolDialog::getFillStyle()
{
    QString myFillStyle = 
      cboFillStyle->itemData(cboFillStyle->currentIndex(),Qt::UserRole).toString();
    return QgsSymbologyUtils::qString2BrushStyle(myFillStyle);
}

void QgsSingleSymbolDialog::resendSettingsChanged()
{
    emit settingsChanged();
}

QString QgsSingleSymbolDialog::label()
{
    return mLabelEdit->text();
}

void QgsSingleSymbolDialog::setLabel(QString label)
{
    mLabelEdit->setText(label);
}

void QgsSingleSymbolDialog::symbolChanged 
    ( QListWidgetItem * current, QListWidgetItem * previous )
{
    current->setBackground( QBrush ( Qt::cyan ) );
    if (previous)
    {
      previous->setBackground( QBrush ( Qt::white ) );
    }
    emit settingsChanged();
}

void QgsSingleSymbolDialog::fillStyleChanged( int theIndex )
{
  //if the new style is texture we need to enable the texture
  //selection button, otherwise disable it
  QString myFillStyle = 
      cboFillStyle->itemData( theIndex ,Qt::UserRole ).toString();
  if ( "TexturePattern" == myFillStyle )
  {
    toolSelectTexture->setEnabled( true );
  }
  else
  {
    toolSelectTexture->setEnabled( false );
  }

}
