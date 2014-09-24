/***************************************************************************
                         qgsgraduatedrendererv2classeditor.cpp  -  description
                             -------------------
    begin                : September 2014
    copyright            : (C) 2014 by Chris Crook
    email                : ccrook@linz.govt.nz
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgraduatedrendererclasseditor.h"
#include "qmessagebox.h"

const int MaxRanges=900;
const int WarnRanges=100;

QgsGraduatedRendererClassEditor::QgsGraduatedRendererClassEditor( QWidget *parent, Qt::WindowFlags fl )
    : QDialog( parent, fl ),
      mRenderer(0),
      mLayer(0),
      mMinValue(0.0),
      mMaxValue(0.0)
{
  setupUi( this );
}

QgsGraduatedRendererClassEditor::~QgsGraduatedRendererClassEditor()
{

}

void QgsGraduatedRendererClassEditor::setRendererAndLayer( QgsGraduatedSymbolRendererV2 *renderer, QgsVectorLayer *layer )
{
  mRenderer = renderer;
  mLayer = layer;
  mMinValue=0.0;
  mMaxValue=0.0;
  mLayerName->setText("");
  mAttrName->setText("");
  mRangeString->setText("");
  if( mLayer && mRenderer )
  {
    mLayerName->setText( mLayer->name() );
    mAttrName->setText( mRenderer->classAttribute());
    QApplication::setOverrideCursor( Qt::WaitCursor );
    QList<double> values = mRenderer->getDataValues(mLayer);
    QApplication::restoreOverrideCursor();
    if( values.length() > 0)
    {
      qSort( values );
      mMinValue = values.first();
      mMaxValue = values.last();
      int ndp=mRenderer->decimalPlaces();
      QString range=QString::number(mMinValue,'f',ndp)+" - "+QString::number(mMaxValue,'f',ndp);
      mRangeString->setText(range);
      mBreakPoints->setPlainText( classesToString() );
    }
  }
}

void QgsGraduatedRendererClassEditor::accept()
{
  QgsRangeList ranges;
  QString breakString=mBreakPoints->toPlainText();
  QString error=stringToClasses(breakString,ranges);
  if( error == "" )
  {
    applyRangesToRenderer(ranges);
    QDialog::accept();
    return;
  }
  // Crude mechanism for identifying errors that have been displayed.
  if( error[0] != ' ' )
  {
    QMessageBox::warning(this,tr("Error"),error,QMessageBox::Ok);
  }
  else
  {
    error=error.mid(1);
  }
  mStatusLabel->setText(error);
}


QString QgsGraduatedRendererClassEditor::classesToString()
{
  if( ! mRenderer ) return "";
  QString breaks("");
  const QgsRangeList& ranges=mRenderer->ranges();
  bool first=true;
  QString lastValueStr;
  QString valueStr;
  int ndp=mRenderer->decimalPlaces()+3; // Add 3 to preserve accuracy
  foreach ( const QgsRendererRangeV2& range, ranges )
  {
    double value=range.lowerValue();
    valueStr=QString::number(value,'f',ndp)+"\n";
    if( first )
    {
      breaks += valueStr;
      first=false;
    }
    else if( valueStr != lastValueStr )
    {
      breaks += "|\n";
      breaks += valueStr;
    }
    value=range.upperValue();
    valueStr=QString::number(value,'f',ndp)+"\n";
    breaks += valueStr;
    lastValueStr=valueStr;
  }
  return breaks;
}

QString QgsGraduatedRendererClassEditor::stringToClasses( QString breaks, QgsRangeList &ranges )
{
  // Initiallize the ranges list
  ranges.clear();

  // Normallize string using regex...
  breaks=breaks.trimmed();
  breaks=breaks.replace(QRegExp("\\s*([\\/\\|])\\s*"),"\\1");
  breaks=breaks.replace(QRegExp("\\s+")," ");

  if( breaks == "" ) return QString("");

  // A valid string is now a set of number strings separated by
  // one of ' ','|', or '/'
  QStringList parts=breaks.split(QRegExp("(?=[\\s\\/\\|])"));

  // Check that the numbers are all valid
  int offset=0;
  QList<double> values;
  foreach( const QString &part, parts)
  {
    QString number=part.mid(offset);
    if( number.length() < 1 ) return QString("Missing number in list");
    offset=1;
    bool ok;
    values.append(number.toDouble( &ok ));
    if( ! ok )
    {
      return QString("Invalid number "+number);
    }
  }

  // Is this a min/max/increment type range

  if( values.length() == 1 || parts[1][0]=='/')
  {
    double minimum=mMinValue;
    double maximum=mMaxValue;
    double increment;
    if( values.length() > 1 )
    {
      if( values.length() != 3 )
      {
        return tr("min/max/increment type breakpoints must have three numeric values");
      }
      if( parts[2][0] != '/')
      {
        return tr("Error in min/max/increment string near %1").arg(parts[2]);
      }
      minimum=values[0];
      maximum=values[1];
      if( minimum > maximum )
      {
        return tr("Minimum value %1 less than maximum %2").arg(parts[0]).arg(parts[1].mid(1));
      }
      increment=values[2];
    }
    else
    {
      increment=values[0];
    }
    if( increment <= 0 )
    {
      QString last=parts.last();
      if( parts.length() > 0 ) last=last.mid(1);
      return tr("Increment %1 cannot be less than 0").arg(last);
    }

    if( increment < (maximum-minimum)/MaxRanges )
    {
      return tr("Increment too small - will generate more than %1 classes").arg(MaxRanges);
    }
    if( increment < (maximum-minimum)/WarnRanges )
    {
      QString warning=tr("Increment size will generate more than %1 classes").arg(WarnRanges);
      int result = QMessageBox::question(this,tr("Warning"),
                           tr("Too many ranges %1. Continue?").arg(warning),
                           QMessageBox::Ok | QMessageBox::Cancel);
      // Crudely set first characer to blank to indicate that the message has already been
      // displayed.
      if( result != QMessageBox::Ok ) return QString(" ")+warning;
    }
    double value0=floor(minimum/increment)*increment;
    do
    {
      double value1=value0+increment;
      ranges.append(QgsRendererRangeV2(value0,value1,0,""));
      value0=value1;
    } while( value0 < maximum);
  }

  // Values supplied as set of ranges
  else
  {
    for( int i = 0; i < values.length()-1; i++ )
    {
      if( values[i] >= values[i+1])
      {
        return tr("Breakpoints not in ascending order near %1").arg(parts[i+1]);
      }
      if( parts[i][0] == '|' && parts[i+1][0] == '|' )
      {
        return tr("Cannot have adjacent ranges skipped near %1").arg(parts[i+1]);
      }
      if( parts[i+1][0] == '/' )
      {
        return tr("Invalid breakpoint syntax near %1").arg(parts[i+1]);
      }
    }
    if( parts[1][0] == '|' || parts.last()[0] == '|' )
    {
      return tr("Cannot skip first or last range");
    }
    for( int i = 0; i < values.length()-1; i++ )
    {
      if( parts[i+1][0] == '|') continue;
      ranges.append(QgsRendererRangeV2(values[i],values[i+1],0,""));
    }
  }
  return QString();
}

void QgsGraduatedRendererClassEditor::applyRangesToRenderer( QgsRangeList &ranges )
{
  if( ! mRenderer ) return;
  mRenderer->deleteAllClasses();
  foreach( const QgsRendererRangeV2 &range, ranges )
  {
    QString label=mRenderer->defaultRangeLabel(range);
    mRenderer->addClass(QgsRendererRangeV2(
                          range.lowerValue(),
                          range.upperValue(),
                          mRenderer->sourceSymbol()->clone(),
                          label
                          ));
  }
}
