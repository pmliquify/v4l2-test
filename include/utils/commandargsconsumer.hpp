/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Peter Martienssen
 */

#pragma once

#include <utils/commandargs.hpp>


class CommandArgsConsumer
{
public:
        virtual void printArgs() = 0;
        virtual int setup(CommandArgs &args) = 0;

protected:
        virtual void printArgSection(std::string section);
        virtual void printArg(std::string key, std::string desciption);
};