/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Peter Martienssen
 */

#pragma once

#include "window.hpp"


class CCMWindow : public Window
{
public:
    CCMWindow(const char *name);

    virtual void show(Mat img);
};