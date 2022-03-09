#include <windows.h>
#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "main.h"

#ifndef TDEVICE_H
#define TDEVICE_H

class TDevice
{
public:
	TDevice();
	~TDevice();

public:
	int idPort;
	int idDevice;
	int numPort;
	int numDevice;

	UINT	channels;                   // number of channels
	UINT	*tags;				// tags link
	int     *tagStatus;
	char    **tagValue;

	/*UINT	tags[TAGS_IN_DEVICE];				// tags link
	int     tagStatus[TAGS_IN_DEVICE];
	char    tagValue[TAGS_IN_DEVICE][200];*/

	// Variables
	uint8_t DEV;
	uint8_t FUN;
	uint8_t KOP;
	uint8_t SHIFT;
	uint8_t TARIF;
	uint8_t CORRECT;

	// Коэффициенты
	float KU, KI;
	UINT  Ke, Kpr;

	// Мнговенные значения параметров
	float FLine;
	float Ua, Ub, Uc;
	float Ia, Ib, Ic;
	float Pa, Pb, Pc;
	float Qa, Qb, Qc;
	// Суммарная энергия
	float AEp, AEo, REp, REo;
	// Средняя трехминутная мощность
	float AEp3m, AEo3m, REp3m, REo3m;

	// Заводской номер
	char Serial[10];

private:
protected:
};


#endif // TDEVICE_H
