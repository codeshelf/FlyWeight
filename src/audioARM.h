/*
  FlyWeight
  © Copyright 2005, 2006 Jeffrey B. Williams
  All rights reserved

  $Id$
  $Name$
*/

#ifndef AUDIOARM_H_
#define AUDIOARM_H_

#include "Timer.h"
#include "FreeRTOS.h"
#include "gwTypes.h"

void setupAudioLoader(void);
void setupPWM(void);

void audioLoaderCallback(TmrNumber_t tmrNumber);

#endif /* AUDIOARM_H_ */
