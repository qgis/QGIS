/***************************************************************************
                         qgslabeldialog.cpp  -  render vector labels
                             -------------------
    begin                : August 2004
    copyright            : (C) 2004 by Radim Blazek & Tim Sutton
    email                : blazek@itc.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <iostream>

//#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qpoint.h>
#include "qpixmap.h"
#include <qwidget.h>
#include <qlayout.h>
#include <qtoolbutton.h>
#include <qlabel.h>
#include <qtable.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qfontdatabase.h>
#include <qradiobutton.h> 
#include <qfont.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qcolor.h>
#include <qcolordialog.h>
#include <qfontdatabase.h>
#include <qfontdialog.h>

#include "qgsfield.h"
#include "qgspatterndialog.h"
#include "qgslinestyledialog.h"

#include "qgslabelattributes.h"
#include "qgslabel.h"
#include "qgslabeldialog.h"



#define PIXMAP_WIDTH 200
#define PIXMAP_HEIGHT 20

QgsLabelDialog::QgsLabelDialog ( QgsLabel *label,  QWidget *parent ) : QgsLabelDialogBase (parent)
{
    #ifdef QGISDEBUG
    std::cerr << "QgsLabelDialog::QgsLabelDialog()" << std::endl;
    #endif

    mLabel = label;
    init();
}

void QgsLabelDialog::init ( void )
{
#ifdef QGISDEBUG
  std::cerr << "QgsLabelDialog::init" << std::endl;
#endif
  QgsLabelAttributes * myLabelAttributes = mLabel->layerAttributes();
  //populate a string list with all the field names which will be used to set up the 
  //data bound combos
  std::vector<QgsField> myFieldsVector = mLabel->fields();
  QStringList myFieldStringList;
  myFieldStringList.append ( "" );
  for (unsigned int i = 0; i < myFieldsVector.size(); i++ ) 
  {
    myFieldStringList.append ( myFieldsVector[i].name() );
  }
  //
  //now set all the combos that need field lists using the string list
  //
  cboLabelField->clear();
  cboLabelField->insertStringList(myFieldStringList);
  cboLabelField->setCurrentItem(itemNoForField(mLabel->labelField(QgsLabel::Text),myFieldStringList));


  cboFontField->clear();
  cboFontField->insertStringList(myFieldStringList); 
  cboFontField->setCurrentItem(itemNoForField(mLabel->labelField(QgsLabel::Family),myFieldStringList));

  cboBoldField->clear();
  cboBoldField->insertStringList(myFieldStringList); 
  cboBoldField->setCurrentItem(itemNoForField(mLabel->labelField(QgsLabel::Bold),myFieldStringList));


  cboItalicField->clear();
  cboItalicField->insertStringList(myFieldStringList); 
  cboItalicField->setCurrentItem(itemNoForField(mLabel->labelField(QgsLabel::Italic),myFieldStringList));

  cboUnderlineField->clear();
  cboUnderlineField->insertStringList(myFieldStringList); 
  cboUnderlineField->setCurrentItem(itemNoForField(mLabel->labelField(QgsLabel::Underline),myFieldStringList));

  cboFontSizeField->clear();
  cboFontSizeField->insertStringList(myFieldStringList); 
  cboFontSizeField->setCurrentItem(itemNoForField(mLabel->labelField(QgsLabel::Size),myFieldStringList));

  cboFontTransparencyField->clear();
  cboFontTransparencyField->insertStringList(myFieldStringList); 
  //cboFontTransparencyField->setCurrentItem(itemNoForField(mLabel->labelField(QgsLabel::FontTransparency),myFieldStringList));

  //cboBufferColorField->clear();
  //cboBufferColorField->insertStringList(myFieldStringList); 

  cboBufferSizeField->clear();
  cboBufferSizeField->insertStringList(myFieldStringList); 
  cboBufferSizeField->setCurrentItem(itemNoForField(mLabel->labelField(QgsLabel::BufferSize),myFieldStringList));

  cboBufferTransparencyField->clear();
  cboBufferTransparencyField->insertStringList(myFieldStringList); 
  //cboBufferTransparencyField->setCurrentItem(itemNoForField(mLabel->labelField(QgsLabel::BufferTransparency),myFieldStringList));

  cboXCoordinateField->clear();
  cboXCoordinateField->insertStringList(myFieldStringList); 
  cboXCoordinateField->setCurrentItem(itemNoForField(mLabel->labelField(QgsLabel::XCoordinate),myFieldStringList));

  cboYCoordinateField->clear();
  cboYCoordinateField->insertStringList(myFieldStringList); 
  cboYCoordinateField->setCurrentItem(itemNoForField(mLabel->labelField(QgsLabel::YCoordinate),myFieldStringList));

  cboXOffsetField->clear();
  cboXOffsetField->insertStringList(myFieldStringList); 
  cboXOffsetField->setCurrentItem(itemNoForField(mLabel->labelField(QgsLabel::XOffset),myFieldStringList));

  cboYOffsetField->clear();
  cboYOffsetField->insertStringList(myFieldStringList); 
  cboYOffsetField->setCurrentItem(itemNoForField(mLabel->labelField(QgsLabel::YOffset),myFieldStringList));

  cboAlignmentField->clear();
  cboAlignmentField->insertStringList(myFieldStringList); 
  cboAlignmentField->setCurrentItem(itemNoForField(mLabel->labelField(QgsLabel::Alignment),myFieldStringList));

  cboAngleField->clear();
  cboAngleField->insertStringList(myFieldStringList); 
  cboAngleField->setCurrentItem(itemNoForField(mLabel->labelField(QgsLabel::Angle),myFieldStringList));


  //
  //set the non-databound fields up now
  //
  leDefaultLabel->setText(myLabelAttributes->text());
  mFont.setFamily(myLabelAttributes->family());
  if (myLabelAttributes->sizeIsSet())
  {
    mFont.setPointSize(myLabelAttributes->size());

    int myTypeInt = myLabelAttributes->sizeType();
    if (myTypeInt == QgsLabelAttributes::PointUnits)
    {
      radioFontSizeUnitsPoints->setChecked(true);
    }
    else //assume map units is checked
    { 
      radioFontSizeUnitsMap->setChecked(true);
    }
  }
  else //defaults for when no size has been set
  {
    mFont.setPointSize(myLabelAttributes->size());
    radioFontSizeUnitsPoints->setChecked(true);
  }

  if (myLabelAttributes->boldIsSet())
  {
     mFont.setBold(myLabelAttributes->bold());
  }
  else
  {
     mFont.setBold(false);
  }
  if (myLabelAttributes->italicIsSet())
  {
     mFont.setBold(myLabelAttributes->italic());
  }
  else
  {
     mFont.setItalic(false);
  }
  /*
     myLabelAttributes->setUnderline(mFont.underline());
     myLabelAttributes->setColor(mFontColor); 
     myTypeInt = 0;
     if ( radioOffsetUnitsPoints->isChecked() )
     {
     myTypeInt = QgsLabelAttributes::PointUnits; 
     } 
     else 
     { 
     myTypeInt = QgsLabelAttributes::MapUnits;
     }
     myLabelAttributes->setOffset(spinXOffset->value() ,spinYOffset->value(), myTypeInt);
     myLabelAttributes->setAngle(spinAngle->value()); 

  //the values here may seem a bit counterintuitive - basically everything 
  //is the reverse of the way you think it should be...
  //TODO investigate in QgsLabel why this needs to be the case
  if (radioAboveLeft->isChecked())   myLabelAttributes->setAlignment(Qt::AlignRight | Qt::AlignBottom);
  if (radioBelowLeft->isChecked())   myLabelAttributes->setAlignment(Qt::AlignRight | Qt::AlignTop);
  if (radioAboveRight->isChecked())  myLabelAttributes->setAlignment(Qt::AlignLeft  | Qt::AlignBottom);
  if (radioBelowRight->isChecked())  myLabelAttributes->setAlignment(Qt::AlignLeft  | Qt::AlignTop);
  if (radioLeft->isChecked())        myLabelAttributes->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  if (radioRight->isChecked())       myLabelAttributes->setAlignment(Qt::AlignLeft  | Qt::AlignVCenter);
  if (radioAbove->isChecked())       myLabelAttributes->setAlignment(Qt::AlignBottom| Qt::AlignHCenter); 
  if (radioBelow->isChecked())       myLabelAttributes->setAlignment(Qt::AlignTop   | Qt::AlignHCenter); 
  if (radioOver->isChecked())        myLabelAttributes->setAlignment(Qt::AlignCenter);

  myLabelAttributes->setBufferColor(mBufferColor); 
  myTypeInt = 0;
  if ( radioBufferUnitsPoints->isChecked() )
  {
  myTypeInt = QgsLabelAttributes::PointUnits; 
  } 
  else 
  { 
  myTypeInt = QgsLabelAttributes::MapUnits;
  }
  myLabelAttributes->setBufferSize(spinBufferSize->value(), myTypeInt);
  //TODO - transparency attributes for buffers
  */

}



