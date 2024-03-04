#include <sources/autoexposure.hpp>
#include <cv/imagestats.hpp>
#include <time.h>


AutoExposure::AutoExposure() :
        m_active(false),
        m_test(false),
        m_sub(1),
        m_p(1),
        m_d(0),
        m_target(100),
        m_exposureMin(0),
        m_exposureMax(10000),
        m_exposure(0),
        m_meanLast(0),
        m_testState(TestStateInit),
        m_testCount(0),
        m_meanMin(0),
        m_meanMax(0)
{
        init(10000);
}

int AutoExposure::init(u_int32_t exposure)
{
        m_exposure = exposure;
        m_meanLast = 0;

        return 0;
}

int AutoExposure::run(ImageSource *imageSource, Image *image)
{
        if (!m_active) {
                return 0;
        }

        if (imageSource == NULL) {
                return -1;
        }

        if (m_test) {
                return testLatency(imageSource, image);
        } 

        return calculateExposure(imageSource, image);
}

int AutoExposure::calculateExposure(ImageSource *imageSource, Image *image)
{
        u_int16_t rep = 1;
        clock_t startTime = clock();

        u_int16_t min, max, mean;
        for (int i=0; i<rep; i++) {
                ImageStats::stats(image, min, max, mean, m_sub);
        }
        clock_t elapsedTime = clock() - startTime;
        elapsedTime /= rep;

        if (m_meanLast == 0) {
                m_meanLast = mean;
        }
        int32_t exposureP = ((int16_t)m_target - mean)*m_p;
        int32_t exposureD = ((int16_t)mean - m_meanLast)*m_d;
        m_exposure += exposureP + exposureD;
        imageSource->setExposure(m_exposure);

        printf("Stats (min: %4u, max: %4u; mean: %4u) => AE (p: %d/%d, d: %d/%d, exp: %u, elapsed: %ld us)\n",
                min, max, mean, m_p, exposureP, m_d, exposureD, m_exposure, elapsedTime);

        m_meanLast = mean;

        return 0;
}

int AutoExposure::testLatency(ImageSource *imageSource, Image *image)
{
        u_int16_t min, max, mean;
        ImageStats::stats(image, min, max, mean, m_sub);

        switch(m_testState) {
        case TestStateInit:
                m_initCount = 0;
                m_testCount = 0;
                m_measureCount = 0;
                m_meanMin = mean;
                m_meanMax = mean;
                imageSource->setExposure(m_exposureMax);
                m_testState = TestStateInitMax;
                printf("Init AE latency test ");
                fflush(stdout);
                break;

        case TestStateInitMin:
                printf(">");
                fflush(stdout);
                if (mean < m_meanMin) {
                        m_meanMin = mean;
                        m_testCount = 0;
                } else {
                        m_testCount++;
                }
                if (m_testCount == 3) {
                        m_testCount = 0;
                        m_meanMax = mean;
                        imageSource->setExposure(m_exposureMax);
                        m_testState = TestStateInitMax;
                }
                break;

        case TestStateInitMax:
                printf(">");
                fflush(stdout);
                if (mean > m_meanMax) {
                        m_meanMax = mean;
                        m_testCount = 0;
                } else {
                        m_testCount++;
                }
                if (m_testCount == 3) {
                        m_testCount = 0;
                        imageSource->setExposure(m_exposureMin);
                        m_initCount++;
                        if (m_initCount < 2) {
                                m_meanMin = mean;
                                m_testState = TestStateInitMin;
                        } else {
                                m_testState = TestStateMeasureMin;
                                printf(" (min: %u, max: %u)\n", m_meanMin, m_meanMax);
                        }
                }
                break;

        case TestStateMeasureMin:
                printf("v");
                fflush(stdout);
                if (abs(mean - m_meanMin) < 0.1*m_meanMin+1) {
                        m_latency = (m_measureCount * m_latency + m_testCount)/(m_measureCount + 1);
                        m_measureCount++;
                        printf(" AE Latency: %.2f\n", m_latency);
                        m_testCount = 0;
                        imageSource->setExposure(m_exposureMax);
                        m_testState = TestStateMeasureMax;
                } else {
                        m_testCount++;
                }
                break;

        case TestStateMeasureMax:
                printf("^");
                fflush(stdout);
                if (abs(mean - m_meanMax) < 0.1*m_meanMax+1) {
                        m_latency = (m_measureCount * m_latency + m_testCount)/(m_measureCount + 1);
                        m_measureCount++;
                        printf(" AE Latency: %.2f\n", m_latency);
                        m_testCount = 0;
                        imageSource->setExposure(m_exposureMin);
                        m_testState = TestStateMeasureMin;
                } else {
                        m_testCount++;
                }
                break;
        }

        if (m_testCount > 20) {
                m_testState = TestStateInit;
        }

        return 0;
}