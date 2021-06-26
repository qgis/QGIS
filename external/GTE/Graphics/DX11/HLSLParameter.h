// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/DX11/HLSLResource.h>

namespace gte
{
    class HLSLParameter
    {
    public:
        struct Description
        {
            std::string semanticName;
            unsigned int semanticIndex;
            unsigned int registerIndex;
            D3D_NAME systemValueType;
            D3D_REGISTER_COMPONENT_TYPE componentType;
            unsigned int mask;
            unsigned int readWriteMask;
            unsigned int stream;
            D3D_MIN_PRECISION minPrecision;
        };

        // Construction.  Parameters are reported for inputs, outputs and
        // patch constants.
        HLSLParameter(D3D_SIGNATURE_PARAMETER_DESC const& desc);

        // Member access.
        inline std::string const& GetSemanticName() const
        {
            return mDesc.semanticName;
        }

        inline unsigned int GetSemanticIndex() const
        {
            return mDesc.semanticIndex;
        }

        inline unsigned int GetRegisterIndex() const
        {
            return mDesc.registerIndex;
        }

        inline D3D_NAME GetSystemValueType() const
        {
            return mDesc.systemValueType;
        }

        inline D3D_REGISTER_COMPONENT_TYPE GetComponentType() const
        {
            return mDesc.componentType;
        }

        inline unsigned int GetMask() const
        {
            return mDesc.mask;
        }

        inline unsigned int GetReadWriteMask() const
        {
            return mDesc.readWriteMask;
        }

        inline unsigned int GetStream() const
        {
            return mDesc.stream;
        }

        inline D3D_MIN_PRECISION GetMinPrecision() const
        {
            return mDesc.minPrecision;
        }

        // Print to a text file for human readability.
        void Print(std::ofstream& output) const;

    private:
        Description mDesc;

        // Support for Print.
        static std::string const msSVName[];
        static std::string const msComponentType[];
        static std::string const msMinPrecision[];
    };
}
