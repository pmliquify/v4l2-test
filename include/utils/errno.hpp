/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Peter Martienssen
 */

#pragma once

const char * genericErrors(int err);
const char * errorsForOpen(int err);
const char * errorsForClose(int err);
const char * errorsForSelect(int err);
const char * errorsForIoctl(unsigned long int request, int err);
const char * errorsForRecv(int err);