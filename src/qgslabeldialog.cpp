/***************************************************************************
                         qgslabeldialog.cpp  -  render vector labels
                             -------------------
    begin                : August 2004
    copyright            : (C) 2004 by Radim Blazek
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
#include "qpushbutton.h"
#include "qspinbox.h"
#include "qcolor.h"
#include "qcolordialog.h"
#include <qfontdatabase.h>

#include "qgsfield.h"
#include "qgspatterndialog.h"
#include "qgslinestyledialog.h"
#include "qgsvectorlayerproperties.h"

#include "qgslabelattributes.h"
#include "qgslabel.h"
#include "qgslabeldialog.h"

#define PIXMAP_WIDTH 200
#define PIXMAP_HEIGHT 20

QgsLabelDialog::QgsLabelDialog ( QgsLabel *label,  QWidget *parent )
{
    #ifdef QGISDEBUG
    std::cerr << "QgsLabelDialog::QgsLabelDialog()" << std::endl;
    #endif

    mLabel = label;
    QgsLabelAttributes *att = mLabel->layerAttributes();

    mTable = new QTable ( 13, 3, parent );
    mTable->setSizePolicy ( QSizePolicy ( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
    QVBoxLayout *layout = new QVBoxLayout( parent );
    layout->addWidget( mTable );

    mTable->setLeftMargin(0); // hide row labels

    mTable->horizontalHeader()->setLabel( 0, "Attribute" );
    mTable->horizontalHeader()->setLabel( 1, "Default" );
    mTable->horizontalHeader()->setLabel( 2, "Field" );

    mTable->setColumnWidth ( 0, 200 ); 

    /* Available fields */
    std::vector<QgsField> fields = label->fields();
    QStringList fieldsList;
    fieldsList.append ( "" );
    for ( int i = 0; i < fields.size(); i++ ) {
	fieldsList.append ( fields[i].name() );
    }

    QTableItem *ti;
    QComboTableItem *cti;
    QCheckTableItem *chti;
    QString str;

    /* Text */
    ti = new QTableItem( mTable, QTableItem::Never, "Text" );
    mTable->setItem ( Text, 0, ti );
    
    ti = new QTableItem( mTable, QTableItem::Always, "" );
    mTable->setItem ( Text, 1, ti );

    /* Font family */
    ti = new QTableItem( mTable, QTableItem::Never, "Font family" );
    mTable->setItem ( Family, 0, ti );

    QFontDatabase fontDb;
    cti = new QComboTableItem ( mTable, fontDb.families() );
    mTable->setItem ( Family, 1, cti );

    /* Font size */
    QStringList sizeTypes;
    sizeTypes.append ( "Size in points" );
    sizeTypes.append ( "Size in map units" );
    cti = new QComboTableItem ( mTable, sizeTypes );
    mTable->setItem ( Size, 0, cti );

    ti = new QTableItem( mTable, QTableItem::Always, "" );
    mTable->setItem ( Size, 1, ti );

    /* Font bold */
    ti = new QTableItem( mTable, QTableItem::Never, "Bold" );
    mTable->setItem ( Bold, 0, ti );

    chti = new QCheckTableItem ( mTable, "" );
    mTable->setItem ( Bold, 1, chti );

    /* Italic */
    ti = new QTableItem( mTable, QTableItem::Never, "Italic" );
    mTable->setItem ( Italic, 0, ti );

    chti = new QCheckTableItem ( mTable, "" );
    mTable->setItem ( Italic, 1, chti );

    /* Underline */
    ti = new QTableItem( mTable, QTableItem::Never, "Underline" );
    mTable->setItem ( Underline, 0, ti );

    chti = new QCheckTableItem ( mTable, "" );
    mTable->setItem ( Underline, 1, chti );

    /* Color */
    ti = new QTableItem( mTable, QTableItem::Never, "Color" );
    mTable->setItem ( Color, 0, ti );

    ti = new QTableItem( mTable, QTableItem::Never, "" );
    mTable->setItem ( Color, 1, ti );

    /* X Coordinate */
    ti = new QTableItem( mTable, QTableItem::Never, "X coordinate" );
    mTable->setItem ( XCoordinate, 0, ti );

    ti = new QTableItem( mTable, QTableItem::Never, "" );
    mTable->setItem ( XCoordinate, 1, ti );

    /* Y Coordinate */
    ti = new QTableItem( mTable, QTableItem::Never, "Y coordinate" );
    mTable->setItem ( YCoordinate, 0, ti );

    ti = new QTableItem( mTable, QTableItem::Never, "" );
    mTable->setItem ( YCoordinate, 1, ti );

    /* X offset */
    QStringList xOffsetTypes;
    xOffsetTypes.append ( "X offset in points" );
    xOffsetTypes.append ( "X offset in map units" );
    cti = new QComboTableItem ( mTable, xOffsetTypes );
    mTable->setItem ( XOffset, 0, cti );

    ti = new QTableItem( mTable, QTableItem::Always, "" );
    mTable->setItem ( XOffset, 1, ti );

    /* Y offset */
    ti = new QTableItem( mTable, QTableItem::Never, "Y offset" );
    mTable->setItem ( YOffset, 0, ti );

    ti = new QTableItem( mTable, QTableItem::Always, "" );
    mTable->setItem ( YOffset, 1, ti );

    /* Angle */
    ti = new QTableItem( mTable, QTableItem::Never, "Angle in degrees" );
    mTable->setItem ( Angle, 0, ti );
    
    ti = new QTableItem( mTable, QTableItem::Always, "" );
    mTable->setItem ( Angle, 1, ti );

    /* Alignment */
    ti = new QTableItem( mTable, QTableItem::Never, "Alignment" );
    mTable->setItem ( Alignment, 0, ti );
    
    QStringList alignmentList;
    alignmentList.append ( "center" );
    alignmentList.append ( "left" );
    alignmentList.append ( "right" );
    alignmentList.append ( "top" );
    alignmentList.append ( "bottom" );
    cti = new QComboTableItem ( mTable, alignmentList );
    mTable->setItem ( Alignment, 1, cti );

    /* Add fields */
    for ( int i = 0; i < 13; i++ ) {
	cti = new QComboTableItem ( mTable, fieldsList );
	mTable->setItem ( i, 2, cti );
    }

    // Reset to vector values
    reset();

    /* Connect */
    connect( mTable, SIGNAL( clicked(int, int, int, const QPoint &)), 
	     this, SLOT( tableClicked (int, int) ));
}

