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
#include "qgssymbol.h"
#include "qgssymbologyutils.h"
#include "qgsvectorlayer.h"

#include <QColorDialog>
#include <QPainter>


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
    //set point symbol combo box
    //

    // If this layer doesn't have points, break out of the following
    // two loops after the first iteration. This gives one point
    // symbol in the dialog, etc so that other code can rely on such a
    // fact, but avoids the long time required to load all of the
    // available symbols when they are not needed.

    QStringList ml = QgsMarkerCatalogue::instance()->list();
    mMarkers.clear();

    int size = 18;
    QPen pen (QColor(0,0,255));
    QBrush brush ( QColor(220,220,220), Qt::SolidPattern );

    
    for ( QStringList::iterator it = ml.begin(); it != ml.end(); ++it ) 
    {
      mMarkers.push_back ( *it );
      QPixmap pic = QgsMarkerCatalogue::instance()->pixmapMarker ( *it, size,
          pen, brush );
      mPointSymbolComboBox->insertItem ( pic );
      if (layer->vectorType() != QGis::Point)
      {
        break;
      }
    }

    //
    //set outline / line style
    //
    pbnLineSolid->setPixmap(QgsSymbologyUtils::char2LinePixmap("SolidLine"));
    pbnLineDash->setPixmap(QgsSymbologyUtils::char2LinePixmap("DashLine"));
    pbnLineDot->setPixmap(QgsSymbologyUtils::char2LinePixmap("DotLine"));
    pbnLineDashDot->setPixmap(QgsSymbologyUtils::char2LinePixmap("DashDotLine"));
    pbnLineDashDotDot->setPixmap(QgsSymbologyUtils::char2LinePixmap("DashDotDotLine"));
    pbnLineNoPen->setPixmap(QgsSymbologyUtils::char2LinePixmap("NoPen"));

    //
    //set pattern button group icons and state
    //
    solid->setPixmap(QgsSymbologyUtils::char2PatternPixmap("SolidPattern"));
    horizontal->setPixmap(QgsSymbologyUtils::char2PatternPixmap("HorPattern"));
    vertical->setPixmap(QgsSymbologyUtils::char2PatternPixmap("VerPattern"));
    cross->setPixmap(QgsSymbologyUtils::char2PatternPixmap("CrossPattern"));
    bdiag->setPixmap(QgsSymbologyUtils::char2PatternPixmap("BDiagPattern"));
    fdiag->setPixmap(QgsSymbologyUtils::char2PatternPixmap("FDiagPattern"));
    diagcross->setPixmap(QgsSymbologyUtils::char2PatternPixmap("DiagCrossPattern"));
    dense1->setPixmap(QgsSymbologyUtils::char2PatternPixmap("Dense1Pattern"));
    dense2->setPixmap(QgsSymbologyUtils::char2PatternPixmap("Dense2Pattern"));
    dense3->setPixmap(QgsSymbologyUtils::char2PatternPixmap("Dense3Pattern"));
    dense4->setPixmap(QgsSymbologyUtils::char2PatternPixmap("Dense4Pattern"));
    dense5->setPixmap(QgsSymbologyUtils::char2PatternPixmap("Dense5Pattern"));
    dense6->setPixmap(QgsSymbologyUtils::char2PatternPixmap("Dense6Pattern"));
    dense7->setPixmap(QgsSymbologyUtils::char2PatternPixmap("Dense7Pattern"));
    nopen->setPixmap(QgsSymbologyUtils::char2PatternPixmap("NoBrush"));


    if (mVectorLayer)
    {
        const QgsSingleSymbolRenderer *renderer=dynamic_cast<const QgsSingleSymbolRenderer*>(mVectorLayer->renderer());

        if (renderer)
        {
	    // Set from the existing renderer
	    set ( renderer->symbol() );
	}
        else
        {
            // Take values from an example instance
            QgsSingleSymbolRenderer exampleRenderer = QgsSingleSymbolRenderer( mVectorLayer->vectorType() );
            set ( exampleRenderer.symbol() );
        }

        if (mVectorLayer && mVectorLayer->vectorType() == QGis::Line)
        {
            btnFillColor->setEnabled(false);
            grpPattern->setEnabled(false);
            mGroupPoint->setEnabled(false);
        }
	
        if (mVectorLayer && mVectorLayer->vectorType() == QGis::Polygon) {
            mGroupPoint->setEnabled(false);

        }
       
    }
    else
    {
        qWarning("Warning, layer is a null pointer in QgsSingleSymbolDialog::QgsSingleSymbolDialog(QgsVectorLayer)");
    }

     //do the signal/slot connections
    QObject::connect(btnOutlineColor, SIGNAL(clicked()), this, SLOT(selectOutlineColor()));
    //QObject::connect(stylebutton, SIGNAL(clicked()), this, SLOT(selectOutlineStyle()));
    QObject::connect(btnFillColor, SIGNAL(clicked()), this, SLOT(selectFillColor()));
    QObject::connect(outlinewidthspinbox, SIGNAL(valueChanged(int)), this, SLOT(resendSettingsChanged()));
    QObject::connect(mLabelEdit, SIGNAL(textChanged(const QString&)), this, SLOT(resendSettingsChanged()));
    QObject::connect(mPointSymbolComboBox, SIGNAL(activated(int)), this, SLOT(resendSettingsChanged()));
    QObject::connect(mPointSizeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(resendSettingsChanged()));

    //connect fill style and line style buttons
    QObject::connect(pbnLineSolid, SIGNAL(clicked()), this, SLOT(resendSettingsChanged()));
    QObject::connect(pbnLineDot, SIGNAL(clicked()), this, SLOT(resendSettingsChanged()));
    QObject::connect(pbnLineDashDot, SIGNAL(clicked()), this, SLOT(resendSettingsChanged()));
    QObject::connect(pbnLineDash, SIGNAL(clicked()), this, SLOT(resendSettingsChanged()));
    QObject::connect(pbnLineDashDotDot, SIGNAL(clicked()), this, SLOT(resendSettingsChanged()));
    QObject::connect(pbnLineNoPen, SIGNAL(clicked()), this, SLOT(resendSettingsChanged()));
    QObject::connect(solid, SIGNAL(clicked()), this, SLOT(resendSettingsChanged()));
    QObject::connect(fdiag, SIGNAL(clicked()), this, SLOT(resendSettingsChanged()));
    QObject::connect(dense4, SIGNAL(clicked()), this, SLOT(resendSettingsChanged()));
    QObject::connect(horizontal, SIGNAL(clicked()), this, SLOT(resendSettingsChanged()));
    QObject::connect(bdiag, SIGNAL(clicked()), this, SLOT(resendSettingsChanged()));
    QObject::connect(diagcross, SIGNAL(clicked()), this, SLOT(resendSettingsChanged()));
    QObject::connect(dense5, SIGNAL(clicked()), this, SLOT(resendSettingsChanged()));
    QObject::connect(vertical, SIGNAL(clicked()), this, SLOT(resendSettingsChanged()));
    QObject::connect(dense1, SIGNAL(clicked()), this, SLOT(resendSettingsChanged()));
    QObject::connect(dense3, SIGNAL(clicked()), this, SLOT(resendSettingsChanged()));
    QObject::connect(dense6, SIGNAL(clicked()), this, SLOT(resendSettingsChanged()));
    QObject::connect(cross, SIGNAL(clicked()), this, SLOT(resendSettingsChanged()));
    QObject::connect(dense2, SIGNAL(clicked()), this, SLOT(resendSettingsChanged()));
    QObject::connect(dense7, SIGNAL(clicked()), this, SLOT(resendSettingsChanged()));
    QObject::connect(nopen, SIGNAL(clicked()), this, SLOT(resendSettingsChanged()));

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
    
    setActiveWindow();
}

