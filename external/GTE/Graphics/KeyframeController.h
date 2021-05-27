// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/TransformController.h>
#include <Mathematics/Quaternion.h>

namespace gte
{
    class KeyframeController : public TransformController
    {
    public:
        // Construction and destruction.  If the translations, rotations, and
        // scales all share the same keyframe times, then numCommonTimes is
        // set to a positive number.  Each remaining number is numCommonTimes
        // when the channel exists or zero when it does not.  If the keyframe
        // times are not shared, then numCommonTimes must be set to zero and
        // the remaining numbers set to the appropriate values--positive when
        // the channel exists or zero otherwise.
        //
        // The Transform input initializes the controlled object's local
        // transform.  The previous behavior of this class was to fill in only
        // those transformation channels represented by the key frames, which
        // relied implicitly on the Spatial mObject to have its other channels
        // set appropriately by the application.  Now KeyframeController sets
        // *all* the channels.
        virtual ~KeyframeController() = default;
        KeyframeController(int numCommonTimes, int numTranslations,
            int numRotations, int numScales, Transform<float> const& localTransform);

        // Member access.  After calling the constructor, you must set the
        // data using these functions.
        inline int GetNumCommonTimes() const
        {
            return mNumCommonTimes;
        }

        inline float* GetCommonTimes()
        {
            return mCommonTimes.data();
        }

        inline int GetNumTranslations() const
        {
            return mNumTranslations;
        }

        inline float* GetTranslationTimes()
        {
            return mTranslationTimes.data();
        }

        inline Vector4<float>* GetTranslations()
        {
            return mTranslations.data();
        }

        inline int GetNumRotations() const
        {
            return mNumRotations;
        }

        inline float* GetRotationTimes()
        {
            return mRotationTimes.data();
        }

        inline Quaternion<float>* GetRotations()
        {
            return mRotations.data();
        }

        inline int GetNumScales() const
        {
            return mNumScales;
        }

        inline float* GetScaleTimes()
        {
            return mScaleTimes.data();
        }

        inline float* GetScales()
        {
            return mScales.data();
        }

        // The animation update.  The application time is in milliseconds.
        virtual bool Update(double applicationTime) override;

    protected:
        // Support for looking up keyframes given the specified time.
        static void GetKeyInfo(float ctrlTime, int numTimes, float* times,
            int& lastIndex, float& normTime, int& i0, int& i1);

        Vector4<float> GetTranslate(float normTime, int i0, int i1);
        Matrix4x4<float> GetRotate(float normTime, int i0, int i1);
        float GetScale(float normTime, int i0, int i1);

        // This array is used only when times are shared by translations,
        // rotations, and scales.
        int mNumCommonTimes;
        std::vector<float> mCommonTimes;

        int mNumTranslations;
        std::vector<float> mTranslationTimes;
        std::vector<Vector4<float>> mTranslations;

        int mNumRotations;
        std::vector<float> mRotationTimes;
        std::vector<Quaternion<float>> mRotations;

        int mNumScales;
        std::vector<float> mScaleTimes;
        std::vector<float> mScales;

        // Cached indices for the last found pair of keys used for
        // interpolation.  For a sequence of times, this guarantees an
        // O(1) lookup.
        int mTLastIndex, mRLastIndex, mSLastIndex, mCLastIndex;
    };
}
