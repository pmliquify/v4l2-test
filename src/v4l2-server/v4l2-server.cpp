#include <commandargs.hpp>
#include <v4l2imagesocket.hpp>

void printHelp()
{
        printf("  Usage: v4l2-server [OPTION...]\n");
        printf("\n");
        printf("  --help,    Show this help\n");
        printf("  --port,    \n");
}

int main(int argc, const char *argv[])
{
        CommandArgs args(argc, argv);

        if (args.exists("--help")) {
                printHelp();
                return 0;
        }
        
        int x               = args.optionInt("-x", -1);
        int y               = args.optionInt("-y", -1);
        int print           = args.optionInt("-p", 16);
        int port            = args.optionInt("--port", 9000);

        V4L2Image image;
        V4L2ImageSocket socket;
        socket.listen(port);

        while (true) {
                if (0 == socket.receive(image)) {
                        image.print(print, x, y);
                }
        }

        return 0;
}