void QgsSingleSymbolDialog::selectFillColor()
{
    QColor c = QColorDialog::getColor(btnFillColor->color(), this);

    if ( c.isValid() ) {
        btnFillColor->setColor(c);
        emit settingsChanged();
    }

    setActiveWindow();
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
    sy->setNamedPointSymbol( mMarkers[mPointSymbolComboBox->currentItem()] ) ;
    sy->setPointSize ( mPointSizeSpinBox->value() );
    
    //
    // Apply the line style
    //
    if  (pbnLineNoPen->isOn())
        (sy->setLineStyle(Qt::NoPen));
    else if  (pbnLineDash->isOn())
        (sy->setLineStyle(Qt::DashLine));
    else if  (pbnLineDot->isOn())
        (sy->setLineStyle(Qt::DotLine)) ;
    else if  (pbnLineDashDot->isOn())
        (sy->setLineStyle(Qt::DashDotLine));
    else if  (pbnLineDashDotDot->isOn())
        (sy->setLineStyle(Qt::DashDotDotLine)) ;
    else
        (sy->setLineStyle(Qt::SolidLine)); //default to solid


    //
    // Apply the pattern
    //

    if (solid->isOn())
    {
        sy->setFillStyle(Qt::SolidPattern);
    }
    else if (fdiag->isOn())
    {
        sy->setFillStyle(Qt::FDiagPattern);
    }
    else if (dense4->isOn())
    {
        sy->setFillStyle(Qt::Dense4Pattern);
    }
    else if (horizontal->isOn())
    {
        sy->setFillStyle(Qt::HorPattern);
    }
    else if (dense5->isOn())
    {
        sy->setFillStyle(Qt::Dense5Pattern);
    }
    else if (diagcross->isOn())
    {
        sy->setFillStyle(Qt::DiagCrossPattern);
    }
    else if (dense1->isOn())
    {
        sy->setFillStyle(Qt::Dense1Pattern);
    }
    else if (dense6->isOn())
    {
        sy->setFillStyle(Qt::Dense6Pattern);
    }
    else if (vertical->isOn())
    {
        sy->setFillStyle(Qt::VerPattern);
    }
    else if (dense7->isOn())
    {
        sy->setFillStyle(Qt::Dense7Pattern);
    }
    else if (cross->isOn())
    {
        sy->setFillStyle(Qt::CrossPattern);
    }
    else if (dense2->isOn())
    {
        sy->setFillStyle(Qt::Dense2Pattern);
    }
    else if (bdiag->isOn())
    {
        sy->setFillStyle(Qt::BDiagPattern);
    }
    else if (dense3->isOn())
    {
        sy->setFillStyle(Qt::Dense3Pattern);
    }
    else if (nopen->isOn())
    {
        sy->setFillStyle(Qt::NoBrush);
    }

    //apply the label
    sy->setLabel(mLabelEdit->text());
}