void QgsLabelDialog::changeFont ( void )
{
    #ifdef QGISDEBUG
    std::cerr << "QgsLabelDialog::changeFont()" << std::endl;
    #endif
    bool resultFlag;
    mFont = QFontDialog::getFont(&resultFlag, mFont, this );
    if ( resultFlag ) 
    {
        // font is set to the font the user selected
    } else 
    {
        // the user cancelled the dialog; font is set to the initial
        // value, in this case Helvetica [Cronyx], 10
    }
    lblSample->setFont(mFont);
}

void QgsLabelDialog::changeFontColor(void)
{
    #ifdef QGISDEBUG
    std::cerr << "QgsLabelDialog::changeFontColor()" << std::endl;
    #endif
    mFontColor = QColorDialog::getColor ( mFontColor );
    lblSample->setPaletteForegroundColor(mFontColor);
}

void QgsLabelDialog::changeBufferColor(void)
{
    #ifdef QGISDEBUG
    std::cerr << "QgsLabelDialog::changeBufferColor()" << std::endl;
    #endif
    mBufferColor = QColorDialog::getColor ( mBufferColor );
    lblSample->setPaletteBackgroundColor(mBufferColor);
}


int QgsLabelDialog::itemNoForField(QString theFieldName, QStringList theFieldList)
{
    int myItemInt=0;
    for ( QStringList::Iterator it = theFieldList.begin(); it != theFieldList.end(); ++it ) 
    {
        if (theFieldName == *it) return myItemInt;
        ++myItemInt;
    }
    //if no matches assume first item in list is blank and return that
    return 0;
}

