#include <version.h>
#include <utils/commandargs.hpp>
#include <sources/v4l2imagesource.hpp>
#include <sources/socketserversource.hpp>
#include <runners/streamrunner.hpp>
#include <runners/isprunner.hpp>
#include <runners/gstreamerRunner.hpp>
#include <runners/imagesaverrunner.hpp>
#include <runners/videoRunner.hpp>

#include <runners/noisetestrunner.hpp>
#include <runners/socketclientrunner.hpp>
#include <csignal> // for signal handling

ImageSourceRunnerMap runnerMap;

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
        runnerMap["gstreamer"] = new GstreamerRunner();
        runnerMap["imagesaver"] = new ImageSaverRunner();
        runnerMap["video"] = new VideoRunner();


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
                it->second->closeRunner();
                delete it->second;
        }
        runnerMap.clear();
}

void signalHandler(int signum)
{
        deleteRunners(runnerMap);
        sleep(1);
        exit(signum);
}

int main(int argc, const char *argv[])
{
        // Register signal handler for Ctrl-C
        signal(SIGINT, signalHandler);

        CommandArgs args(argc, argv);

        ImageSource *imageSource = createImageSource(args);

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