/***************************************************************************
                           qgscomposerscalebar.cpp
                             -------------------
    begin                : March 2005
    copyright            : (C) 2005 by Radim Blazek
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

#include "qgscomposerscalebar.h"
#include "qgscomposermap.h"
#include "qgsproject.h"

#include <QFontDialog>
#include <QPainter>
#include <QGraphicsScene>

#include <cmath>
#include <iostream>

//can we/should we remove the x and y parameters?
QgsComposerScalebar::QgsComposerScalebar ( QgsComposition *composition, int id, 
	                                            int x, int y )
    : QWidget(composition),
    QAbstractGraphicsShapeItem(0),
    mComposition(composition),
    mMap(0),
    mBrush(QColor(150,150,150))
{
  setupUi(this);

  std::cout << "QgsComposerScalebar::QgsComposerScalebar()" << std::endl;
  mId = id;
  mSelected = false;

  mMapCanvas = mComposition->mapCanvas();

  QAbstractGraphicsShapeItem::setPos(x, y);

  init();

  // Set map to the first available if any
  std::vector < QgsComposerMap * >maps = mComposition->maps();
  if (maps.size() > 0)
    {
      mMap = maps[0]->id();
    }
  // Set default according to the map
  QgsComposerMap *map = mComposition->map(mMap);
  if (map)
    {
      mMapUnitsPerUnit = 1.;
      mUnitLabel = "m";

      // make one segment cca 1/10 of map width and it will be 1xxx, 2xxx or 5xxx
      double mapwidth = 1. * map->QGraphicsRectItem::rect().width() / map->scale();

      mSegmentLength = mapwidth / 10;

      int powerOf10 = int (pow(10.0, int (log(mSegmentLength) / log(10.0)))); // from scalebar plugin

      int isize = (int) ceil(mSegmentLength / powerOf10);

      if (isize == 3)
        isize = 2;
      else if (isize == 4)
        isize = 5;
      else if (isize > 5 && isize < 8)
        isize = 5;
      else if (isize > 7)
        isize = 10;

      mSegmentLength = isize * powerOf10;

      // the scale bar will take cca 1/4 of the map width
      // But always have at least one segment.
      mNumSegments = std::max(1, (int) (mapwidth / 4 / mSegmentLength));

      int segsize = (int) (mSegmentLength * map->scale());

      int fontsize = segsize / 3;

      if(fontsize < 6)
      {
        fontsize = 6;
      }

      mFont.setPointSize(fontsize);
  } else
    {
      mFont.setPointSize(6);
      mMapUnitsPerUnit = 1.;
      mUnitLabel = "m";
      mSegmentLength = 1000.;
      mNumSegments = 5;
    }

  // Calc size
  recalculate();

  // Add to scene
  mComposition->canvas()->addItem(this);

  QAbstractGraphicsShapeItem::show();
  QAbstractGraphicsShapeItem::update();

  writeSettings();
}

QgsComposerScalebar::QgsComposerScalebar ( QgsComposition *composition, int id ) 
    : QAbstractGraphicsShapeItem(0),
    mComposition(composition),
    mMap(0),
    mBrush(QColor(150,150,150))
{
  std::cout << "QgsComposerScalebar::QgsComposerScalebar()" << std::endl;

  setupUi(this);

  mId = id;
  mSelected = false;

  mMapCanvas = mComposition->mapCanvas();

  init();

  readSettings();

  // Calc size
  recalculate();

  // Add to scene
  mComposition->canvas()->addItem(this);

  QAbstractGraphicsShapeItem::show();
  QAbstractGraphicsShapeItem::update();
}

void QgsComposerScalebar::init(void)
{
  mUnitLabel = "m";

  // Rectangle
  QAbstractGraphicsShapeItem::setZValue(50);
//    setActive(true);

  // Default value (map units?) for the scalebar border
  mPen.setWidthF(.5);

  // Plot style
  setPlotStyle(QgsComposition::Preview);

  connect(mComposition, SIGNAL(mapChanged(int)), this, SLOT(mapChanged(int)));
}

QgsComposerScalebar::~QgsComposerScalebar()
{
  std::cerr << "QgsComposerScalebar::~QgsComposerScalebar()" << std::endl;
  QGraphicsItem::hide();
}

QRectF QgsComposerScalebar::render(QPainter * p)
{
  std::cout << "QgsComposerScalebar::render p = " << p << std::endl;

  // Painter can be 0, create dummy to avoid many if below
  QPainter *painter;
  QPixmap *pixmap=NULL;
  if (p)
  {
      painter = p;
  }else
  {
    pixmap = new QPixmap(1, 1);
    painter = new QPainter(pixmap);
  }

  std::cout << "mComposition->scale() = " << mComposition->scale() << std::endl;

  QgsComposerMap *map = mComposition->map(mMap); //Get the topmost map from the composition

  double segwidth;
  if(map)
  {
    segwidth = (mSegmentLength * map->scale());  //width of one segment
  }
  else //if there is no map, make up a segment width
  {
    segwidth = mSegmentLength/100;
  }
  double width = (segwidth * mNumSegments); //width of whole scalebar
  double barLx = -width / 2; //x offset to the left

  double barHeight = (25.4 * mComposition->scale() * mFont.pointSize() / 72);

  double tickSize = barHeight * 1.5; //ticks go from the base of the bar to 1/2 the bar's height above it

  // Draw background rectangle
  painter->setPen(QPen(QColor(255, 255, 255), 0));
  painter->setBrush(QBrush(QColor(255, 255, 255), Qt::SolidPattern));
  painter->drawRect(QRectF(barLx, -barHeight, width, barHeight));

  // set the painter to have grey background and black border
  painter->setPen(QPen(QColor(0, 0, 0), mPen.widthF()));
  painter->setBrush(QBrush(QColor(127, 127, 127)));//default to solid brush

  // fill every other segment with grey
  for (int i = 0; i < mNumSegments; i += 2)
  {
    painter->drawRect(QRectF(barLx + ((double)i * segwidth), -barHeight, segwidth, barHeight));
  }

  // draw a box around the all of the segments
  painter->setBrush(Qt::NoBrush);
  painter->drawRect(QRectF(barLx, -barHeight, width, barHeight));

  // set up the pen to draw the tick marks
  painter->setPen(QPen(QColor(0, 0, 0), mPen.widthF()));

  // draw the tick marks
  for (int i = 0; i <= mNumSegments; i++)
  {
    painter->drawLine(QLineF(barLx + ((double)i * segwidth), 0, barLx + (i * segwidth), -tickSize));
  }


  // labels

  // Font size in canvas units
  float size = 25.4 * mComposition->scale() * mFont.pointSizeFloat() / 72;


  // Create QFontMetrics so we can correctly place the text
  QFont font(mFont);
  font.setPointSizeFloat(size);
  QFontMetrics metrics(font);

  if (plotStyle() == QgsComposition::Postscript)
  {
    font.setPointSizeF(metrics.ascent() * 72.0 / mComposition->resolution());
std::cout << "scalebar using PS font size!" << std::endl;
  }

  painter->setFont(font);

  // Not sure about Style Strategy, QFont::PreferMatch?
  font.setStyleStrategy((QFont::StyleStrategy) (QFont::PreferOutline | QFont::PreferAntialias));

  double offset = .5 * tickSize; //vertical offset above the top of the tick marks
  double textRightOverhang=0;//amount the label text hangs over the right edge of the scalebar - used for the bounding box

  for (int i = 0; i <= mNumSegments; i++)
  {
    int lab = (int) ((double)i * mSegmentLength / mMapUnitsPerUnit);

    QString txt = QString::number(lab);
    double shift = (double)metrics.width(txt) / 2;

    if (i == mNumSegments) //on the last label, append the appropriate unit symbol
    {
      txt.append(" " + mUnitLabel);
      textRightOverhang = (double)metrics.width(txt) - shift;
    }

    double x = barLx + (i * segwidth) - shift; //figure out the bottom left corner and draw the text
    double y = -tickSize - offset - metrics.descent();
    painter->drawText(QPointF(x, y), txt);

  }//end of label drawing

  double totalHeight = tickSize + offset + metrics.height();

  if (!p)
    {
      delete painter;
      delete pixmap;
    }

//Add the 1/2 the pen width to get the full bounding box
return QRectF(barLx - (mPen.widthF()/2), -totalHeight, width + textRightOverhang, totalHeight + (mPen.widthF()/2));
}

void QgsComposerScalebar::paint(QPainter * painter, const QStyleOptionGraphicsItem * itemStyle, QWidget * pWidget)
{
  std::cout << "draw mPlotStyle = " << plotStyle() << std::endl;

  mBoundingRect = render(painter);

  // Show selected / Highlight
  if (mSelected && plotStyle() == QgsComposition::Preview)
    {
      painter->setPen(mComposition->selectionPen());
      painter->setBrush(mComposition->selectionBrush());

      double s = mComposition->selectionBoxSize();
      QRectF r = boundingRect();

      painter->drawRect(QRectF(r.x(), r.y(), s, s));
      painter->drawRect(QRectF(r.x() + r.width() - s, r.y(), s, s));
      painter->drawRect(QRectF(r.x() + r.width() - s, r.y() + r.height() - s, s, s));
      painter->drawRect(QRectF(r.x(), r.y() + r.height() - s, s, s));
    }
}

/*
void QgsComposerScalebar::drawShape ( QPainter & painter )
{
    paint ( painter );
}
*/
void QgsComposerScalebar::on_mFontButton_clicked(void)
{
  bool result;

  mFont = QFontDialog::getFont(&result, mFont, this);

  if (result)
    {
      recalculate();
      QAbstractGraphicsShapeItem::update();
      QAbstractGraphicsShapeItem::scene()->update();
      writeSettings();
    }
}

