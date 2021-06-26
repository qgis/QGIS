// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <mutex>
#include <set>
#include <stdexcept>
#include <string>

namespace gte
{
    class Logger
    {
    public:
        // Listeners subscribe to Logger to receive message strings.
        class Listener
        {
        public:
            enum
            {
                LISTEN_FOR_NOTHING = 0x00000000,
                LISTEN_FOR_ASSERTION = 0x00000001,
                LISTEN_FOR_ERROR = 0x00000002,
                LISTEN_FOR_WARNING = 0x00000004,
                LISTEN_FOR_INFORMATION = 0x00000008,
                LISTEN_FOR_ALL = 0xFFFFFFFF
            };

            // Construction and destruction.
            virtual ~Listener() = default;

            Listener(int flags = LISTEN_FOR_NOTHING)
                :
                mFlags(flags)
            {
            }

            // What the listener wants to hear.
            inline int GetFlags() const
            {
                return mFlags;
            }

            // Handlers for the messages received from the logger.
            void Assertion(std::string const& message)
            {
                Report("\nGTE ASSERTION:\n" + message);
            }

            void Error(std::string const& message)
            {
                Report("\nGTE ERROR:\n" + message);
            }

            void Warning(std::string const& message)
            {
                Report("\nGTE WARNING:\n" + message);
            }

            void Information(std::string const& message)
            {
                Report("\nGTE INFORMATION:\n" + message);
            }

        private:
            virtual void Report(std::string const& message)
            {
                // Stub for derived classes.
                (void)message;
            }

            int mFlags;
        };

        // Construction.  The Logger object is designed to exist only for a
        // single-line call.  A string is generated from the input parameters and
        // is used for reporting.
        Logger(char const* file, char const* function, int line, std::string const& message)
        {
            mMessage =
                "File: " + std::string(file) + "\n" +
                "Func: " + std::string(function) + "\n" +
                "Line: " + std::to_string(line) + "\n" +
                message + "\n\n";
        }

        // Notify current listeners about the logged information.
        void Assertion()
        {
            Mutex().lock();
            for (auto listener : Listeners())
            {
                if (listener->GetFlags() & Listener::LISTEN_FOR_ASSERTION)
                {
                    listener->Assertion(mMessage);
                }
            }
            Mutex().unlock();
        }

        void Error()
        {
            Mutex().lock();
            for (auto listener : Listeners())
            {
                if (listener->GetFlags() & Listener::LISTEN_FOR_ERROR)
                {
                    listener->Error(mMessage);
                }
            }
            Mutex().unlock();
        }

        void Warning()
        {
            Mutex().lock();
            for (auto listener : Listeners())
            {
                if (listener->GetFlags() & Listener::LISTEN_FOR_WARNING)
                {
                    listener->Warning(mMessage);
                }
            }
            Mutex().unlock();
        }
        void Information()
        {
            Mutex().lock();
            for (auto listener : Listeners())
            {
                if (listener->GetFlags() & Listener::LISTEN_FOR_INFORMATION)
                {
                    listener->Information(mMessage);
                }
            }
            Mutex().unlock();
        }

        static void Subscribe(Listener* listener)
        {
            Mutex().lock();
            Listeners().insert(listener);
            Mutex().unlock();
        }

        static void Unsubscribe(Listener* listener)
        {
            Mutex().lock();
            Listeners().erase(listener);
            Mutex().unlock();
        }


    private:
        std::string mMessage;

        static std::mutex& Mutex()
        {
            static std::mutex sMutex;
            return sMutex;
        }

        static std::set<Listener*>& Listeners()
        {
            static std::set<Listener*> sListeners;
            return sListeners;
        }
    };

}


#define LogAssert(condition, message) \
    if (!(condition)) \
    { \
        gte::Logger(__FILE__, __FUNCTION__, __LINE__, message).Assertion(); \
        throw std::runtime_error(message); \
    }

#define LogError(message) \
    gte::Logger(__FILE__, __FUNCTION__, __LINE__, message).Error(); \
    throw std::runtime_error(message)

#define LogWarning(message) \
    gte::Logger(__FILE__, __FUNCTION__, __LINE__, message).Warning()

#define LogInformation(message) \
    gte::Logger(__FILE__, __FUNCTION__, __LINE__, message).Information()
