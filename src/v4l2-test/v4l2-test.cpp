#include <map>
#include "streamtest.hpp"
#include "isptest.hpp"
#include "noisetest.hpp"

typedef std::map<std::string, V4L2Test *> TestMap;


bool printHelp(CommandArgs &args, TestMap &testMap, V4L2ImageSource &imageSource, V4L2Image &image)
{
        if (args.exists("--help")) {
                printf("Usage: v4l2-test [OPTION...]   (v0.2.0)\n");
                printf("\n");
                printf("Options:\n");
                printf("  --help                Show this help\n");
                printf("\n");
                printf("Test options:\n");
                for (auto it = testMap.begin(); it != testMap.end(); ++it) {
                        printf("  %s\n", it->first.c_str());
                }
                imageSource.printArgs();
                image.printArgs();
                for (auto it = testMap.begin(); it != testMap.end(); ++it) {
                        if (args.exists(it->first)) {
                                V4L2Test *test = it->second;
                                test->printArgs();
                                break;
                        }
                }
                printf("\n");
                return true;
        }
        return false;
}

void createTests(TestMap &testMap)
{
        testMap["--stream"] = new StreamTest();
        testMap["--isp"] = new IspTest();
        testMap["--noise"] = new NoiseTest();
}

void executeTest(CommandArgs &args, TestMap &testMap, V4L2ImageSource &imageSource, V4L2Image &image)
{
        for (auto it = testMap.begin(); it != testMap.end(); ++it) {
                if (args.exists(it->first)) {
                        V4L2Test *test = it->second;
                        test->setup(args);
                        test->exec(imageSource, image);
                }
        }
}

void deleteTests(TestMap &testMap)
{
        for (auto it = testMap.begin(); it != testMap.end(); ++it) {
                delete it->second;
        }
}

int main(int argc, const char *argv[])
{
        CommandArgs args(argc, argv);
        V4L2ImageSource imageSource;
        V4L2Image image;
        TestMap testMap;
        
        createTests(testMap);
        
        if (!printHelp(args, testMap, imageSource, image)) {
                imageSource.setup(args);
                image.setup(args);
                executeTest(args, testMap, imageSource, image);
                imageSource.close();
        }

        deleteTests(testMap);

        return 0;
}