void QgsComposerScalebar::on_mUnitLabelLineEdit_returnPressed()
{
  mUnitLabel = mUnitLabelLineEdit->text();
  recalculate();
  QAbstractGraphicsShapeItem::update();
  QAbstractGraphicsShapeItem::scene()->update();
  writeSettings();
}

void QgsComposerScalebar::on_mMapComboBox_activated(int i)
{
  mMap = mMaps[i];
  recalculate();
  QAbstractGraphicsShapeItem::update();
  QAbstractGraphicsShapeItem::scene()->update();
  writeSettings();
}

void QgsComposerScalebar::mapChanged(int id)
{
  if (id != mMap)
    return;
  recalculate();
  QAbstractGraphicsShapeItem::update();
  QAbstractGraphicsShapeItem::scene()->update();
}

void QgsComposerScalebar::sizeChanged()
{
  mSegmentLength = mSegmentLengthLineEdit->text().toDouble();
  mNumSegments = mNumSegmentsLineEdit->text().toInt();
  mPen.setWidthF(mLineWidthSpinBox->value());
  mMapUnitsPerUnit = mMapUnitsPerUnitLineEdit->text().toInt();
  recalculate();
  QAbstractGraphicsShapeItem::update();
  QAbstractGraphicsShapeItem::scene()->update();
  writeSettings();
}