void QgsLabelDialog::reset ( void )
{
    #ifdef QGISDEBUG
    std::cerr << "QgsLabelDialog::reset" << std::endl;
    #endif

    QString str;
    QgsLabelAttributes *att = mLabel->layerAttributes();
    QTableItem *item;
    QComboTableItem *combo;
    QCheckTableItem *check;
    int type;

    /* Text */
    mTable->item(Text,1)->setText( att->text() ); 
    
    mTable->setText(Text,1,att->text()); 
    mTable->updateCell(Text,1);

    /* Font family */
    combo = dynamic_cast < QComboTableItem * > ( mTable->item ( Family, 1 ) );
    combo->setCurrentItem ( att->family() ); 
    mTable->updateCell(Family,1);

    /* Font size */
    combo = dynamic_cast < QComboTableItem * > ( mTable->item ( Size, 0 ) );
    if ( att->sizeType() == QgsLabelAttributes::MapUnits ) {
	combo->setCurrentItem(1);
    } else {
	combo->setCurrentItem(0);
    }
    str.sprintf ( "%.0f", att->size() );
    mTable->setText( Size, 1, str ); 
    //mTable->updateCell(Size,0);
    mTable->updateCell(Size,1);

    /* Font bold */
    check = dynamic_cast < QCheckTableItem *> ( mTable->item ( Bold, 1 ) ) ;
    check->setChecked ( att->bold() );
    mTable->updateCell(Bold,1);

    /* Italic */
    check = dynamic_cast < QCheckTableItem *> ( mTable->item ( Italic, 1 ) ) ;
    check->setChecked ( att->italic() );

    /* Underline */
    check = dynamic_cast < QCheckTableItem *> ( mTable->item ( Underline, 1 ) ) ;
    check->setChecked ( att->underline() );

    /* Color */
    mColor = att->color();
    QPixmap pm ( PIXMAP_WIDTH, PIXMAP_HEIGHT );
    pm.fill ( mColor );
    mTable->item(Color,1)->setPixmap ( pm ); 

    /* X offset */
    combo = dynamic_cast < QComboTableItem * > ( mTable->item ( XOffset, 0 ) );
    if ( att->offsetType() == QgsLabelAttributes::MapUnits ) {
	combo->setCurrentItem(1);
    } else {
	combo->setCurrentItem(0);
    }
    str.sprintf ( "%.0f", att->xOffset() );
    mTable->setText(XOffset,1,str); 
    mTable->updateCell(XOffset,1);

    /* Y offset */
    str.sprintf ( "%.0f", att->yOffset() );
    mTable->setText(YOffset,1,str); 
    mTable->updateCell(YOffset,1);

    /* Angle */
    str.sprintf ( "%.0f", att->angle() );
    mTable->setText(Angle,1,str); 
    mTable->updateCell(Angle,1);

    /* Alignment */
    combo = dynamic_cast < QComboTableItem * > ( mTable->item ( Alignment, 1 ) );
    combo->setCurrentItem ( QgsLabelAttributes::alignmentName(att->alignment()) );

    (dynamic_cast <QComboTableItem*>(mTable->item(Text,2)))->setCurrentItem(mLabel->labelField(QgsLabel::Text));
    (dynamic_cast <QComboTableItem*>(mTable->item(Family,2)))->setCurrentItem(mLabel->labelField(QgsLabel::Family));
    (dynamic_cast <QComboTableItem*>(mTable->item(Size,2)))->setCurrentItem(mLabel->labelField(QgsLabel::Size));
    (dynamic_cast <QComboTableItem*>(mTable->item(Bold,2)))->setCurrentItem(mLabel->labelField(QgsLabel::Bold));
    (dynamic_cast <QComboTableItem*>(mTable->item(Italic,2)))->setCurrentItem(mLabel->labelField(QgsLabel::Italic));
    (dynamic_cast <QComboTableItem*>(mTable->item(Underline,2)))->setCurrentItem(mLabel->labelField(QgsLabel::Underline));
    (dynamic_cast <QComboTableItem*>(mTable->item(Color,2)))->setCurrentItem(mLabel->labelField(QgsLabel::Color));
    (dynamic_cast <QComboTableItem*>(mTable->item(XCoordinate,2)))->setCurrentItem(mLabel->labelField(QgsLabel::XCoordinate));
    (dynamic_cast <QComboTableItem*>(mTable->item(YCoordinate,2)))->setCurrentItem(mLabel->labelField(QgsLabel::YCoordinate));
    (dynamic_cast <QComboTableItem*>(mTable->item(XOffset,2)))->setCurrentItem(mLabel->labelField(QgsLabel::XOffset));
    (dynamic_cast <QComboTableItem*>(mTable->item(YOffset,2)))->setCurrentItem(mLabel->labelField(QgsLabel::YOffset));
    (dynamic_cast <QComboTableItem*>(mTable->item(Angle,2)))->setCurrentItem(mLabel->labelField(QgsLabel::Angle));
    (dynamic_cast <QComboTableItem*>(mTable->item(Alignment,2)))->setCurrentItem(mLabel->labelField(QgsLabel::Alignment));
}

