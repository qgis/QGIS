/***************************************************************************
    qgsadvanceddigitizingtoolsregistry.cpp
                             -------------------
    begin                : July 27 2024
    copyright            : (C) 2024 by Mathieu Pellerin
    email                : mathieu at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSADVANCEDDIGITIZINGTOOLSREGSITRY_H
#define QGSADVANCEDDIGITIZINGTOOLSREGSITRY_H

#include "qgis_sip.h"
#include "qgis_gui.h"

#include <QIcon>
#include <QMap>

class QgsAdvancedDigitizingDockWidget;
class QgsAdvancedDigitizingTool;
class QgsMapCanvas;

/**
 * \ingroup gui
 *  \brief Stores metadata about one advanced digitizing tool class.
 *  \since QGIS 3.40
 */
class GUI_EXPORT QgsAdvancedDigitizingToolAbstractMetadata
{
  public:
    /**
     * Constructor for QgsAdvancedDigitizingToolAbstractMetadata with the specified tool \a name.
     *
     * \a visibleName should be set to a translated, user visible name identifying the corresponding annotation item.
     *
     * An optional \a icon can be set, which will be used by the advanced digitizing dock widget.
     */
    QgsAdvancedDigitizingToolAbstractMetadata( const QString &name, const QString &visibleName, const QIcon &icon = QIcon() )
      : mName( name )
      , mVisibleName( visibleName )
      , mIcon( icon )
    {}

    virtual ~QgsAdvancedDigitizingToolAbstractMetadata() = default;

    //! Returns the tool's unique name
    QString name() const { return mName; }

    //! Returns the tool's translatable user-friendly name
    QString visibleName() const { return mVisibleName; }

    //! Returns the tool's icon
    QIcon icon() const { return mIcon; }

    //! Returns new tool of this type. Return NULLPTR on error
    virtual QgsAdvancedDigitizingTool *createTool( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget ) SIP_FACTORY;

  protected:
    QString mName;
    QString mVisibleName;
    QIcon mIcon;
};

#ifndef SIP_RUN

typedef std::function<QgsAdvancedDigitizingTool *( QgsMapCanvas *, QgsAdvancedDigitizingDockWidget * )> QgsAdvancedDigitizingToolFunc SIP_SKIP;

/**
 * \ingroup gui
 * \brief Convenience metadata class that uses static functions to handle advanced digitizing tool creation
 * \note not available in Python bindings
 * \since QGIS 3.40
 */
class GUI_EXPORT QgsAdvancedDigitizingToolMetadata : public QgsAdvancedDigitizingToolAbstractMetadata
{
  public:
    /**
     * Constructor for QgsAdvancedDigitizingToolAbstractMetadata with the specified tool \a name.
     *
     * \a visibleName should be set to a translated, user visible name identifying the corresponding annotation item.
     *
     * An optional \a icon can be set, which will be used by the advanced digitizing dock widget.
     *
     * A tool creation function can be declared through the \a toolFunction parameter.
     */
    QgsAdvancedDigitizingToolMetadata( const QString &name, const QString &visibleName, const QIcon &icon = QIcon(), const QgsAdvancedDigitizingToolFunc &toolFunction = nullptr )
      : QgsAdvancedDigitizingToolAbstractMetadata( name, visibleName, icon )
      , mToolFunc( toolFunction )
    {}

    //! Returns the tool creation function
    QgsAdvancedDigitizingToolFunc toolFunction() const { return mToolFunc; }

    //! Sets the tool creation \a function.
    void setToolFunction( const QgsAdvancedDigitizingToolFunc &function ) { mToolFunc = function; }

    QgsAdvancedDigitizingTool *createTool( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget ) override;

  protected:
    QgsAdvancedDigitizingToolFunc mToolFunc = nullptr;
};

#endif

/**
 * \ingroup gui
 * \brief Registry of available advanced digitizing tools.
 *
 * QgsAdvancedDigitizingToolsRegistry is not usually directly created, but rather accessed through
 * QgsGui::advancedDigitizingToolsRegistry().
 *
 * \since QGIS 3.40
 */
class GUI_EXPORT QgsAdvancedDigitizingToolsRegistry
{
  public:
    QgsAdvancedDigitizingToolsRegistry() = default;
    ~QgsAdvancedDigitizingToolsRegistry();

    QgsAdvancedDigitizingToolsRegistry( const QgsAdvancedDigitizingToolsRegistry &rh ) = delete;
    QgsAdvancedDigitizingToolsRegistry &operator=( const QgsAdvancedDigitizingToolsRegistry &rh ) = delete;

    //! Adds the default tools shipped in QGIS
    void addDefaultTools();

    //! Adds an advanced digitizing tool (take ownership) and return TRUE on success
    bool addTool( QgsAdvancedDigitizingToolAbstractMetadata *toolMetaData SIP_TRANSFER );

    //! Removes the advanced digitizing tool matching the provided \a name and return TRUE on success
    bool removeTool( const QString &name );

    //! Returns the advanced digitizing tool matching the provided \a name or NULLPTR when no match available
    QgsAdvancedDigitizingToolAbstractMetadata *toolMetadata( const QString &name );

    //! Returns the list of registered tool names
    const QStringList toolMetadataNames() const;

  private:
#ifdef SIP_RUN
    QgsAdvancedDigitizingToolsRegistry( const QgsAdvancedDigitizingToolsRegistry &rh );
#endif

    QMap<QString, QgsAdvancedDigitizingToolAbstractMetadata *> mTools;
};

#endif // QGSADVANCEDDIGITIZINGTOOLSREGSITRY_H