void QgsComposerScalebar::on_mLineWidthSpinBox_valueChanged()
{
  sizeChanged();
}

void QgsComposerScalebar::on_mMapUnitsPerUnitLineEdit_returnPressed()
{
  sizeChanged();
}

void QgsComposerScalebar::on_mNumSegmentsLineEdit_returnPressed()
{
  sizeChanged();
}

void QgsComposerScalebar::on_mSegmentLengthLineEdit_returnPressed()
{
  sizeChanged();
}
/*
void QgsComposerScalebar::moveBy(double x, double y)
{
  std::cout << "QgsComposerScalebar::move" << std::endl;
  QGraphicsItem::moveBy(x, y);

  recalculate();
  //writeSettings(); // not necessary called by composition
}*/

void QgsComposerScalebar::recalculate(void)
{
  std::cout << "QgsComposerScalebar::recalculate" << std::endl;

  // !!! prepareGeometryChange() MUST BE called before the value returned by areaPoints() changes
  //Is this still true after the port to GraphicsView?
  QAbstractGraphicsShapeItem::prepareGeometryChange();

  mBoundingRect = render(0);

  QGraphicsItem::update();
}

QRectF QgsComposerScalebar::boundingRect(void) const
{
  std::cout << "QgsComposerScalebar::boundingRect" << std::endl;
  return mBoundingRect;
}

QPolygonF QgsComposerScalebar::areaPoints(void) const
{
  std::cout << "QgsComposerScalebar::areaPoints" << std::endl;

  QRectF r = boundingRect();
  QPolygonF pa;
  pa << QPointF(r.x(), r.y());
  pa << QPointF(r.x() + r.width(), r.y());
  pa << QPointF(r.x() + r.width(), r.y() + r.height());
  pa << QPointF(r.x(), r.y() + r.height());
  return pa;
}

void QgsComposerScalebar::setOptions(void)
{
  mSegmentLengthLineEdit->setText(QString::number(mSegmentLength));
  mNumSegmentsLineEdit->setText(QString::number(mNumSegments));
  mUnitLabelLineEdit->setText(mUnitLabel);
  mMapUnitsPerUnitLineEdit->setText(QString::number(mMapUnitsPerUnit));

  mLineWidthSpinBox->setValue(mPen.widthF());

  // Maps
  mMapComboBox->clear();
  std::vector < QgsComposerMap * >maps = mComposition->maps();

  mMaps.clear();

  bool found = false;
  mMapComboBox->insertItem("", 0);
  mMaps.push_back(0);
  for (int i = 0; i < (int)maps.size(); i++)
    {
      mMapComboBox->insertItem(maps[i]->name(), i + 1);
      mMaps.push_back(maps[i]->id());

      if (maps[i]->id() == mMap)
        {
          found = true;
          mMapComboBox->setCurrentItem(i + 1);
        }
    }

  if (!found)
    {
      mMap = 0;
      mMapComboBox->setCurrentItem(0);
    }
}