void QgsSingleSymbolDialog::apply()
{
    QgsSymbol* sy = new QgsSymbol(mVectorLayer->vectorType());
    apply(sy);

    QgsSingleSymbolRenderer *renderer = new QgsSingleSymbolRenderer(mVectorLayer->vectorType());
    renderer->addSymbol(sy);
    mVectorLayer->setRenderer(renderer);
    mVectorLayer->refreshLegend();
}

void QgsSingleSymbolDialog::set ( const QgsSymbol *sy ) 
{
	// Set point symbol
    for ( int i = 0; i < mMarkers.size(); i++ ) {
	    if ( mMarkers[i] ==  sy->pointSymbolName() ) {
	        mPointSymbolComboBox->setCurrentItem ( i );
		break;
	    }
	}
	mPointSizeSpinBox->setValue ( sy->pointSize() );
	
	outlinewidthspinbox->setValue(sy->pen().width());

	//set line width 1 as minimum to avoid confusion between line width 0 and no pen line style
	// ... but, drawLine is not correct with width > 0 -> until solved set to 0
	outlinewidthspinbox->setMinValue(0);

    btnFillColor->setColor( sy->brush().color() );

    btnOutlineColor->setColor( sy->pen().color() );

	//stylebutton->setName(QgsSymbologyUtils::penStyle2Char(sy->pen().style()));
	//stylebutton->setPixmap(QgsSymbologyUtils::char2LinePixmap(stylebutton->name()));
	//load the icons stored in QgsSymbologyUtils.cpp (to avoid redundancy)

	QPen myPen = sy->pen();

	switch ( myPen.style() )
	{
	    case Qt::NoPen :
		pbnLineNoPen->setOn(true);
		break;
	    case Qt::DashLine :
		pbnLineDash->setOn(true);
		break;
	    case Qt::DotLine :
		pbnLineDot->setOn(true);
		break;
	    case Qt::DashDotLine :
		pbnLineDashDot->setOn(true);
		break;
	    case Qt::DashDotDotLine :
		pbnLineDashDotDot->setOn(true);
		break;
	    default :
		pbnLineSolid->setOn(true); // default to solid
		break;
	}

	QBrush myBrush = sy->brush();

	switch ( myBrush.style() )
	{
	    case Qt::SolidPattern :
		solid->setOn(true);
		break;
	    case Qt::HorPattern :
		horizontal->setOn(true);
		break;
	    case Qt::VerPattern :
		vertical->setOn(true);
		break;
	    case  Qt::CrossPattern :
		cross->setOn(true);
		break;
	    case Qt::BDiagPattern :
		bdiag->setOn(true);
		break;
	    case Qt::FDiagPattern :
		fdiag->setOn(true);
		break;
	    case Qt::DiagCrossPattern :
		diagcross->setOn(true);
		break;
	    case Qt::Dense1Pattern :
		dense1->setOn(true);
		break;
	    case Qt::Dense2Pattern :
		dense2->setOn(true);
		break;
	    case Qt::Dense3Pattern :
		dense3->setOn(true);
		break;
	    case Qt::Dense4Pattern :
		dense4->setOn(true);
		break;
	    case Qt::Dense5Pattern :
		dense5->setOn(true);
		break;
	    case Qt::Dense6Pattern :
		dense6->setOn(true);
		break;
	    case Qt::Dense7Pattern :
		dense7->setOn(true);
		break;
	    case Qt::NoBrush :
		nopen->setOn(true);
		break;
	    default :
		solid->setOn(true);
		break;
	}
}

