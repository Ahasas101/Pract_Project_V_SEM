/*
 * sim.c
 *
 *  Created on: Oct 4, 2025
 *      Author: Ahasas
 */

#include "gsm.h"
 //   PA2     ------> USART2_TX
   // PA3     ------> USART2_RX

volatile uint8_t count = 0;

extern UART_HandleTypeDef huart2;
static void GsmSendCommand(char* command);
static void GsmGetResponse(char* command);
static volatile Gsm_Rec_Status_def RecStatus;
static volatile uint8_t RecBuffer[100];
static volatile uint8_t data;
static volatile uint8_t nextlinecount = 0;
static uint8_t data_pos = 0;

static void GsmSendCommand(char* command)
{
	HAL_UART_Transmit(&huart2, (uint8_t*)command, strlen(command), HAL_MAX_DELAY);
	HAL_Delay(200);
}

static void GsmGetResponse(char* command)
{
	data_pos = 0;
	nextlinecount = 0;
	memset((char*)RecBuffer, 0, sizeof(RecBuffer));
	HAL_UART_Transmit(&huart2, (uint8_t*)command, strlen(command), HAL_MAX_DELAY);
	RecStatus = Busy;
	HAL_UART_Receive_IT(&huart2, (uint8_t*)&data, 1);

}

void GsmInit(void)
{
	GsmSendCommand("ATE0\r\n");          // Disable echo
	GsmSendCommand("AT+CMEE=0\r\n");     // Disable verbose errors
	GsmSendCommand("AT+CREG=0\r\n");     // Disable network registration URC
	GsmSendCommand("AT+CGREG=0\r\n");    // Disable GPRS registration URC
	GsmSendCommand("AT+CNMI=0,0,0,0,0\r\n"); // Disable SMS notifications
	GsmSendCommand("AT+CLIP=0\r\n");     // Disable caller ID notifications
	//GsmSendCommand("AT&W\r\n");          // Save settings permanently

}

Gsm_Status_def GsmGetStatus(void)
{
	GsmGetResponse("AT\r\n");
	while(RecStatus != Available);
	if (strcmp((char*)RecBuffer, "OK") == 0)
	{
		return GsmActive;
	}
	else
	{
		return GsmError;
	}


}

Sim_Status_def GsmGetSimStatus(void) // AT+CPIN?
{
	GsmGetResponse("AT+CPIN?");
	while(RecStatus != Available);
	if (strcmp((char*)RecBuffer, "+CPIN: READY") == 0)
	{
		return SimReady;
	}
	else if (strcmp((char*)RecBuffer, "+CPIN: SIM PIN") == 0)
	{
		return SimLocked;
	}
	else if (strcmp((char*)RecBuffer, "+CPIN: SIM PUK") == 0)
	{
		return SimPuk;
	}

	else if (strcmp((char*)RecBuffer, "+CPIN: NOT INSERTED") == 0)
	{
		return NotPresent;
	}
	else if (strcmp((char*)RecBuffer, "+CPIN: SIM FAILURE") == 0)
	{
		return SimFailure;
	}
	else
	{
		return UnknownError;
	}



}

char* GsmGetSimICCID(void); // AT+CCID
Call_Status_def GsmCallNo(char* no) //ATD7017586549;
{

}
Call_Status_def GsmCallHangUp(void); //ATH


void GsmCallbackHandler(void)
{ // /r/nOK/r/n
	count++;
	if(nextlinecount <= 2)
	{
		if(data == '\n' || data == '\r')
		{
			if(data == '\r') {nextlinecount++;}
		}
		else
		{
		RecBuffer[data_pos] = data;
		data_pos++;
		}

		HAL_UART_Receive_IT(&huart2, (uint8_t*)&data, 1);
	}

	else
	{
		//nextlinecount = 0;
		RecBuffer[data_pos] = '\0';
		data_pos = 0;
		RecStatus = Available;
	}
}