void QgsLabelDialog::tableClicked ( int row, int col )
{
    #ifdef QGISDEBUG
    std::cerr << "QgsLabelDialog::tableClicked()" << std::endl;
    #endif

    if ( row == Color && col == 1 ) changeColor();

}

void QgsLabelDialog::changeColor(void)
{
    #ifdef QGISDEBUG
    std::cerr << "QgsLabelDialog::changeColor()" << std::endl;
    #endif

    mColor = QColorDialog::getColor ( mColor );
    QPixmap pm ( PIXMAP_WIDTH, PIXMAP_HEIGHT );
    pm.fill( mColor );
    mTable->item(Color,1)->setPixmap(pm);
    mTable->repaint();
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

    QgsLabelAttributes *att = mLabel->layerAttributes();
    QTableItem *item;
    QComboTableItem *combo;
    QCheckTableItem *check;
    int type;

    /* Text */
    item = mTable->item ( Text, 1 );
    att->setText ( item->text() );

    /* Font family */
    combo = dynamic_cast < QComboTableItem * > ( mTable->item ( Family, 1 ) );
    att->setFamily ( combo->currentText() );
    
    /* Font size */
    combo = dynamic_cast < QComboTableItem * > ( mTable->item ( Size, 0 ) );
    item = mTable->item ( Size, 1 );
    if ( combo->currentItem() == 0 ) {
	type = QgsLabelAttributes::PointUnits; 
    } else { 
	type = QgsLabelAttributes::MapUnits;
    }
    att->setSize ( item->text().toDouble(), type );

    /* Font bold */
    check = dynamic_cast < QCheckTableItem *> ( mTable->item ( Bold, 1 ) ) ;
    att->setBold ( check->isChecked() );

    /* Italic */
    check = dynamic_cast < QCheckTableItem *> ( mTable->item ( Italic, 1 ) ) ;
    att->setItalic ( check->isChecked() );

    /* Underline */
    check = dynamic_cast < QCheckTableItem *> ( mTable->item ( Underline, 1 ) ) ;
    att->setUnderline ( check->isChecked() );

    /* Color */
    att->setColor ( mColor ); 


    /* X, Y offset */
    combo = dynamic_cast < QComboTableItem * > ( mTable->item ( XOffset, 0 ) );
    if ( combo->currentItem() == 0 ) {
	type = QgsLabelAttributes::PointUnits; 
    } else { 
	type = QgsLabelAttributes::MapUnits;
    }
    
    att->setOffset ( mTable->item(XOffset,1)->text().toDouble(),
	             mTable->item(YOffset,1)->text().toDouble(), type );

    /* Angle */
    att->setAngle ( mTable->item(Angle,1)->text().toDouble() );

    /* Alignment */
    combo = dynamic_cast < QComboTableItem * > ( mTable->item ( Alignment, 1 ) );
    att->setAlignment ( QgsLabelAttributes::alignmentCode(combo->currentText()) );

    /* Fields */
    mLabel->setLabelField( QgsLabel::Text, ( dynamic_cast <QComboTableItem*>(mTable->item(Text,2)))->currentText() );
    mLabel->setLabelField( QgsLabel::Family, ( dynamic_cast <QComboTableItem*>(mTable->item(Family,2)) )->currentText() );
    mLabel->setLabelField( QgsLabel::Size, ( dynamic_cast <QComboTableItem*>(mTable->item(Size,2)) )->currentText() );
    mLabel->setLabelField( QgsLabel::Bold, ( dynamic_cast <QComboTableItem*>(mTable->item(Bold,2)) )->currentText() );
    mLabel->setLabelField( QgsLabel::Italic, ( dynamic_cast <QComboTableItem*>(mTable->item(Italic,2)) )->currentText() );
    mLabel->setLabelField( QgsLabel::Underline, ( dynamic_cast <QComboTableItem*>(mTable->item(Underline,2)) )->currentText() );
    mLabel->setLabelField( QgsLabel::Color, ( dynamic_cast <QComboTableItem*>(mTable->item(Color,2)) )->currentText() );
    mLabel->setLabelField( QgsLabel::XCoordinate, ( dynamic_cast <QComboTableItem*>(mTable->item(XCoordinate,2)) )->currentText() );
    mLabel->setLabelField( QgsLabel::YCoordinate, ( dynamic_cast <QComboTableItem*>(mTable->item(YCoordinate,2)) )->currentText() );
    mLabel->setLabelField( QgsLabel::XOffset, ( dynamic_cast <QComboTableItem*>(mTable->item(XOffset,2)) )->currentText() );
    mLabel->setLabelField( QgsLabel::YOffset, ( dynamic_cast <QComboTableItem*>(mTable->item(YOffset,2)) )->currentText() );
    mLabel->setLabelField( QgsLabel::Angle, ( dynamic_cast <QComboTableItem*>(mTable->item(Angle,2)) )->currentText() );
    mLabel->setLabelField( QgsLabel::Alignment, ( dynamic_cast <QComboTableItem*>(mTable->item(Alignment,2)) )->currentText() );
    
}

