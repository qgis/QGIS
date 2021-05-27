// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <algorithm>
#include <numeric>
#include <random>
#include <vector>

// Base class support for least-squares fitting algorithms and for RANSAC
// algorithms.

// Expose this define if you want the code to verify that the incoming
// indices to the fitting functions are valid.
#define GTE_APPR_QUERY_VALIDATE_INDICES

namespace gte
{
    template <typename Real, typename ObservationType>
    class ApprQuery
    {
    public:
        // Construction and destruction.
        ApprQuery() = default;
        virtual ~ApprQuery() = default;

        // The base-class Fit* functions are generic but need to call the
        // indexed fitting function for the specific derived class.
        virtual bool FitIndexed(
            size_t numObservations, ObservationType const* observations,
            size_t numIndices, int const* indices) = 0;

        bool ValidIndices(
            size_t numObservations, ObservationType const* observations,
            size_t numIndices, int const* indices)
        {
#if defined(GTE_APPR_QUERY_VALIDATE_INDICES)
            if (observations && indices &&
                GetMinimumRequired() <= numIndices && numIndices <= numObservations)
            {
                int const* currentIndex = indices;
                for (size_t i = 0; i < numIndices; ++i)
                {
                    if (*currentIndex++ >= static_cast<int>(numObservations))
                    {
                        return false;
                    }
                }
                return true;
            }
            return false;
#else
            // The caller is responsible for passing correctly formed data.
            (void)numObservations;
            (void)observations;
            (void)numIndices;
            (void)indices;
            return true;
#endif
        }

        // Estimate the model parameters for all observations passed in via
        // raw pointers.
        bool Fit(size_t numObservations, ObservationType const* observations)
        {
            std::vector<int> indices(numObservations);
            std::iota(indices.begin(), indices.end(), 0);
            return FitIndexed(numObservations, observations, indices.size(), indices.data());
        }

        // Estimate the model parameters for all observations passed in via
        // std::vector.
        bool Fit(std::vector<ObservationType> const& observations)
        {
            std::vector<int> indices(observations.size());
            std::iota(indices.begin(), indices.end(), 0);
            return FitIndexed(observations.size(), observations.data(), indices.size(), indices.data());
        }

        // Estimate the model parameters for a contiguous subset of
        // observations.
        bool Fit(std::vector<ObservationType> const& observations, size_t imin, size_t imax)
        {
            if (imin <= imax)
            {
                size_t numIndices = static_cast<size_t>(imax - imin + 1);
                std::vector<int> indices(numIndices);
                std::iota(indices.begin(), indices.end(), static_cast<int>(imin));
                return FitIndexed(observations.size(), observations.data(), indices.size(), indices.data());
            }
            else
            {
                return false;
            }
        }

        // Estimate the model parameters for an indexed subset of observations.
        virtual bool Fit(std::vector<ObservationType> const& observations,
            std::vector<int> const& indices)
        {
            return FitIndexed(observations.size(), observations.data(), indices.size(), indices.data());
        }

        // Estimate the model parameters for the subset of observations
        // specified by the indices and the number of indices that is possibly
        // smaller than indices.size().
        bool Fit(std::vector<ObservationType> const& observations,
            std::vector<int> const& indices, size_t numIndices)
        {
            size_t imax = std::min(numIndices, indices.size());
            std::vector<int> localindices(imax);
            std::copy(indices.begin(), indices.begin() + imax, localindices.begin());
            return FitIndexed(observations.size(), observations.data(), localindices.size(), localindices.data());
        }


        // Apply the RANdom SAmple Consensus algorithm for fitting a model to
        // observations. The algorithm requires three virtual functions to be
        // implemented by the derived classes.

        // The minimum number of observations required to fit the model.
        virtual size_t GetMinimumRequired() const = 0;

        // Compute the model error for the specified observation for the
        // current model parameters.
        virtual Real Error(ObservationType const& observation) const = 0;

        // Copy the parameters between two models. This is used to copy the
        // candidate-model parameters to the current best-fit model.
        virtual void CopyParameters(ApprQuery const* input) = 0;

        static bool RANSAC(ApprQuery& candidateModel, std::vector<ObservationType> const& observations,
            size_t numRequiredForGoodFit, Real maxErrorForGoodFit, size_t numIterations,
            std::vector<int>& bestConsensus, ApprQuery& bestModel)
        {
            size_t const numObservations = observations.size();
            size_t const minRequired = candidateModel.GetMinimumRequired();
            if (numObservations < minRequired)
            {
                // Too few observations for model fitting.
                return false;
            }

            // The first part of the array will store the consensus set,
            // initially filled with the minimum number of indices that
            // correspond to the candidate inliers. The last part will store
            // the remaining indices. These points are tested against the
            // model and are added to the consensus set when they fit. All
            // the index manipulation is done in place. Initially, the
            // candidates are the identity permutation.
            std::vector<int> candidates(numObservations);
            std::iota(candidates.begin(), candidates.end(), 0);

            if (numObservations == minRequired)
            {
                // We have the minimum number of observations to generate the
                // model, so RANSAC cannot be used. Compute the model with the
                // entire set of observations.
                bestConsensus = candidates;
                return bestModel.Fit(observations);
            }

            size_t bestNumFittedObservations = minRequired;

            for (size_t i = 0; i < numIterations; ++i)
            {
                // Randomly permute the previous candidates, partitioning the
                // array into GetMinimumRequired() indices (the candidate
                // inliers) followed by the remaining indices (candidates for
                // testing against the model).
                std::shuffle(candidates.begin(), candidates.end(), std::default_random_engine());

                // Fit the model to the inliers.
                if (candidateModel.Fit(observations, candidates, minRequired))
                {
                    // Test each remaining observation whether it fits the
                    // model. If it does, include it in the consensus set.
                    size_t numFittedObservations = minRequired;
                    for (size_t j = minRequired; j < numObservations; ++j)
                    {
                        Real error = candidateModel.Error(observations[candidates[j]]);
                        if (error <= maxErrorForGoodFit)
                        {
                            std::swap(candidates[j], candidates[numFittedObservations]);
                            ++numFittedObservations;
                        }
                    }

                    if (numFittedObservations >= numRequiredForGoodFit)
                    {
                        // We have observations that fit the model. Update the
                        // best model using the consensus set.
                        candidateModel.Fit(observations, candidates, numFittedObservations);
                        if (numFittedObservations > bestNumFittedObservations)
                        {
                            // The consensus set is larger than the previous
                            // consensus set, so its model becomes the best one.
                            bestModel.CopyParameters(&candidateModel);
                            bestConsensus.resize(numFittedObservations);
                            std::copy(candidates.begin(), candidates.begin() + numFittedObservations, bestConsensus.begin());
                            bestNumFittedObservations = numFittedObservations;
                        }
                    }
                }
            }

            return bestNumFittedObservations >= numRequiredForGoodFit;
        }
    };
}
