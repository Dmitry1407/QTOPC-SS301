#undef UNICODE
#define _WIN32_WINNT 0x0501
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#include <time.h>

#include "Server.h"

// Need to link with Ws2_32.lib
//#pragma comment (lib, "Ws2_32.lib")
//#pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
//#define DEFAULT_ADDR "192.168.1.11"
#define DEFAULT_ADDR "127.0.0.1"
#define DEFAULT_PORT "4002"
//#define DEFAULT_PORT "4012"

int __cdecl main(void)
{
    int numByte;
    int iParam=2500;
    int NumPort = 5;    //Номер последовательного порта
    time_t t;
    struct tm *t_m;
    COMMTIMEOUTS ct;
    uint8_t szIN[256]="",  szOUT[256]="";

    WSADATA wsaData;
    int iResult;
    SOCKET ListenSocket, ClientSocket;

    struct addrinfo *result = NULL;
    struct addrinfo hints;

    int iSendResult;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    while (1) {

    ListenSocket = INVALID_SOCKET;
    ClientSocket = INVALID_SOCKET;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return -1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = PF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    //iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    iResult = getaddrinfo(DEFAULT_ADDR, DEFAULT_PORT, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return -1;
    }

    // Create a SOCKET for connecting to server
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return -1;
    }

    // Setup the TCP listening socket
    iResult = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return -1;
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return -1;
    }

    // Accept a client socket
    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return -1;
    }

    // No longer need server socket
    closesocket(ListenSocket);

    // Receive until the peer shuts down the connection
    do {

        iResult = recv(ClientSocket, (char *)szIN, 8, 0);
        if (iResult > 0) {

					time (&t);
					t_m = localtime(&t);

					printf("%02d:%02d:%02d - Bytes received: %d\n", t_m->tm_hour, t_m->tm_min, t_m->tm_sec, iResult);

					printf("IN: %02X", szIN[0]);
					for (int i=1;i<7;i++){
							printf(" %02X", szIN[i]);
					}
					printf(" %02X\n", szIN[7]);

					ADDR = szIN[0];
					KOP = szIN[2];

					printf("- ADDR = 0x%02X\n", ADDR);
					printf("- FUN =  0x%02X\n", szIN[1]);
					printf("- KOP =  0x%02X\n", KOP);

					float qwerty = 4.0;
					printf("- Param.flt = 0x%08X\n", qwerty);

					printf("- CRC =  0x%02X 0x%02X\n", szIN[6], szIN[7]);

					CRC = CalcCRC(szIN, 6);

					if (CRC == (szIN[7]<<8 | (szIN[6]))){
							if (Pila>100) kPila=-1;
							if (Pila<1) kPila=1;
							Pila+=kPila;
							Param.flt = float(Pila);

							szOUT[0]=ADDR;
							szOUT[1]=szIN[1];
							szOUT[2]=KOP;
							szOUT[3]=0;

							switch(KOP){
								case 1:
								case 3:
								case 5:
								case 8:
								case 9:
									LenOut = 22;
									szOUT[4] = Param.data[0];
									szOUT[5] = Param.data[1];
									szOUT[6] = Param.data[2];
									szOUT[7] = Param.data[3];

									szOUT[8] = Param.data[0];
									szOUT[9] = Param.data[1];
									szOUT[10] = Param.data[2];
									szOUT[11] = Param.data[3];

									szOUT[12] = Param.data[0];
									szOUT[13] = Param.data[1];
									szOUT[14] = Param.data[2];
									szOUT[15] = Param.data[3];

									szOUT[16] = Param.data[0];
									szOUT[17] = Param.data[1];
									szOUT[18] = Param.data[2];
									szOUT[19] = Param.data[3];

									CRC = CalcCRC(szOUT, LenOut-2);
									szOUT[LenOut-2]=CRC & 0x00FF ;
									szOUT[LenOut-1]=(CRC>>8) & 0x00FF;
									break;

								case 10:
								case 11:
									LenOut = 18;
									szOUT[4] = Param.data[0];
									szOUT[5] = Param.data[1];
									szOUT[6] = Param.data[2];
									szOUT[7] = Param.data[3];

									szOUT[8] = Param.data[0];
									szOUT[9] = Param.data[1];
									szOUT[10] = Param.data[2];
									szOUT[11] = Param.data[3];

									szOUT[12] = Param.data[0];
									szOUT[13] = Param.data[1];
									szOUT[14] = Param.data[2];
									szOUT[15] = Param.data[3];

									CRC = CalcCRC(szOUT, LenOut-2);
									szOUT[LenOut-2]=CRC & 0x00FF ;
									szOUT[LenOut-1]=(CRC>>8) & 0x00FF;
									break;

								case 13:
									LenOut = 10;
									szOUT[4] = Param.data[0];
									szOUT[5] = Param.data[1];
									szOUT[6] = Param.data[2];
									szOUT[7] = Param.data[3];

									CRC = CalcCRC(szOUT, LenOut-2);
									szOUT[LenOut-2]=CRC & 0x00FF ;
									szOUT[LenOut-1]=(CRC>>8) & 0x00FF;
									break;

								case 18:
									LenOut = 16;
									szOUT[4] = 'S';
									szOUT[5] = 'e';
									szOUT[6] = 'r';
									szOUT[7] = 'i';
									szOUT[8] = 'a';
									szOUT[9] = 'l';
									szOUT[11] = '0';
									szOUT[10] = '0';
									szOUT[12] = '0';
									szOUT[13] = '1';

									CRC = CalcCRC(szOUT, LenOut-2);
									szOUT[LenOut-2]=CRC & 0x00FF ;
									szOUT[LenOut-1]=(CRC>>8) & 0x00FF;
									break;

								case 24:
									LenOut = 14;
									szOUT[4] = (iParam & 0x000000FF);
									szOUT[5] = ((iParam & 0x000000FF)<<8);
									szOUT[6] = ((iParam & 0x000000FF)<<16);
									szOUT[7] = ((iParam & 0x000000FF)<<24);

									szOUT[8] = (iParam & 0x000000FF);
									szOUT[9] = ((iParam & 0x000000FF)<<8);
									szOUT[10] = 0xFF;
									szOUT[11] = 0xFF;

									CRC = CalcCRC(szOUT, LenOut-2);
									szOUT[LenOut-2]=CRC & 0x00FF ;
									szOUT[LenOut-1]=(CRC>>8) & 0x00FF;
									break;

								case 32:
									LenOut = 12;
									szOUT[4] = 30;
									szOUT[5] = 20;
									szOUT[6] = 10;
									szOUT[7] = 10;
									szOUT[8] = 10;
									szOUT[9] = 10;

									CRC = CalcCRC(szOUT, LenOut-2);
									szOUT[LenOut-2]=CRC & 0x00FF ;
									szOUT[LenOut-1]=(CRC>>8) & 0x00FF;
									break;

								case 52:
									LenOut = 24;
									szOUT[4] = Param.data[0];
									szOUT[5] = Param.data[1];
									szOUT[6] = Param.data[2];
									szOUT[7] = Param.data[3];

									szOUT[8] = Param.data[0];
									szOUT[9] = Param.data[1];
									szOUT[10] = Param.data[2];
									szOUT[11] = Param.data[3];

									szOUT[12] = Param.data[0];
									szOUT[13] = Param.data[1];
									szOUT[14] = Param.data[2];
									szOUT[15] = Param.data[3];

									szOUT[16] = Param.data[0];
									szOUT[17] = Param.data[1];
									szOUT[18] = Param.data[2];
									szOUT[19] = Param.data[3];

									szOUT[20] = 0xFF;
									szOUT[21] = 0xFF;

									CRC = CalcCRC(szOUT, LenOut-2);
									szOUT[LenOut-2]=CRC & 0x00FF ;
									szOUT[LenOut-1]=(CRC>>8) & 0x00FF;
									break;
							}

							Sleep(50);

							// Echo the buffer back to the sender
							iSendResult = send( ClientSocket, (const char*)szOUT, LenOut, 0 );
							if (iSendResult == SOCKET_ERROR) {
									printf("send failed with error: %d\n", WSAGetLastError());
									closesocket(ClientSocket);
									WSACleanup();
									return -1;
							}

							time (&t);
							t_m = localtime(&t);

							printf("%02d:%02d:%02d - Bytes sent: %d\n", t_m->tm_hour, t_m->tm_min, t_m->tm_sec, iSendResult);
							printf("OUT: %02X", szOUT[0]);

							for (int i=1;i<(LenOut-1);i++){
									printf(" %02X", szOUT[i]);
							}

							printf(" %02X\n\n", szOUT[LenOut-1]);
					}
					// Ошибка контрольной суммы
					else{
							printf("\n---------------------------\n");
							printf("Fail CRC!\nCRC IN: %02X%02X\nTRUE CRC: %02X\n", szIN[6], szIN[7], CRC);
							printf("---------------------------\n\n");
					}
        }
        else if (iResult == 0)
            printf("Connection closing...\n");
        else  {
            printf("recv failed with error: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return -1;
        }

    } while (iResult > 0);

    // shutdown the connection since we're done
    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return -1;
    }

    // cleanup
    closesocket(ClientSocket);
    WSACleanup();

    }

    WSACleanup();
    return 0;
}

// Вычисление контрольной суммы
uint16_t CalcCRC(uint8_t* Telegram, uint8_t len){

    const uint8_t tblCRChi[]={
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81,
    0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
    0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40};

    const uint8_t tblCRClo[]={
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4,
    0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
    0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD,
    0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7,
    0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
    0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE,
    0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2,
    0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
    0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB,
    0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50, 0x90, 0x91,
    0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
    0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88,
    0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80,
    0x40};

    uint16_t idx;
    uint8_t CRChi = 0xFF;
    uint8_t CRClo = 0xFF;

    while(len--)
    {
        idx = (CRChi ^ *Telegram++) & 0xFF;
        CRChi = CRClo ^ tblCRChi[idx];
        CRClo = tblCRClo[idx];
    }
    return ((CRChi << 8) | CRClo);
}
