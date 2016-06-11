/***************************************************************************
    qgsdatadefinedsymboldialog.h
    ---------------------
    begin                : March 2013
    copyright            : (C) 2013 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDATADEFINEDSYMBOLLAYERDIALOG_H
#define QGSDATADEFINEDSYMBOLLAYERDIALOG_H

#include "ui_qgsdatadefinedsymboldialogbase.h"
#include <QDialog>

class QgsVectorLayer;
class QComboBox;


/** \ingroup gui
 * \class QgsDataDefinedSymbolDialog
 * \deprecated no longer used and will be removed in QGIS 3.0
 */
class GUI_EXPORT QgsDataDefinedSymbolDialog: public QDialog, private Ui::QgsDataDefinedSymbolDialog
{
    Q_OBJECT
  public:

    struct DataDefinedSymbolEntry
    {
      DataDefinedSymbolEntry( const QString& p, const QString& t, const QString& v, const QString& h ):
          property( p ), title( t ), initialValue( v ), helpText( h ) {}
      QString property;
      QString title;
      QString initialValue;
      QString helpText;
    };

    //! @deprecated will be removed in QGIS 3.0
    Q_DECL_DEPRECATED QgsDataDefinedSymbolDialog( const QList< DataDefinedSymbolEntry >& entries, const QgsVectorLayer* vl, QWidget * parent = nullptr, const Qt::WindowFlags& f = nullptr );
    ~QgsDataDefinedSymbolDialog();

    //! @deprecated will be removed in QGIS 3.0
    Q_DECL_DEPRECATED QMap< QString, QString > dataDefinedProperties() const;

    //common help texts
    //! @deprecated will be removed in QGIS 3.0
    Q_DECL_DEPRECATED static QString doubleHelpText();
    //! @deprecated will be removed in QGIS 3.0
    Q_DECL_DEPRECATED static QString colorHelpText();
    //! @deprecated will be removed in QGIS 3.0
    Q_DECL_DEPRECATED static QString offsetHelpText();
    //! @deprecated will be removed in QGIS 3.0
    Q_DECL_DEPRECATED static QString fileNameHelpText();
    //! @deprecated will be removed in QGIS 3.0
    Q_DECL_DEPRECATED static QString horizontalAnchorHelpText();
    //! @deprecated will be removed in QGIS 3.0
    Q_DECL_DEPRECATED static QString verticalAnchorHelpText();
    //! @deprecated will be removed in QGIS 3.0
    Q_DECL_DEPRECATED static QString gradientTypeHelpText();
    //! @deprecated will be removed in QGIS 3.0
    Q_DECL_DEPRECATED static QString gradientCoordModeHelpText();
    //! @deprecated will be removed in QGIS 3.0
    Q_DECL_DEPRECATED static QString gradientSpreadHelpText();
    //! @deprecated will be removed in QGIS 3.0
    Q_DECL_DEPRECATED static QString boolHelpText();
    //! @deprecated will be removed in QGIS 3.0
    Q_DECL_DEPRECATED static QString lineStyleHelpText();
    //! @deprecated will be removed in QGIS 3.0
    Q_DECL_DEPRECATED static QString joinStyleHelpText();
    //! @deprecated will be removed in QGIS 3.0
    Q_DECL_DEPRECATED static QString capStyleHelpText();
    //! @deprecated will be removed in QGIS 3.0
    Q_DECL_DEPRECATED static QString fillStyleHelpText();

  private:
    const QgsVectorLayer* mVectorLayer;
};

#endif // QGSDATADEFINEDSYMBOLLAYERDIALOG_H
