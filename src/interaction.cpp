#include "communicate.hpp"
#include <errno.h>
#include <unistd.h>

int main()
{
    int fd;
    if ((fd = serialOpen ("/dev/ttyS0", 115200)) < 0)
    {
        fprintf (stdout, "Unable to open serial device: %s\n", strerror (errno));
        return 1;
    }
    if (wiringPiSetup () == -1)
    {
        fprintf (stdout, "Unable to start wiringPi: %s\n", strerror (errno));
        return 1;
    }
    // printf("open ok\n");
    // sendDis(fd, 50);
    // printf("send ok\n");
    // sleep(1);
    // sendDis(fd, 50);
    // printf("send ok\n");
    // sleep(1);
    serialSend(fd, "speed", 400);
    sleep(1);
    while(1)
    {
        // serialSend(fd, "up", 400);
        // printf("send ok\n");
        // sleep(1);
        // serialSend(fd, "down", 400);
        // printf("send ok\n");
        // sleep(1);
        serialSend(fd, "distance", 100);
        printf("send ok\n");
        sleep(3);

        // step test
        // sendStepLift(fd, true, 400);
        // sleep(1);
        // printf("send up\n");
        // printf("send 400\n");
        // sendStepLift(fd, false, 400);
        // sleep(1);
        // printf("send down\n");
        // printf("send 400\n");
        // dis test
    }
    return 0;
}