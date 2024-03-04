#include <version.h>
#include <utils/commandargs.hpp>
#include <sources/v4l2imagesource.hpp>
#include <sources/socketserversource.hpp>
#include <runners/streamrunner.hpp>
#include <runners/isprunner.hpp>
#include <runners/noisetestrunner.hpp>
#include <runners/socketclientrunner.hpp>


bool printHelp(CommandArgs &args, ImageSourceRunnerMap &runnerMap, ImageSource *imageSource)
{
        if (args.exists("--help")) {
                printf("Usage: v4l2-test [OPTION...]\n");
                printf("v%s  (Build: %s / %s)\n", V4L2TEST_VERSION, __TIME__, __DATE__);
                printf("\n");
                printf("Options:\n");
                printf("  --help                Show this help\n");
                printf("\n");
                printf("Runner options:\n");
                for (auto it = runnerMap.begin(); it != runnerMap.end(); ++it) {
                        printf("  %s\n", it->first.c_str());
                }
                imageSource->printArgs();
                for (auto it = runnerMap.begin(); it != runnerMap.end(); ++it) {
                        if (args.exists(it->first)) {
                                ImageSourceRunner *runner = it->second;
                                runner->printArgs();
                                break;
                        }
                }
                printf("\n");
                return true;
        }
        return false;
}

ImageSource *createImageSource(CommandArgs &args)
{
        if (args.exists("server")) {
                return new SocketServerSource();
        }
        return new V4L2ImageSource();
}

void createRunners(ImageSourceRunnerMap &runnerMap)
{
        runnerMap["stream"] = new StreamRunner();
        runnerMap["isp"] = new IspRunner();
        runnerMap["noise"] = new NoiseTestRunner();
        runnerMap["client"] = new SocketClientRunner();
}

void runRunner(CommandArgs &args, ImageSourceRunnerMap &runnerMap, ImageSource *imageSource)
{
        for (auto it = runnerMap.begin(); it != runnerMap.end(); ++it) {
                if (args.exists(it->first)) {
                        ImageSourceRunner *runner = it->second;
                        runner->setup(args);
                        runner->run(imageSource);
                }
        }
}

void deleteRunners(ImageSourceRunnerMap &runnerMap)
{
        for (auto it = runnerMap.begin(); it != runnerMap.end(); ++it) {
                delete it->second;
        }
        runnerMap.clear();
}

int main(int argc, const char *argv[])
{
        CommandArgs args(argc, argv);

        ImageSource *imageSource = createImageSource(args);

        ImageSourceRunnerMap runnerMap;
        createRunners(runnerMap);
        
        if (!printHelp(args, runnerMap, imageSource)) {
                imageSource->setup(args);
                runRunner(args, runnerMap, imageSource);
                imageSource->close();
        }

        deleteRunners(runnerMap);
        delete imageSource;

        return 0;
}