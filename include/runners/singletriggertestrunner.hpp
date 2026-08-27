/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Peter Martienssen
 */

#pragma once

#include <runners/socketclientrunner.hpp>
#include <cstdint>


class SingleTriggerTestRunner : public SocketClientRunner
{
public:
        SingleTriggerTestRunner();

        void printArgs();
        int setup(CommandArgs &args);

protected:
        virtual void prepareNextImage(ImageSource *imageSource);
        virtual int processImage(ImageSource *imageSource, Image *image);

private:
        uint64_t m_executionTime;
        uint64_t m_startTime;
};
