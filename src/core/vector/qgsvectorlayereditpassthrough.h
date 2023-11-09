/***************************************************************************
    qgsvectorlayereditpassthrough.h
    ---------------------
    begin                : Jan 12 2015
    copyright            : (C) 2015 by Sandro Mani
    email                : manisandro at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSVECTORLAYEREDITPASSTHROUGH_H
#define QGSVECTORLAYEREDITPASSTHROUGH_H

#include "qgis_core.h"
#include "qgsvectorlayereditbuffer.h"

class QgsVectorLayer;
class QgsVectorLayerUndoPassthroughCommand;
class QgsTransaction;

#ifdef SIP_RUN
% ModuleHeaderCode
#include <qgsvectorlayereditpassthrough.h>
% End
#endif

/**
 * \ingroup core
 * \class QgsVectorLayerEditPassthrough
 */
class CORE_EXPORT QgsVectorLayerEditPassthrough : public QgsVectorLayerEditBuffer
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( qobject_cast<QgsVectorLayerEditPassthrough *>( sipCpp ) )
      sipType = sipType_QgsVectorLayerEditPassthrough;
    else
      sipType = nullptr;
    SIP_END
#endif

    Q_OBJECT
  public:
    QgsVectorLayerEditPassthrough( QgsVectorLayer *layer );
    bool isModified() const override;
    bool addFeature( QgsFeature &f ) override;
    bool addFeatures( QgsFeatureList &features ) override;
    bool deleteFeature( QgsFeatureId fid ) override;
    bool deleteFeatures( const QgsFeatureIds &fids ) override;
    bool changeGeometry( QgsFeatureId fid, const QgsGeometry &geom ) override;
    bool changeAttributeValue( QgsFeatureId fid, int field, const QVariant &newValue, const QVariant &oldValue = QVariant() ) override;

    /**
     * Changes values of attributes (but does not commit it).
     * \returns TRUE if attributes are well updated, FALSE otherwise
     * \since QGIS 3.0
     */
    bool changeAttributeValues( QgsFeatureId fid, const QgsAttributeMap &newValues, const QgsAttributeMap &oldValues ) override;

    bool addAttribute( const QgsField &field ) override;
    bool deleteAttribute( int attr ) override;
    bool renameAttribute( int attr, const QString &newName ) override;
    bool commitChanges( QStringList &commitErrors ) override;
    void rollBack() override;

    /**
     * Update underlying data with a SQL query embedded in a transaction.
     *
     * \param transaction Transaction in which the sql query has been run
     * \param sql The SQL query updating data
     * \param name The name of the undo/redo command
     *
     * \returns TRUE if the undo/redo command is well added to the stack, FALSE otherwise
     *
     * \since QGIS 3.0
     */
    bool update( QgsTransaction *transaction, const QString &sql, const QString &name );

  private:
    bool mModified;

    // utility function to avoid cpy/paste
    bool modify( QgsVectorLayerUndoPassthroughCommand *cmd );

};

#endif // QGSVECTORLAYEREDITPASSTHROUGH_H
