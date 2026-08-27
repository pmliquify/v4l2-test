/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Peter Martienssen
 */

#pragma once

#include <runners/basicstreamrunner.hpp>


class StreamRunner : public BasicStreamRunner
{
public:
        virtual void printArgs();
};