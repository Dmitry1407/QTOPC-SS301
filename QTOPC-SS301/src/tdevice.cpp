#include "tdevice.h"

TDevice::TDevice()
{
	tags = new UINT[TAGS_IN_DEVICE];

	tagStatus = new int[TAGS_IN_DEVICE];

	tagValue = new char * [TAGS_IN_DEVICE];
	for (int i=0; i<TAGS_IN_DEVICE; i++)
	{
		tagValue[i] = new char[200];
		strcpy(tagValue[i], "0");
	}
}

TDevice::~TDevice()
{
	delete [] tags;
	delete [] tagStatus;
	for (int i=0; i<DEV_NUM_MAX; i++)
	{
		delete []tagValue[i];
	}
	delete []tagValue;
}
