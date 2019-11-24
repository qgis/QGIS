/***************************************************************************
  qgsabstractwebserviceproviderconnection.h - QgsAbstractWebServiceProviderConnection

 ---------------------
 begin                : 21.11.2019
 copyright            : (C) 2019 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSABSTRACTWEBSERVICEPROVIDERCONNECTION_H
#define QGSABSTRACTWEBSERVICEPROVIDERCONNECTION_H

#include "qgsabstractproviderconnection.h"
#include "qgis_core.h"
#include "qgsexception.h"
#include "qgscoordinatereferencesystem.h"

#include <QObject>

/**
 * The QgsAbstractWebServiceProviderConnection class provides common functionality
 * for web service based connections (OGC, ESRI etc.).
 *
 * Web services typically have nested groups of layers, layers can be retrieved
 * as a flat list or as a tree-like structure (TODO).
 *
 * The class methods will throw exceptions in case the requested operation
 * is not supported or cannot be performed without errors.
 *
 * \ingroup core
 * \since QGIS 3.12
 */
class CORE_EXPORT QgsAbstractWebServiceProviderConnection : public QgsAbstractProviderConnection
{

    Q_GADGET

  public:

    /**
     * Flags for layer properties.
     *
     * Flags can be useful for filtering the layers returned
     * from layers().
     */
    enum LayerFlag
    {
      Aspatial = 1 << 1,          //!< Aspatial layer (it does not contain any geometry column)
      Vector = 1 << 2,            //!< Vector layer (it does contain one geometry column)
      Raster = 1 << 3,            //!< Raster layer
      Group =  1 << 4,            //!< A group of layers
    };

    Q_ENUMS( LayerFlag )
    Q_DECLARE_FLAGS( LayerFlags, LayerFlag )
    Q_FLAG( LayerFlags )

    /**
     * The LayerProperty class represents a web service layer
     */
    struct CORE_EXPORT LayerProperty
    {

#ifdef SIP_RUN
        SIP_PYOBJECT __repr__();
        % MethodCode
        QString str = QStringLiteral( "<QgsAbstractWebServiceProviderConnection.LayerProperty: '%1'>" ).arg( sipCpp->layerName() );
        sipRes = PyUnicode_FromString( str.toUtf8().constData() );
        % End
#endif

      public:

        /**
         * Returns the layer name
         */
        QString layerName() const;

        /**
         * Sets the layer name to \a name
         */
        void setLayerName( const QString &name );

        /**
         * Returns the layer flags
         */
        LayerFlags flags() const;

        /**
         * Sets the layer \a flags
         */
        void setFlags( const LayerFlags &flags );

        /**
         * Returns the layer comment
         */
        QString comment() const;

        /**
         * Sets the layer \a comment
         */
        void setComment( const QString &comment );

        /**
         * Returns additional information about the layer
         *
         * Provider classes may use this property
         * to store custom bits of information.
         */
        QVariantMap info() const;

        /**
         * Sets additional information about the layer to \a info
         *
         * Provider classes may use this property
         * to store custom bits of information.
         */
        void setInfo( const QVariantMap &info );

        /**
         * Sets a \a flag
         */
        void setFlag( const LayerFlag &flag );

        /**
         * Returns layer CRS
         */
        QgsCoordinateReferenceSystem crs() const;

        /**
         * Sets layer CRS to \a crs
         */
        void setCrs( const QgsCoordinateReferenceSystem &crs );

        /**
         * Returns layer WKB type
         */
        QgsWkbTypes::Type wkbType() const;

        /**
         * Sets layer WKB type to \a wkbType
         */
        void setWkbType( const QgsWkbTypes::Type &wkbType );

      private:

        //! Layer name
        QString                       mLayerName;
        LayerFlags                    mFlags;
        QString                       mComment;
        //! Additional unstructured information about the table
        QVariantMap                   mInfo;
        QgsCoordinateReferenceSystem  mCrs;
        QgsWkbTypes::Type             mWkbType;
    };

    /**
     * The Capability enum represent the operations supported by the connection
     */
    enum Capability
    {
      Layers = 1 << 1,             //!< Can list layers
      LayerExists = 1 << 2,        //!< Can check if layer exists
      Spatial = 1 << 3,            //!< The connection supports spatial layers
    };

    Q_ENUM( Capability )
    Q_DECLARE_FLAGS( Capabilities, Capability )
    Q_FLAG( Capabilities )


    /**
     * Creates a new connection with \a name by reading its configuration from the settings.
     * If a connection with this name cannot be found, an empty connection will be returned.
     */
    QgsAbstractWebServiceProviderConnection( const QString &name );

    /**
     * Creates a new connection from the given \a uri and \a configuration.
     * The connection is not automatically stored in the settings.
     * \see store()
     */
    QgsAbstractWebServiceProviderConnection( const QString &uri, const QVariantMap &configuration );

    // Public interface

    /**
     * Returns connection capabilities
     */
    Capabilities capabilities() const;

    // Operations interface

    /**
     * Returns the URI string for the given \a layerName
     * Raises a QgsProviderConnectionException if any errors are encountered.
     * \throws QgsProviderConnectionException
     */
    virtual QString layerUri( const QString &layerName ) const SIP_THROW( QgsProviderConnectionException );

    /**
     * Returns information on the layers as a flat list of layer properties.
     * Raises a QgsProviderConnectionException if any errors are encountered.
     * \param flags filter layers by flags, this option completely overrides search options stored in the connection
     * \throws QgsProviderConnectionException
     * \note Not available in Python bindings
     */
    virtual QList<QgsAbstractWebServiceProviderConnection::LayerProperty> layers( const QgsAbstractWebServiceProviderConnection::LayerFlags &flags = nullptr ) const SIP_SKIP;

    /**
     * Returns information on a layer with name \a layerName
     * Raises a QgsProviderConnectionException if any errors are encountered or if the layer does not exist.
     * \throws QgsProviderConnectionException
     */
    virtual QgsAbstractWebServiceProviderConnection::LayerProperty layer( const QString &layerName ) const;

    /**
     * Returns information on the layers
     * Raises a QgsProviderConnectionException if any errors are encountered.
     * \param flags filter tables by flags, this option completely overrides search options stored in the connection
     * \throws QgsProviderConnectionException
     */
    QList<QgsAbstractWebServiceProviderConnection::LayerProperty> layersInt( const int flags = 0 ) const SIP_THROW( QgsProviderConnectionException ) SIP_PYNAME( layers );

    /**
     * Checks whether a layer \a layerName exists
     * Raises a QgsProviderConnectionException if any errors are encountered.
     * \throws QgsProviderConnectionException
     */
    virtual bool layerExists( const QString &layerName ) const SIP_THROW( QgsProviderConnectionException );

    // QgsAbstractProviderConnection interface
    void store( const QString &name ) const override;
    void remove( const QString &name ) const override;

  protected:

///@cond PRIVATE

    /**
     * Checks if \a capability is supported and throws and exception if it's not
     * \throws QgsProviderConnectionException
     */
    void checkCapability( Capability capability ) const;
///@endcond

    Capabilities mCapabilities = nullptr SIP_SKIP;
    QString mServiceName;

};

#endif // QGSABSTRACTWEBSERVICEPROVIDERCONNECTION_H
