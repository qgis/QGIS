/***************************************************************************
    qgseditorwidgetautoconf.cpp
    ---------------------
    begin                : July 2016
    copyright            : (C) 2016 by Patrick Valsecchi
    email                : patrick.valsecchi at camptocamp.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgseditorwidgetautoconf.h"
#include "qgseditorwidgetregistry.h"

/** \ingroup gui
 * Widget auto conf plugin that guesses what widget type to use in function of what the widgets support.
 *
 * @note not available in Python bindings
 * @note added in QGIS 3.0
 */
class FromFactoriesPlugin: public QgsEditorWidgetAutoConfPlugin
{
  public:
    virtual QgsEditorWidgetSetup editorWidgetSetup( const QgsVectorLayer* vl, const QString& fieldName, int& score ) const override
    {
      int bestScore = 0;
      QString bestType;
      const QMap<QString, QgsEditorWidgetFactory*> factories = QgsEditorWidgetRegistry::instance()->factories();
      for ( QMap<QString, QgsEditorWidgetFactory*>::const_iterator i = factories.begin(); i != factories.end(); ++i )
      {
        const int index = vl->fields().lookupField( fieldName );
        if ( index >= 0 )
        {
          const int score = i.value()->fieldScore( vl, index );
          if ( score > bestScore )
          {
            bestType = i.key();
            bestScore = score;
          }
        }
      }
      if ( bestScore > 0 )
      {
        score = 10;
        return QgsEditorWidgetSetup( bestType, QgsEditorWidgetConfig() );
      }
      return QgsEditorWidgetSetup();
    }
};


/** \ingroup gui
 * Widget auto conf plugin that reads the widget setup to use from what the data provider says.
 *
 * @note not available in Python bindings
 * @note added in QGIS 3.0
 */
class FromDbTablePlugin: public QgsEditorWidgetAutoConfPlugin
{
  public:
    virtual QgsEditorWidgetSetup editorWidgetSetup( const QgsVectorLayer* vl, const QString& fieldName, int& score ) const override
    {
      QgsField field = vl->fields().field( fieldName );
      if ( !field.editorWidgetSetup().isNull() )
      {
        score = 20;
        return field.editorWidgetSetup();
      }
      else
      {
        return QgsEditorWidgetSetup();
      }
    }
};

///@cond PRIVATE
QgsEditorWidgetAutoConf::QgsEditorWidgetAutoConf()
{
  registerPlugin( new FromFactoriesPlugin() );
  registerPlugin( new FromDbTablePlugin() );
}

QgsEditorWidgetSetup QgsEditorWidgetAutoConf::editorWidgetSetup( const QgsVectorLayer* vl, const QString& fieldName ) const
{
  QgsEditorWidgetSetup result( "TextEdit", QgsEditorWidgetConfig() );

  if ( vl->fields().indexFromName( fieldName ) >= 0 )
  {
    int bestScore = 0;
    Q_FOREACH ( QSharedPointer<QgsEditorWidgetAutoConfPlugin> cur, plugins )
    {
      int score = 0;
      const QgsEditorWidgetSetup curResult = cur->editorWidgetSetup( vl, fieldName, score );
      if ( score > bestScore )
      {
        result = curResult;
        bestScore = score;
      }
    }
  }

  return result;
}

void QgsEditorWidgetAutoConf::registerPlugin( QgsEditorWidgetAutoConfPlugin* plugin )
{
  plugins.append( QSharedPointer<QgsEditorWidgetAutoConfPlugin>( plugin ) );
}
///@endcond
