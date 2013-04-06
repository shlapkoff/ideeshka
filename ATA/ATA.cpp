// АВМиС. Лабораторная работа 2
// Опрос ATA (IDE) устройств
// Solodky, Shlapkov / 050502
// andrey@shlapkoff.name / April 2013

#include "stdafx.h"
#include "hexioctrl.h"
#include <stdio.h>
#include <conio.h>
#include <iostream>
#include <string.h>

#define IDENTIFY_DEVICE 		0xEC
#define IDENTIFY_PACKET_DEVICE	0xA1
#define FILE_OUTPUT				"output.html"

const int dataRegister[2] = {0x1F0, 0x170};
const int DH_register[2] = {0x1F6, 0x176};
const int StateCommandRegister[2] = {0x1F7, 0x177};
const int altStateRegister[2] = {0x3F6, 0x376};

unsigned short data[256]; // 256 word

void WaitDeviceBusy(int channelNum);
bool getDeviceInfo(int devNum, int channelNum);
void showTable(FILE *f);
bool waitReady(int channelNum);


int main()
{

    ALLOW_IO_OPERATIONS;
    setlocale(LC_ALL, "");

	FILE *f = fopen(FILE_OUTPUT,"w");
	fprintf (f,"<table><tr><th>Device:</th><th>Channel:</th><th>Model:</th><th>Serial Number:</th><th>Type:</th><th>Size:</th><th>PIO:</th><th>Multiword DMA:</th><th>Ultra DMA:</th></tr>\n");
	printf ("Start generation .html file...\n");
    for (int channel = 0; channel <= 1; channel++ )
        for (int device = 0; device <= 1; device++ )
        {              
            if(getDeviceInfo(device, channel)) 
            {
				char *nchannel = new char[50];
				char *ndevice  = new char[50];
				if (channel == 0)
					strcpy (nchannel, "IDE1");
				else 
					strcpy (nchannel, "IDE2");
				if (device == 0)
					strcpy (ndevice, "Master");
				else
					strcpy (ndevice, "Slave");
                fprintf(f,"<tr><th> %s </th> <th>%s</th>\n", nchannel, ndevice);
                showTable(f);
            }
        }
		printf ("Generation Complete!\n");
		printf ("Open output.html file\n");
        system("pause");
        return 0;
}


bool waitReady(int channelNum)
{
    for (int i = 0; i < 1000; i++)
    {
        unsigned char state = _inp(altStateRegister[channelNum]);	//status register
        if(state & (1 << 6)) return true;
    }
    return false;
}


void  WaitDeviceBusy(int channelNum)
{
    unsigned char state;
    do state = _inp(altStateRegister[channelNum]); //status register
    while (state & (1 << 7));
}

bool  getDeviceInfo(int devNum, int channelNum)
{  
    const int commands[2] = {IDENTIFY_PACKET_DEVICE, IDENTIFY_DEVICE};
    for (int i = 0; i < 2; i++)
    {
        // wait bit BSY
        WaitDeviceBusy(channelNum);

        // adress device
        unsigned char regData = (devNum << 4) + (7 << 5); //111X0000
        _outp(DH_register[channelNum], regData); //numver device in DH

        // if device set
        if(!waitReady(channelNum))  return false;      

        // write code command in register commands
        _outp(StateCommandRegister[channelNum], commands[i]); // 

        WaitDeviceBusy(channelNum);
    }

    // get config block
    for( int i = 0; i < 256; i++ )
        data[i] = _inpw(dataRegister[channelNum] );
   
    return true;
}

void showTable(FILE *f)
{
	//model
	fprintf (f, "<th>");
    for(int i = 27; i <= 46; i++)
        fprintf(f, "%c%c", data[i] >> 8, data[i] & 0x00FF );
	fprintf (f, "</th>");
	//serial number
	fprintf (f, "<th>");
    for( int i = 10; i <= 19; i++ )
        fprintf(f,"%c%c", data[i] >> 8, data[i] & 0x00FF );
	fprintf (f, "</th>");
	//Type 
    if(data[0] & (1 << 15)) fprintf(f,"<th>ATAPI</th>\n");
    else fprintf(f,"<th>ATA</th>\n");

    // if ATA
    if(!(data[0] & (1 << 15)))
    {
		fprintf(f, "<th>");
        fprintf(f, "%lu byte", ((unsigned long *)data)[30] >> 1);	
		fprintf(f, "</th>");
    } 
	else
	{
		fprintf (f,"<th></th>");
	}

	fprintf(f, "<th>");
    fprintf(f, "  [%s%s", (data[64] & 1) ? "+" : "-" ,"] PIO 3 <br/>" );
    fprintf(f, "  [%s%s", (data[64] & 2) ? "+" : "-" ,"] PIO 4 <br/>" );
	fprintf(f, "</th>");

	fprintf(f, "<th>");
    fprintf(f, "  [%s%s", (data[63] & 1) ? "+" : "-" ,"] MWDMA 0 <br/>" );
    fprintf(f, "  [%s%s", (data[63] & 2) ? "+" : "-" ,"] MWDMA 1 <br/>" );
    fprintf(f, "  [%s%s", (data[63] & 4) ? "+" : "-" ,"] MWDMA 2 <br/>" );
	fprintf(f, "</th>");

	fprintf(f, "<th>");
    fprintf(f, "  [%s%s", (data[88] & 1) ? "+" : "-" ,"] UDMA Mode 0 <br/>" );
    fprintf(f, "  [%s%s", (data[88] & (1 << 1)) ? "+" : "-" ,"] UDMA Mode 1<br/>" );
    fprintf(f, "  [%s%s", (data[88] & (1 << 2)) ? "+" : "-" ,"] UDMA Mode 2<br/>" );
    fprintf(f, "  [%s%s", (data[88] & (1 << 3)) ? "+" : "-" ,"] UDMA Mode 3<br/>" );
    fprintf(f, "  [%s%s", (data[88] & (1 << 4)) ? "+" : "-" ,"] UDMA Mode 4<br/>" );
    fprintf(f, "  [%s%s", (data[88] & (1 << 5)) ? "+" : "-" ,"] UDMA Mode 5<br/>" );
	fprintf(f, "</th>");

}