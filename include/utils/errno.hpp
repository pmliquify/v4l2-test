#pragma once

const char * genericErrors(int err);
const char * errorsForOpen(int err);
const char * errorsForClose(int err);
const char * errorsForSelect(int err);
const char * errorsForIoctl(unsigned long int request, int err);
const char * errorsForRecv(int err);