void QgsComposerScalebar::setSelected(bool s)
{
  mSelected = s;
  QAbstractGraphicsShapeItem::update(); // show highlight
}

bool QgsComposerScalebar::selected(void)
{
  return mSelected;
}

QWidget *QgsComposerScalebar::options(void)
{
  setOptions();
  return (dynamic_cast < QWidget * >(this));
}

bool QgsComposerScalebar::writeSettings(void)
{
  std::cout << "QgsComposerScalebar::writeSettings" << std::endl;
  QString path;
  path.sprintf("/composition_%d/scalebar_%d/", mComposition->id(), mId);

  QgsProject::instance()->writeEntry("Compositions", path + "x", mComposition->toMM((int) QAbstractGraphicsShapeItem::x()));
  QgsProject::instance()->writeEntry("Compositions", path + "y", mComposition->toMM((int) QAbstractGraphicsShapeItem::y()));

  QgsProject::instance()->writeEntry("Compositions", path + "map", mMap);

  QgsProject::instance()->writeEntry("Compositions", path + "unit/label", mUnitLabel);
  QgsProject::instance()->writeEntry("Compositions", path + "unit/mapunits", mMapUnitsPerUnit);

  QgsProject::instance()->writeEntry("Compositions", path + "segmentsize", mSegmentLength);
  QgsProject::instance()->writeEntry("Compositions", path + "numsegments", mNumSegments);

  QgsProject::instance()->writeEntry("Compositions", path + "font/size", mFont.pointSize());
  QgsProject::instance()->writeEntry("Compositions", path + "font/family", mFont.family());
  QgsProject::instance()->writeEntry("Compositions", path + "font/weight", mFont.weight());
  QgsProject::instance()->writeEntry("Compositions", path + "font/underline", mFont.underline());
  QgsProject::instance()->writeEntry("Compositions", path + "font/strikeout", mFont.strikeOut());

  QgsProject::instance()->writeEntry("Compositions", path + "pen/width", (double) mPen.widthF());

  return true;
}

bool QgsComposerScalebar::readSettings(void)
{
  bool ok;
  QString path;
  path.sprintf("/composition_%d/scalebar_%d/", mComposition->id(), mId);

  double x = mComposition->fromMM(QgsProject::instance()->readDoubleEntry("Compositions", path + "x", 0, &ok));
  double y = mComposition->fromMM(QgsProject::instance()->readDoubleEntry("Compositions", path + "y", 0, &ok));
  QAbstractGraphicsShapeItem::setPos(x, y);

  mMap = QgsProject::instance()->readNumEntry("Compositions", path + "map", 0, &ok);
  mUnitLabel = QgsProject::instance()->readEntry("Compositions", path + "unit/label", "???", &ok);
  mMapUnitsPerUnit = QgsProject::instance()->readDoubleEntry("Compositions", path + "unit/mapunits", 1., &ok);

  mSegmentLength = QgsProject::instance()->readDoubleEntry("Compositions", path + "segmentsize", 1000., &ok);
  mNumSegments = QgsProject::instance()->readNumEntry("Compositions", path + "numsegments", 5, &ok);

  mFont.setFamily(QgsProject::instance()->readEntry("Compositions", path + "font/family", "", &ok));
  mFont.setPointSize(QgsProject::instance()->readNumEntry("Compositions", path + "font/size", 10, &ok));
  mFont.setWeight(QgsProject::instance()->readNumEntry("Compositions", path + "font/weight", (int) QFont::Normal, &ok));
  mFont.setUnderline(QgsProject::instance()->readBoolEntry("Compositions", path + "font/underline", false, &ok));
  mFont.setStrikeOut(QgsProject::instance()->readBoolEntry("Compositions", path + "font/strikeout", false, &ok));

  mPen.setWidthF(QgsProject::instance()->readDoubleEntry("Compositions", path + "pen/width", 1, &ok));

  recalculate();

  return true;
}

bool QgsComposerScalebar::removeSettings(void)
{
  std::cerr << "QgsComposerScalebar::deleteSettings" << std::endl;

  QString path;
  path.sprintf("/composition_%d/scalebar_%d", mComposition->id(), mId);
  return QgsProject::instance()->removeEntry("Compositions", path);
}

bool QgsComposerScalebar::writeXML(QDomNode & node, QDomDocument & document, bool temp)
{
  return true;
}

bool QgsComposerScalebar::readXML(QDomNode & node)
{
  return true;
}
