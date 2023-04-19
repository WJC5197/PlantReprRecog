#ifndef _COMMUNICATE_HPP_
#define _COMMUNICATE_HPP_

#define _ORANGE_PI_ 1

#if _ORANGE_PI_
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include <wiringPi.h>
#include <wiringSerial.h>

int openSerial(int &fd, const char s[], int baud)
{
  if ((fd = serialOpen(s, baud)) < 0)
  {
    fprintf(stdout, "Unable to open serial device: %s\n", strerror(errno));
    return 1;
  }
  if (wiringPiSetup() == -1)
  {
    fprintf(stdout, "Unable to start wiringPi: %s\n", strerror(errno));
    return 1;
  }
  return 0;
}

void serialSend(int fd, const char frameHeader[], int numContent)
{
  char c[255];
  serialPuts(fd, frameHeader);
  serialPutchar(fd, 13);
  serialPutchar(fd, 10);
  sleep(1);
  sprintf(c, "%d", numContent);
  serialPuts(fd, c);
  serialPutchar(fd, 13);
  serialPutchar(fd, 10);
}
#endif

#endif
