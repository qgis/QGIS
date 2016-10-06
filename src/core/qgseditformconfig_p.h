/***************************************************************************
  qgseditformconfig_p - %{Cpp:License:ClassName}

 ---------------------
 begin                : 18.8.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSEDITFORMCONFIG_P_H
#define QGSEDITFORMCONFIG_P_H

#include <QMap>

#include "qgsfields.h"
#include "qgseditformconfig.h"

/// @cond PRIVATE

class QgsEditFormConfigPrivate : public QSharedData
{
  public:
    QgsEditFormConfigPrivate()
        : mInvisibleRootContainer( new QgsAttributeEditorContainer( QString::null, nullptr ) )
        , mConfiguredRootContainer( false )
        , mEditorLayout( QgsEditFormConfig::GeneratedLayout )
        , mInitCodeSource( QgsEditFormConfig::CodeSourceNone )
        , mSuppressForm( QgsEditFormConfig::SuppressDefault )
    {}

    QgsEditFormConfigPrivate( const QgsEditFormConfigPrivate& o )
        : QSharedData( o )
        , mInvisibleRootContainer( static_cast<QgsAttributeEditorContainer*>( o.mInvisibleRootContainer->clone( nullptr ) ) )
        , mConfiguredRootContainer( o.mConfiguredRootContainer )
        , mConstraints( o.mConstraints )
        , mConstraintsDescription( o.mConstraintsDescription )
        , mFieldEditables( o.mFieldEditables )
        , mLabelOnTop( o.mLabelOnTop )
        , mNotNull( o.mNotNull )
        , mEditorWidgetTypes( o.mEditorWidgetTypes )
        , mWidgetConfigs( o.mWidgetConfigs )
        , mEditorLayout( o.mEditorLayout )
        , mUiFormPath( o.mUiFormPath )
        , mInitFunction( o.mInitFunction )
        , mInitCodeSource( o.mInitCodeSource )
        , mInitCode( o.mInitCode )
        , mSuppressForm( o.mSuppressForm )
        , mFields( o.mFields )
    {}

    ~QgsEditFormConfigPrivate()
    {
      delete mInvisibleRootContainer;
    }

    /** The invisible root container for attribute editors in the drag and drop designer */
    QgsAttributeEditorContainer* mInvisibleRootContainer;

    /** This flag is set if the root container was configured by the user */
    bool mConfiguredRootContainer;

    QMap< QString, QString> mConstraints;
    QMap< QString, QString> mConstraintsDescription;
    QMap< QString, bool> mFieldEditables;
    QMap< QString, bool> mLabelOnTop;
    QMap< QString, bool> mNotNull;

    QMap<QString, QString> mEditorWidgetTypes;
    QMap<QString, QgsEditorWidgetConfig > mWidgetConfigs;

    /** Defines the default layout to use for the attribute editor (Drag and drop, UI File, Generated) */
    QgsEditFormConfig::EditorLayout mEditorLayout;

    /** Init form instance */
    QString mUiFormPath;
    /** Name of the python form init function */
    QString mInitFunction;
    /** Path of the python external file to be loaded */
    QString mInitFilePath;
    /** Choose the source of the init founction */
    QgsEditFormConfig::PythonInitCodeSource mInitCodeSource;
    /** Python init code provided in the dialog */
    QString mInitCode;

    /** Type of feature form suppression after feature creation */
    QgsEditFormConfig::FeatureFormSuppress mSuppressForm;

    QgsFields mFields;
};

/// @endcond

#endif // QGSEDITFORMCONFIG_P_H
