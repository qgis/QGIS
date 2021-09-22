/***************************************************************************
    qgsdatetimeeditconfig.cpp
     --------------------------------------
    Date                 : 03.2014
    Copyright            : (C) 2014 Denis Rouzaud
    Email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdatetimeeditconfig.h"
#include "qgsdatetimeeditfactory.h"
#include "qgsvectorlayer.h"
#include "qgsdatetimefieldformatter.h"

QgsDateTimeEditConfig::QgsDateTimeEditConfig( QgsVectorLayer *vl, int fieldIdx, QWidget *parent )
  : QgsEditorConfigWidget( vl, fieldIdx, parent )
{
  setupUi( this );

  mFieldFormatComboBox->clear();
  mFieldFormatComboBox->addItem( tr( "Date" ), QgsDateTimeFieldFormatter::DATE_FORMAT );
  mFieldFormatComboBox->addItem( tr( "Time" ), QgsDateTimeFieldFormatter::TIME_FORMAT );
  mFieldFormatComboBox->addItem( tr( "Date Time" ), QgsDateTimeFieldFormatter::DATETIME_FORMAT );
  mFieldFormatComboBox->addItem( tr( "ISO Date Time" ), QgsDateTimeFieldFormatter::QT_ISO_FORMAT );
  mFieldFormatComboBox->addItem( tr( "Custom" ), QString() );

  mHelpLabel->setTextFormat( Qt::RichText );
  mHelpLabel->setText(
    QStringLiteral(
      "<html><head/><body>"
      "<table border=\"0\" style=\"margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px;\" cellspacing=\"2\" cellpadding=\"0\" bgcolor=\"#f6f6f6\">"
      "<thead><tr>"
      "<td style=\"vertical-align:top; padding-left:10; padding-right:15; padding-top:5; padding-bottom:5;\">"
      "<p align=\"left\"><span style=\"font-family:'Open Sans,sans-serif'; font-size:12px; font-weight:600; color:#363534;\">" )
    + tr( "Qt ISO Date format" )
    + QStringLiteral( "</span></p>"
                      "</td>"
                      "</tr></thead>"
                      "<tr>"
                      "<td bgcolor=\"#ffffff\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#ffffff;\">"
                      "<a href=\"http://www.iso.org/iso/catalogue_detail?csnumber=40874\">" )  //#spellok
    + tr( "ISO 8601" )
    + QStringLiteral( "</a> " )
    + tr( "extended format: either <code>yyyy-MM-dd</code> for dates or <code>yyyy-MM-ddTHH:mm:ss</code> (e.g. 2017-07-24T15:46:29), or with a time-zone suffix (Z for UTC otherwise an offset as [+|-]HH:mm) where appropriate for combined dates and times." )
    + QStringLiteral(
      "</span></p>"
      "</td>"
      "</tr>"
      "</table>"
      "<br>"
      "<table border=\"0\" style=\"margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px;\" cellspacing=\"2\" cellpadding=\"0\" bgcolor=\"#f6f6f6\">"
      "<thead><tr>"
      "<td style=\"vertical-align:top; padding-left:10; padding-right:15; padding-top:5; padding-bottom:5;\">"
      "<p align=\"center\"><span style=\"font-family:'Open Sans,sans-serif'; font-size:12px; font-weight:600; color:#363534;\">" )
    + tr( "Format" )
    + QStringLiteral( "</span></p>"
                      "</td>"
                      "<td style=\"vertical-align:top; padding-left:10; padding-right:15; padding-top:5; padding-bottom:5;\">"
                      "<p align=\"center\"><span style=\"font-family:'Open Sans,sans-serif'; font-size:12px; font-weight:600; color:#363534;\">" )
    + tr( "Examples result" )
    + QStringLiteral( "</span></p>"
                      "</td>"
                      "</tr></thead>"
                      "<tr>"
                      "<td bgcolor=\"#f6f6f6\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#f6f6f6;\">dd.MM.yyyy</span></p>"
                      "</td>"
                      "<td bgcolor=\"#f6f6f6\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#f6f6f6;\">21.05.2001</span></p>"
                      "</td>"
                      "</tr>"
                      "<tr>"
                      "<td bgcolor=\"#ffffff\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#ffffff;\">ddd MMMM d yy</span></p>"
                      "</td>"
                      "<td bgcolor=\"#ffffff\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#ffffff;\">Tue May 21 01</span></p>"
                      "</td>"
                      "</tr>"
                      "<tr>"
                      "<td bgcolor=\"#f6f6f6\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#f6f6f6;\">hh:mm:ss.zzz</span></p>"
                      "</td>"
                      "<td bgcolor=\"#f6f6f6\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#f6f6f6;\">14:13:09.042</span></p>"
                      "</td>"
                      "</tr>"
                      "<tr>"
                      "<td bgcolor=\"#ffffff\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#ffffff;\">h:m:s ap</span></p>"
                      "</td>"
                      "<td bgcolor=\"#ffffff\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#ffffff;\">2:13:9 pm</span></p>"
                      "</td>"
                      "</tr>"
                      "</table>"
                      "<p><br/></p>"
                      "<table border=\"0\" style=\"margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px;\" cellspacing=\"2\" cellpadding=\"0\" bgcolor=\"#f6f6f6\">"
                      "<thead><tr>"
                      "<td style=\"vertical-align:top; padding-left:10; padding-right:15; padding-top:5; padding-bottom:5;\">"
                      "<p align=\"center\"><span style=\"font-family:'Open Sans,sans-serif'; font-size:12px; font-weight:600; color:#363534;\">" )
    + tr( "Expression" )
    + QStringLiteral( "</span></p>"
                      "</td>"
                      "<td style=\"vertical-align:top; padding-left:10; padding-right:15; padding-top:5; padding-bottom:5;\">"
                      "<p align=\"center\"><span style=\"font-family:'Open Sans,sans-serif'; font-size:12px; font-weight:600; color:#363534;\">" )
    + tr( "Date output" )
    + QStringLiteral( "</span></p>"
                      "</td>"
                      "</tr></thead>"
                      "<tr>"
                      "<td bgcolor=\"#f6f6f6\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#f6f6f6;\">d</span></p>"
                      "</td>"
                      "<td bgcolor=\"#f6f6f6\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#f6f6f6;\">" )
    + tr( "the day as number without a leading zero (1 to 31)" )
    + QStringLiteral( "</span></p>"
                      "</td>"
                      "</tr>"
                      "<tr>"
                      "<td bgcolor=\"#ffffff\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#ffffff;\">dd</span></p>"
                      "</td>"
                      "<td bgcolor=\"#ffffff\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#ffffff;\">" )
    + tr( "the day as number with a leading zero (01 to 31)" )
    + QStringLiteral( "</span></p>"
                      "</td>"
                      "</tr>"
                      "<tr>"
                      "<td bgcolor=\"#f6f6f6\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#f6f6f6;\">ddd</span></p>"
                      "</td>"
                      "<td bgcolor=\"#f6f6f6\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#f6f6f6;\">" )
    + tr( "the abbreviated localized day name (e.g. 'Mon' to 'Sun'). Uses the system locale to localize the name, i.e. " )
    + QStringLiteral( "</span><a href=\"http://qt-project.org/doc/qt-5/qlocale.html#system\"><span style=\"font-family:'Arial,Open Sans,sans-serif'; font-size:12px; font-weight:600; text-decoration: underline; color:#00732f;\">QLocale</span></a><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e;\">().</span></p>"
                      "</td>"
                      "</tr>"
                      "<tr>"
                      "<td bgcolor=\"#ffffff\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#ffffff;\">dddd</span></p>"
                      "</td>"
                      "<td bgcolor=\"#ffffff\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#ffffff;\">" )
    + tr( "the long localized day name (e.g. 'Monday' to '" )
    + QStringLiteral( "</span><a href=\"http://qt-project.org/doc/qt-5/qt.html#DayOfWeek-enum\"><span style=\"font-family:'Arial,Open Sans,sans-serif'; font-size:12px; font-weight:600; text-decoration: underline; color:#00732f;\">Qt::Sunday</span></a><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e;\">')." )
    + tr( "Uses the system locale to localize the name, i.e. " )
    + QStringLiteral( "</span><a href=\"http://qt-project.org/doc/qt-5/qlocale.html#system\"><span style=\"font-family:'Arial,Open Sans,sans-serif'; font-size:12px; font-weight:600; text-decoration: underline; color:#00732f;\">QLocale</span></a><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e;\">().</span></p>"
                      "</td>"
                      "</tr>"
                      "<tr>"
                      "<td bgcolor=\"#f6f6f6\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#f6f6f6;\">M</span></p>"
                      "</td>"
                      "<td bgcolor=\"#f6f6f6\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#f6f6f6;\">" )
    + tr( "the month as number without a leading zero (1-12)" )
    + QStringLiteral( "</span></p>"
                      "</td>"
                      "</tr>"
                      "<tr>"
                      "<td bgcolor=\"#ffffff\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#ffffff;\">MM</span></p>"
                      "</td>"
                      "<td bgcolor=\"#ffffff\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#ffffff;\">" )
    + tr( "the month as number with a leading zero (01-12)" )
    + QStringLiteral( "</span></p>"
                      "</td>"
                      "</tr>"
                      "<tr>"
                      "<td bgcolor=\"#f6f6f6\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#f6f6f6;\">MMM</span></p>"
                      "</td>"
                      "<td bgcolor=\"#f6f6f6\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#f6f6f6;\">" )
    + tr( "the abbreviated localized month name (e.g. 'Jan' to 'Dec'). Uses the system locale to localize the name, i.e." )
    + QStringLiteral( "</span><a href=\"http://qt-project.org/doc/qt-5/qlocale.html#system\"><span style=\"font-family:'Arial,Open Sans,sans-serif'; font-size:12px; font-weight:600; text-decoration: underline; color:#00732f;\">QLocale</span></a><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e;\">().</span></p>"
                      "</td>"
                      "</tr>"
                      "<tr>"
                      "<td bgcolor=\"#ffffff\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#ffffff;\">MMMM</span></p>"
                      "</td>"
                      "<td bgcolor=\"#ffffff\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#ffffff;\">" )
    + tr( "the long localized month name (e.g. 'January' to 'December'). Uses the system locale to localize the name, i.e." )
    + QStringLiteral( "</span><a href=\"http://qt-project.org/doc/qt-5/qlocale.html#system\"><span style=\"font-family:'Arial,Open Sans,sans-serif'; font-size:12px; font-weight:600; text-decoration: underline; color:#00732f;\">QLocale</span></a><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e;\">().</span></p>"
                      "</td>"
                      "</tr>"
                      "<tr>"
                      "<td bgcolor=\"#f6f6f6\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#f6f6f6;\">yy</span></p>"
                      "</td>"
                      "<td bgcolor=\"#f6f6f6\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#f6f6f6;\">" )
    + tr( "the year as two digit number (00-99)" )
    + QStringLiteral( "</span></p>"
                      "</td>"
                      "</tr>"
                      "<tr>"
                      "<td bgcolor=\"#ffffff\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#ffffff;\">yyyy</span></p>"
                      "</td>"
                      "<td bgcolor=\"#ffffff\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#ffffff;\">" )
    + tr( "the year as four digit number" )
    + QStringLiteral( "</span></p>"
                      "</td>"
                      "</tr>"
                      "</table>"
                      "<p><br/></p>"
                      "<table border=\"0\" style=\"margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px;\" cellspacing=\"2\" cellpadding=\"0\" bgcolor=\"#f6f6f6\">"
                      "<thead><tr>"
                      "<td style=\"vertical-align:top; padding-left:10; padding-right:15; padding-top:5; padding-bottom:5;\">"
                      "<p align=\"center\"><span style=\"font-family:'Open Sans,sans-serif'; font-size:12px; font-weight:600; color:#363534;\">" )
    + tr( "Expression" )
    + QStringLiteral( "</span></p>"
                      "</td>"
                      "<td style=\"vertical-align:top; padding-left:10; padding-right:15; padding-top:5; padding-bottom:5;\">"
                      "<p align=\"center\"><span style=\"font-family:'Open Sans,sans-serif'; font-size:12px; font-weight:600; color:#363534;\">" )
    + tr( "Time output" )
    + QStringLiteral( "</span></p>"
                      "</td>"
                      "</tr></thead>"
                      "<tr>"
                      "<td bgcolor=\"#f6f6f6\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#f6f6f6;\">h</span></p>"
                      "</td>"
                      "<td bgcolor=\"#f6f6f6\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#f6f6f6;\">" )
    + tr( "the hour without a leading zero (0 to 23 or 1 to 12 if AM/PM display)" )
    + QStringLiteral( "</span></p>"
                      "</td>"
                      "</tr>"
                      "<tr>"
                      "<td bgcolor=\"#ffffff\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#ffffff;\">hh</span></p>"
                      "</td>"
                      "<td bgcolor=\"#ffffff\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#ffffff;\">" )
    + tr( "the hour with a leading zero (00 to 23 or 01 to 12 if AM/PM display)" )
    + QStringLiteral( "</span></p>"
                      "</td>"
                      "</tr>"
                      "<tr>"
                      "<td bgcolor=\"#f6f6f6\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#f6f6f6;\">H</span></p>"
                      "</td>"
                      "<td bgcolor=\"#f6f6f6\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#f6f6f6;\">" )
    + tr( "the hour without a leading zero (0 to 23, even with AM/PM display)" )
    + QStringLiteral( "</span></p>"
                      "</td>"
                      "</tr>"
                      "<tr>"
                      "<td bgcolor=\"#ffffff\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#ffffff;\">HH</span></p>"
                      "</td>"
                      "<td bgcolor=\"#ffffff\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#ffffff;\">" )
    + tr( "the hour with a leading zero (00 to 23, even with AM/PM display)" )
    + QStringLiteral( "</span></p>"
                      "</td>"
                      "</tr>"
                      "<tr>"
                      "<td bgcolor=\"#f6f6f6\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#f6f6f6;\">m</span></p>"
                      "</td>"
                      "<td bgcolor=\"#f6f6f6\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#f6f6f6;\">" )
    + tr( "the minute without a leading zero (0 to 59)" )
    + QStringLiteral( "</span></p>"
                      "</td>"
                      "</tr>"
                      "<tr>"
                      "<td bgcolor=\"#ffffff\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#ffffff;\">mm</span></p>"
                      "</td>"
                      "<td bgcolor=\"#ffffff\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#ffffff;\">" )
    + tr( "the minute with a leading zero (00 to 59)" )
    + QStringLiteral( "</span></p>"
                      "</td>"
                      "</tr>"
                      "<tr>"
                      "<td bgcolor=\"#f6f6f6\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#f6f6f6;\">s</span></p>"
                      "</td>"
                      "<td bgcolor=\"#f6f6f6\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#f6f6f6;\">" )
    + tr( "the second without a leading zero (0 to 59)" )
    + QStringLiteral( "</span></p>"
                      "</td>"
                      "</tr>"
                      "<tr>"
                      "<td bgcolor=\"#ffffff\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#ffffff;\">ss</span></p>"
                      "</td>"
                      "<td bgcolor=\"#ffffff\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#ffffff;\">" )
    + tr( "the second with a leading zero (00 to 59)" )
    + QStringLiteral( "</span></p>"
                      "</td>"
                      "</tr>"
                      "<tr>"
                      "<td bgcolor=\"#f6f6f6\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#f6f6f6;\">z</span></p>"
                      "</td>"
                      "<td bgcolor=\"#f6f6f6\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#f6f6f6;\">" )
    + tr( "the milliseconds without trailing zeroes (0 to 999)" )
    + QStringLiteral( "</span></p>"
                      "</td>"
                      "</tr>"
                      "<tr>"
                      "<td bgcolor=\"#ffffff\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#ffffff;\">zzz</span></p>"
                      "</td>"
                      "<td bgcolor=\"#ffffff\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#ffffff;\">" )
    + tr( "the milliseconds with trailing zeroes (000 to 999)" )
    + QStringLiteral( "</span></p>"
                      "</td>"
                      "</tr><tr>"
                      "<td bgcolor=\"#f6f6f6\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#f6f6f6;\">AP or A</span></p>"
                      "</td>"
                      "<td bgcolor=\"#f6f6f6\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#f6f6f6;\">" )
    + tr( "use AM/PM display." )
    + QStringLiteral( "</span><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; font-style:italic; color:#66666e;\">A/AP</span><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e;\"> " )
    + tr( "will be replaced by either" )
    + QStringLiteral( "&quot;AM&quot;" )
    + tr( "or" )
    + QStringLiteral( "&quot;PM&quot;.</span></p>"
                      "</td>"
                      "</tr>"
                      "<tr>"
                      "<td bgcolor=\"#ffffff\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#ffffff;\">ap " )
    + tr( "or" )
    + QStringLiteral( " a</span></p>"
                      "</td>"
                      "<td bgcolor=\"#ffffff\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#ffffff;\">" )
    + tr( "use am/pm display." )
    + QStringLiteral( "</span><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; font-style:italic; color:#66666e;\">a/ap</span><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e;\"> " )
    + tr( "will be replaced by either " )
    + QStringLiteral( "&quot;am&quot;" )
    + tr( "or" )
    + QStringLiteral( "&quot;pm&quot;.</span></p>"
                      "</td>"
                      "</tr>"
                      "<tr>"
                      "<td bgcolor=\"#f6f6f6\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#f6f6f6;\">t</span></p>"
                      "</td>"
                      "<td bgcolor=\"#f6f6f6\" style=\"vertical-align:top; padding-left:10; padding-right:10; padding-top:3; padding-bottom:3;\">"
                      "<p><span style=\"font-family:'Open Sans,sans-serif'; font-size:11px; color:#66666e; background-color:#f6f6f6;\">" )
    + tr( "the timezone (for example &quot;CEST&quot;)" )
    + QStringLiteral( "</span></p>"
                      "</td>"
                      "</tr>"
                      "</table>"
                      "<p><br/></p>"
                      "</body></html>" )
  );

  mDemoDateTimeEdit->setDateTime( QDateTime::currentDateTime() );

  connect( mDisplayFormatEdit, &QLineEdit::textChanged, this, &QgsDateTimeEditConfig::updateDemoWidget );
  connect( mCalendarPopupCheckBox, &QAbstractButton::toggled, this, &QgsDateTimeEditConfig::updateDemoWidget );

  connect( mFieldFormatComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsDateTimeEditConfig::updateFieldFormat );
  connect( mFieldFormatEdit, &QLineEdit::textChanged, this, &QgsDateTimeEditConfig::updateDisplayFormat );
  connect( mDisplayFormatComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsDateTimeEditConfig::displayFormatChanged );

  connect( mFieldHelpToolButton, &QAbstractButton::clicked, this, &QgsDateTimeEditConfig::showHelp );
  connect( mDisplayHelpToolButton, &QAbstractButton::clicked, this, &QgsDateTimeEditConfig::showHelp );

  connect( mFieldFormatEdit, &QLineEdit::textChanged, this, &QgsEditorConfigWidget::changed );
  connect( mDisplayFormatEdit, &QLineEdit::textChanged, this, &QgsEditorConfigWidget::changed );
  connect( mCalendarPopupCheckBox, &QAbstractButton::toggled, this, &QgsEditorConfigWidget::changed );
  connect( mAllowNullCheckBox, &QAbstractButton::toggled, this, &QgsEditorConfigWidget::changed );

  // initialize
  updateFieldFormat( mFieldFormatComboBox->currentIndex() );
  displayFormatChanged( mDisplayFormatComboBox->currentIndex() );
}


void QgsDateTimeEditConfig::updateDemoWidget()
{
  mDemoDateTimeEdit->setDisplayFormat( mDisplayFormatEdit->text() );
  mDemoDateTimeEdit->setCalendarPopup( mCalendarPopupCheckBox->isChecked() );
}


void QgsDateTimeEditConfig::updateFieldFormat( int idx )
{
  Q_UNUSED( idx )
  const QString format = mFieldFormatComboBox->currentData().toString();
  const bool custom = format.isEmpty();
  if ( !custom )
  {
    mFieldFormatEdit->setText( format );
  }
  else if ( mFieldFormatEdit->text() == QgsDateTimeFieldFormatter::QT_ISO_FORMAT )
  {
    mFieldFormatEdit->setText( QgsDateTimeFieldFormatter::DISPLAY_FOR_ISO_FORMAT );
  }

  mFieldFormatEdit->setEnabled( custom );
  mFieldHelpToolButton->setVisible( custom );
  if ( mFieldHelpToolButton->isHidden() && mDisplayHelpToolButton->isHidden() )
  {
    mHelpScrollArea->setVisible( false );
  }
}


void QgsDateTimeEditConfig::updateDisplayFormat( const QString &fieldFormat )
{
  if ( mDisplayFormatComboBox->currentIndex() == 0 )
  {
    // i.e. display format is default
    if ( mFieldFormatComboBox->currentData() == QgsDateTimeFieldFormatter::QT_ISO_FORMAT )
    {
      mDisplayFormatEdit->setText( QgsDateTimeFieldFormatter::DISPLAY_FOR_ISO_FORMAT );
    }
    else
    {
      mDisplayFormatEdit->setText( fieldFormat );
    }
  }
}


void QgsDateTimeEditConfig::displayFormatChanged( int idx )
{
  const bool custom = idx == 1;
  mDisplayFormatEdit->setEnabled( custom );
  mDisplayHelpToolButton->setVisible( custom );
  if ( mFieldHelpToolButton->isHidden() && mDisplayHelpToolButton->isHidden() )
  {
    mHelpScrollArea->setVisible( false );
  }
  if ( !custom )
  {
    if ( mFieldFormatComboBox->currentData() == QgsDateTimeFieldFormatter::QT_ISO_FORMAT )
    {
      mDisplayFormatEdit->setText( QgsDateTimeFieldFormatter::DISPLAY_FOR_ISO_FORMAT );
    }
    else
    {
      mDisplayFormatEdit->setText( mFieldFormatEdit->text() );
    }
  }
}

void QgsDateTimeEditConfig::showHelp( bool buttonChecked )
{
  mFieldHelpToolButton->setChecked( buttonChecked );
  mDisplayHelpToolButton->setChecked( buttonChecked );
  mHelpScrollArea->setVisible( buttonChecked );
}


QVariantMap QgsDateTimeEditConfig::config()
{
  QVariantMap myConfig;

  myConfig.insert( QStringLiteral( "field_iso_format" ), mFieldFormatEdit->text() == QgsDateTimeFieldFormatter::QT_ISO_FORMAT );
  myConfig.insert( QStringLiteral( "field_format" ), mFieldFormatEdit->text() );
  myConfig.insert( QStringLiteral( "display_format" ), mDisplayFormatEdit->text() );
  myConfig.insert( QStringLiteral( "calendar_popup" ), mCalendarPopupCheckBox->isChecked() );
  myConfig.insert( QStringLiteral( "allow_null" ), mAllowNullCheckBox->isChecked() );

  return myConfig;
}

void QgsDateTimeEditConfig::setConfig( const QVariantMap &config )
{
  const QgsField fieldDef = layer()->fields().at( field() );
  const QString fieldFormat = config.value( QStringLiteral( "field_format" ), QgsDateTimeFieldFormatter::defaultFormat( fieldDef.type() ) ).toString();
  mFieldFormatEdit->setText( fieldFormat );

  const int idx = mFieldFormatComboBox->findData( fieldFormat );
  if ( idx >= 0 )
  {
    mFieldFormatComboBox->setCurrentIndex( idx );
  }
  else
  {
    mFieldFormatComboBox->setCurrentIndex( 4 );
  }

  const QString displayFormat = config.value( QStringLiteral( "display_format" ), QgsDateTimeFieldFormatter::defaultFormat( fieldDef.type() ) ).toString();
  mDisplayFormatEdit->setText( displayFormat );
  if ( displayFormat == mFieldFormatEdit->text() )
  {
    mDisplayFormatComboBox->setCurrentIndex( 0 );
  }
  else
  {
    mDisplayFormatComboBox->setCurrentIndex( 1 );
  }

  mCalendarPopupCheckBox->setChecked( config.value( QStringLiteral( "calendar_popup" ), true ).toBool() );
  mAllowNullCheckBox->setChecked( config.value( QStringLiteral( "allow_null" ), true ).toBool() );
}