QgsLabelDialog::~QgsLabelDialog()
{
    #ifdef QGISDEBUG
    std::cerr << "QgsLabelDialog::~QgsLabelDialog()" << std::endl;
    #endif
}

void QgsLabelDialog::apply()
{
    #ifdef QGISDEBUG
    std::cerr << "QgsLabelDialog::apply()" << std::endl;
    #endif

    //set the label props that are NOT bound to a field in the attributes tbl
    //All of these are set in the layerAttributes member of the layer
    QgsLabelAttributes * myLabelAttributes = mLabel->layerAttributes();
    myLabelAttributes->setText(leDefaultLabel->text()); 
    myLabelAttributes->setFamily(mFont.family());
    int myTypeInt = 0;
    if ( radioFontSizeUnitsPoints->isChecked())
    {
       myTypeInt = QgsLabelAttributes::PointUnits; 
    } 
    else //assume map units is checked
    { 
       myTypeInt = QgsLabelAttributes::MapUnits;
    }
    myLabelAttributes->setSize(mFont.pointSize() , myTypeInt);
    myLabelAttributes->setBold(mFont.bold());
    myLabelAttributes->setItalic(mFont.italic());
    myLabelAttributes->setUnderline(mFont.underline());
    myLabelAttributes->setColor(mFontColor); 
    myTypeInt = 0;
    if ( radioOffsetUnitsPoints->isChecked() )
    {
       myTypeInt = QgsLabelAttributes::PointUnits; 
    } 
    else 
    { 
       myTypeInt = QgsLabelAttributes::MapUnits;
    }
    myLabelAttributes->setOffset(spinXOffset->value() ,spinYOffset->value(), myTypeInt);
    myLabelAttributes->setAngle(spinAngle->value()); 

    //the values here may seem a bit counterintuitive - basically everything 
    //is the reverse of the way you think it should be...
    //TODO investigate in QgsLabel why this needs to be the case
    if (radioAboveLeft->isChecked())   myLabelAttributes->setAlignment(Qt::AlignRight | Qt::AlignBottom);
    if (radioBelowLeft->isChecked())   myLabelAttributes->setAlignment(Qt::AlignRight | Qt::AlignTop);
    if (radioAboveRight->isChecked())  myLabelAttributes->setAlignment(Qt::AlignLeft  | Qt::AlignBottom);
    if (radioBelowRight->isChecked())  myLabelAttributes->setAlignment(Qt::AlignLeft  | Qt::AlignTop);
    if (radioLeft->isChecked())        myLabelAttributes->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    if (radioRight->isChecked())       myLabelAttributes->setAlignment(Qt::AlignLeft  | Qt::AlignVCenter);
    if (radioAbove->isChecked())       myLabelAttributes->setAlignment(Qt::AlignBottom| Qt::AlignHCenter); 
    if (radioBelow->isChecked())       myLabelAttributes->setAlignment(Qt::AlignTop   | Qt::AlignHCenter); 
    if (radioOver->isChecked())        myLabelAttributes->setAlignment(Qt::AlignCenter);
      
    myLabelAttributes->setBufferColor(mBufferColor); 
    myTypeInt = 0;
    if ( radioBufferUnitsPoints->isChecked() )
    {
       myTypeInt = QgsLabelAttributes::PointUnits; 
    } 
    else 
    { 
       myTypeInt = QgsLabelAttributes::MapUnits;
    }
    myLabelAttributes->setBufferSize(spinBufferSize->value(), myTypeInt);
    //TODO - transparency attributes for buffers
    
    //set the label props that are data bound to a field in the attributes tbl
    mLabel->setLabelField( QgsLabel::Text,  cboLabelField->currentText() );
    mLabel->setLabelField( QgsLabel::Family, cboFontField->currentText() );
    mLabel->setLabelField( QgsLabel::Bold,  cboBoldField->currentText() );
    mLabel->setLabelField( QgsLabel::Italic,  cboItalicField->currentText() );
    mLabel->setLabelField( QgsLabel::Underline,  cboUnderlineField->currentText() );
    mLabel->setLabelField( QgsLabel::Size,  cboFontSizeField->currentText() );
    mLabel->setLabelField( QgsLabel::BufferSize,  cboBufferSizeField->currentText() );
    //mLabel->setLabelField( QgsLabel::BufferTransparency,  cboBufferTransparencyField->currentText() );
    mLabel->setLabelField( QgsLabel::XCoordinate,  cboXCoordinateField->currentText() );
    mLabel->setLabelField( QgsLabel::YCoordinate,  cboYCoordinateField->currentText() );
    mLabel->setLabelField( QgsLabel::XOffset,  cboXOffsetField->currentText() );
    mLabel->setLabelField( QgsLabel::YOffset,  cboYOffsetField->currentText() );
    mLabel->setLabelField( QgsLabel::Alignment,  cboAlignmentField->currentText() );
    mLabel->setLabelField( QgsLabel::Angle,  cboAngleField->currentText() );

}

