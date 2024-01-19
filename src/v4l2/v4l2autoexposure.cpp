#include <v4l2autoexposure.hpp>
#include <time.h>


V4L2AutoExposure::V4L2AutoExposure(V4L2ImageSource *imageSource) :
        m_test(false),
        m_sub(1),
        m_aep(1),
        m_aed(0),
        m_aeTarget(100),
        m_exposureMin(0),
        m_exposureMax(10000),
        m_imageSource(imageSource),
        m_exposure(0),
        m_meanLast(0),
        m_testState(TestStateInit),
        m_testCount(0),
        m_meanMin(0),
        m_meanMax(0)
{
        init(10000);
}

int V4L2AutoExposure::init(u_int32_t exposure)
{
        m_exposure = exposure;
        m_meanLast = 0;

        return 0;
}

int V4L2AutoExposure::exec(V4L2Image &image)
{
        if (m_imageSource == NULL) {
                return -1;
        }

        if (m_test) {
                return testLatency(image);
        } 

        return calculateExposure(image);
}

int V4L2AutoExposure::calculateExposure(V4L2Image &image)
{
        u_int16_t rep = 1;
        clock_t startTime = clock();

        u_int16_t min, max, mean;
        for (int i=0; i<rep; i++) {
                image.stats(min, max, mean, m_sub);
        }
        clock_t elapsedTime = clock() - startTime;
        elapsedTime /= rep;

        if (m_meanLast == 0) {
                m_meanLast = mean;
        }
        int32_t exposureP = ((int16_t)m_aeTarget - mean)*m_aep;
        int32_t exposureD = ((int16_t)mean - m_meanLast)*m_aed;
        m_exposure += exposureP + exposureD;
        m_imageSource->setExposure(m_exposure);

        printf("Stats (min: %4u, max: %4u; mean: %4u) => AE (p: %d/%d, d: %d/%d, exp: %u, elapsed: %ld us)\n",
                min, max, mean, m_aep, exposureP, m_aed, exposureD, m_exposure, elapsedTime);

        m_meanLast = mean;

        return 0;
}

int V4L2AutoExposure::testLatency(V4L2Image &image)
{
        u_int16_t min, max, mean;
        image.stats(min, max, mean, m_sub);

        switch(m_testState) {
        case TestStateInit:
                m_initCount = 0;
                m_testCount = 0;
                m_measureCount = 0;
                m_meanMin = mean;
                m_meanMax = mean;
                m_imageSource->setExposure(m_exposureMax);
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
                        m_imageSource->setExposure(m_exposureMax);
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
                        m_imageSource->setExposure(m_exposureMin);
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
                        m_imageSource->setExposure(m_exposureMax);
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
                        m_imageSource->setExposure(m_exposureMin);
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