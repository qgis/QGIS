/***************************************************************************
    qgslegacyhelpers.cpp
     --------------------------------------
    Date                 : 13.5.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslegacyhelpers.h"

#include "qgsvectorlayer.h"

const QString QgsLegacyHelpers::convertEditType( QgsVectorLayer::EditType editType, QgsEditorWidgetConfig& cfg, QgsVectorLayer* vl, const QString& name, const QDomElement &editTypeElement )
{
  QString widgetType = "TextEdit"; // Fallback

  switch ( editType )
  {
    case QgsVectorLayer::ValueMap:
    {
      widgetType = "ValueMap";
      QDomNodeList valueMapNodes = editTypeElement.childNodes();

      for ( int j = 0; j < valueMapNodes.size(); j++ )
      {
        QDomElement value = valueMapNodes.at( j ).toElement();
        cfg.insert( value.attribute( "key" ), value.attribute( "value" ) );
      }
      break;
    }

    case QgsVectorLayer::EditRange:
    {
      widgetType = "Range";
      cfg.insert( "Style", "Edit" );
      cfg.insert( "Min", editTypeElement.attribute( "min" ).toFloat() );
      cfg.insert( "Max", editTypeElement.attribute( "max" ).toFloat() );
      cfg.insert( "Step", editTypeElement.attribute( "step" ).toFloat() );
      break;
    }

    case QgsVectorLayer::SliderRange:
    {
      widgetType = "SliderRange";
      cfg.insert( "Style", "Slider" );
      cfg.insert( "Min", editTypeElement.attribute( "min" ).toFloat() );
      cfg.insert( "Max", editTypeElement.attribute( "max" ).toFloat() );
      cfg.insert( "Step", editTypeElement.attribute( "step" ).toFloat() );
      break;
    }

    case QgsVectorLayer::DialRange:
    {
      widgetType = "DialRange";
      cfg.insert( "Style", "Dial" );
      cfg.insert( "Min", editTypeElement.attribute( "min" ).toFloat() );
      cfg.insert( "Max", editTypeElement.attribute( "max" ).toFloat() );
      cfg.insert( "Step", editTypeElement.attribute( "step" ).toFloat() );
      break;
    }

    case QgsVectorLayer::CheckBox:
    {
      widgetType = "CheckBox";
      cfg.insert( "CheckedState", editTypeElement.attribute( "checked" ) );
      cfg.insert( "UncheckedState", editTypeElement.attribute( "unchecked" ) );
      break;
    }

    case QgsVectorLayer::ValueRelation:
    {
      widgetType = "ValueRelation";
      cfg.insert( "AllowNull", editTypeElement.attribute( "allowNull" ) == "true" );
      cfg.insert( "OrderByValue", editTypeElement.attribute( "orderByValue" ) == "true" );
      cfg.insert( "AllowMulti", editTypeElement.attribute( "allowMulti", "false" ) == "true" );
      QString filterExpression;
      if ( editTypeElement.hasAttribute( "filterAttributeColumn" ) &&
           editTypeElement.hasAttribute( "filterAttributeValue" ) )
      {
        filterExpression = QString( "\"%1\"='%2'" )
                           .arg( editTypeElement.attribute( "filterAttributeColumn" ) )
                           .arg( editTypeElement.attribute( "filterAttributeValue" ) );
      }
      else
      {
        filterExpression  = editTypeElement.attribute( "filterExpression", QString::null );
      }
      cfg.insert( "FilterExpression", filterExpression );
      cfg.insert( "Layer", editTypeElement.attribute( "layer" ) );
      cfg.insert( "Key", editTypeElement.attribute( "key" ) );
      cfg.insert( "Value", editTypeElement.attribute( "value" ) );

      break;
    }

    case QgsVectorLayer::Calendar:
    {
      widgetType = "DateTime";
      cfg.insert( "display_format", editTypeElement.attribute( "dateFormat" ) );
      cfg.insert( "field_format", "yyyy-MM-dd" );
      break;
    }

    case QgsVectorLayer::Photo:
    {
      widgetType = "Photo";
      cfg.insert( "Width", editTypeElement.attribute( "widgetWidth" ).toInt() );
      cfg.insert( "Height", editTypeElement.attribute( "widgetHeight" ).toInt() );
      break;
    }

    case QgsVectorLayer::WebView:
    {
      widgetType = "WebView";
      cfg.insert( "Width", editTypeElement.attribute( "widgetWidth" ).toInt() );
      cfg.insert( "Height", editTypeElement.attribute( "widgetHeight" ).toInt() );
      break;
    }

    case QgsVectorLayer::Classification:
    {
      widgetType = "Classification";
      break;
    }

    case QgsVectorLayer::FileName:
    {
      widgetType = "FileName";
      break;
    }

    case QgsVectorLayer::Immutable:
    {
      widgetType = "TextEdit";
      cfg.insert( "IsMultiline", false );
      vl->setFieldEditable( vl->fields().fieldNameIndex( name ), false );
      break;
    }

    case QgsVectorLayer::Hidden:
    {
      widgetType = "Hidden";
      break;
    }

    case QgsVectorLayer::LineEdit:
    {
      widgetType = "TextEdit";
      cfg.insert( "IsMultiline", false );
      break;
    }

    case QgsVectorLayer::TextEdit:
    {
      widgetType = "TextEdit";
      cfg.insert( "IsMultiline", true );
      cfg.insert( "UseHtml", false );
      break;
    }

    case QgsVectorLayer::Enumeration:
    {
      widgetType = "Enumeration";
      break;
    }

    case QgsVectorLayer::UniqueValues:
    {
      widgetType = "UniqueValues";
      cfg.insert( "Editable", false );
      break;
    }

    case QgsVectorLayer::UniqueValuesEditable:
    {
      widgetType = "UniqueValues";
      cfg.insert( "Editable", true );
      break;
    }

    case QgsVectorLayer::UuidGenerator:
    {
      widgetType = "UuidGenerator";
      break;
    }

    case QgsVectorLayer::Color:
    {
      widgetType = "Color";
      break;
    }

    case QgsVectorLayer::EditorWidgetV2: // Should not land here
      break;
  }

  return widgetType;
}

QgsVectorLayer::EditType QgsLegacyHelpers::convertEditType( const QString& editType, const QgsEditorWidgetConfig& cfg, QgsVectorLayer* vl, const QString& name )
{
  int idx = vl->fieldNameIndex( name );

  if ( !vl->fieldEditable( idx ) )
  {
    return QgsVectorLayer::Immutable;
  }

  if ( editType == "Hidden" )
  {
    return QgsVectorLayer::Hidden;
  }

  if ( editType == "ValueMap" )
  {
    return QgsVectorLayer::ValueMap;
  }

  if ( editType == "TextEdit" )
  {
    if ( cfg.value( "IsMultiline" ).toBool() )
    {
      return QgsVectorLayer::TextEdit;
    }
    else
    {
      return QgsVectorLayer::LineEdit;
    }
  }

  if ( editType == "Range" )
  {
    if ( cfg.value( "Style" ).toString() == "SliderRange" )
    {
      return QgsVectorLayer::SliderRange;
    }
    else if ( cfg.value( "Style" ).toString() == "DialRange" )
    {
      return QgsVectorLayer::DialRange;
    }
    else
    {
      return QgsVectorLayer::EditRange;
    }
  }

  if ( editType == "UuidGenerator" )
  {
    return QgsVectorLayer::UuidGenerator;
  }

  if ( editType == "UniqueValues" )
  {
    if ( cfg.value( "Editable" ).toBool() )
    {
      return QgsVectorLayer::UniqueValuesEditable;
    }
    else
    {
      return QgsVectorLayer::UniqueValues;

    }
  }

  if ( editType == "Classification" )
  {
    return QgsVectorLayer::Classification;
  }

  if ( editType == "CheckBox" )
  {
    return QgsVectorLayer::CheckBox;
  }

  if ( editType == "DateTime" )
  {
    return QgsVectorLayer::Calendar;
  }

  if ( editType == "FileName" )
  {
    return QgsVectorLayer::FileName;
  }

  if ( editType == "WebView" )
  {
    return QgsVectorLayer::WebView;
  }

  if ( editType == "Photo" )
  {
    return QgsVectorLayer::Photo;
  }

  if ( editType == "Color" )
  {
    return QgsVectorLayer::Color;
  }

  if ( editType == "Enumeration" )
  {
    return QgsVectorLayer::Enumeration;
  }

  return QgsVectorLayer::EditorWidgetV2;
}