void QgsSingleSymbolDialog::setOutlineColor(QColor& c)
{
    btnOutlineColor->setColor(c);
}

void QgsSingleSymbolDialog::setOutlineStyle(Qt::PenStyle pstyle)
{
    // XXX use switch() instead
    if (pstyle==Qt::NoPen)
        (pbnLineNoPen->setOn(true));
    else if (pstyle==Qt::DashLine)
        (pbnLineDash->setOn(true));
    else if (pstyle==Qt::DotLine)
        (pbnLineDot->setOn(true));
    else if (pstyle==Qt::DashDotLine)
        (pbnLineDashDot->setOn(true));
    else if (pstyle==Qt::DashDotDotLine)
        (pbnLineDashDotDot->setOn(true));
    else
        (pbnLineSolid->setOn(true)); //default to solid
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

    // XXX use switch instead
    if (fstyle==Qt::SolidPattern)
        (solid->setOn(true));
    else if (fstyle==Qt::HorPattern)
        (horizontal->setOn(true));
    else if (fstyle==Qt::VerPattern)
        (vertical->setOn(true));
    else if (fstyle==Qt::CrossPattern)
        (cross->setOn(true));
    else if (fstyle==Qt::BDiagPattern)
        (bdiag->setOn(true));
    else if (fstyle==Qt::FDiagPattern)
        (fdiag->setOn(true));
    else if (fstyle==Qt::DiagCrossPattern)
        (diagcross->setOn(true));
    else if (fstyle==Qt::Dense1Pattern)
        (dense1->setOn(true));
    else if (fstyle==Qt::Dense2Pattern)
        (dense2->setOn(true));
    else if (fstyle==Qt::Dense3Pattern)
        (dense3->setOn(true));
    else if (fstyle==Qt::Dense4Pattern)
        (dense4->setOn(true));
    else if (fstyle==Qt::Dense5Pattern)
        (dense5->setOn(true));
    else if (fstyle==Qt::Dense6Pattern)
        (dense6->setOn(true));
    else if (fstyle==Qt::Dense7Pattern)
        (dense7->setOn(true));
    else if (fstyle==Qt::NoBrush)
        (nopen->setOn(true)); //default to no brush
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
    if  (pbnLineNoPen->isOn())
        return Qt::NoPen;
    else if  (pbnLineDash->isOn())
        return Qt::DashLine;
    else if  (pbnLineDot->isOn())
        return Qt::DotLine ;
    else if  (pbnLineDashDot->isOn())
        return Qt::DashDotLine;
    else if  (pbnLineDashDotDot->isOn())
        return Qt::DashDotDotLine ;
    else
        return Qt::SolidLine; //default to solid

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
    if (solid->isOn())
    {
        return Qt::SolidPattern;
    }
    else if (fdiag->isOn())
    {
        return Qt::FDiagPattern;
    }
    else if (dense4->isOn())
    {
        return Qt::Dense4Pattern;
    }
    else if (horizontal->isOn())
    {
        return Qt::HorPattern;
    }
    else if (dense5->isOn())
    {
        return Qt::Dense5Pattern;
    }
    else if (diagcross->isOn())
    {
        return Qt::DiagCrossPattern;
    }
    else if (dense1->isOn())
    {
        return Qt::Dense1Pattern;
    }
    else if (dense6->isOn())
    {
        return Qt::Dense6Pattern;
    }
    else if (vertical->isOn())
    {
        return Qt::VerPattern;
    }
    else if (dense7->isOn())
    {
        return Qt::Dense7Pattern;
    }
    else if (cross->isOn())
    {
        return Qt::CrossPattern;
    }
    else if (dense2->isOn())
    {
        return Qt::Dense2Pattern;
    }
    else if (bdiag->isOn())
    {
        return Qt::BDiagPattern;
    }
    else if (dense3->isOn())
    {
        return Qt::Dense3Pattern;
    }
    //fall back to transparent
    return Qt::NoBrush;

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


