/* Copyright 2016, Ezequiel Martinez Vazquez
 *
 * All rights reserved.
 *
 * This file is part of Workspace.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*==================[inclusions]=============================================*/

#include "main.h"

/*==================[macros and definitions]=================================*/

/*==================[global data]============================================*/

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/** @brief hardware initialization function
 * @return none
 */
static void initHardware(void);

/** @brief delay function
 * @param t desired milliseconds to wait
 */
static void pausems(uint32_t t);

void * cb (errorUser_t, void *);

void * cbempty (errorUser_t, void *);

void * cbgsmgprs (errorUser_t, void *);

void * cbprint (errorUser_t, void *);

/*==================[internal data definition]===============================*/

/* TIMING COUNTERS */

/** @brief used for delay counter */
static uint32_t pausemsCount = DELAY_INIT;

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*---------------------------------------------------------------------------*/
/*                Hardware initialization and system functions               */
/*---------------------------------------------------------------------------*/

static void initHardware(void)
{
   Board_Init();
   SystemCoreClockUpdate();
   SysTick_Config(SystemCoreClock / 1000);
}

static void pausems(uint32_t t)
{
   pausemsCount = t;
   while (pausemsCount != 0) {
      __WFI();
   }
}

/*---------------------------------------------------------------------------*/
/*                             Callback functions                            */
/*---------------------------------------------------------------------------*/

void * cbempty (errorUser_t error_in, void * input)
{
   dbgPrint("Funcion cbempty ejecutada\r\n");

   if(OK != error_in.errorFrm){
      dbgPrint("Error en la ejecucion de formula: ");

      switch(error_in.errorFrm){

         case ERR_INIT:

            dbgPrint("Error en inicializacion\r\n");

            break;

         case ERR_PROC:

            dbgPrint("Error en proceso\r\n");

            break;

         case ERR_GSM:

            dbgPrint("Error del engine GSM --- ");
            dbgPrint(error_in.errorCmd.cmd);
            dbgPrint("(");
            dbgPrint(error_in.errorCmd.par);
            dbgPrint(") ---\r\n");

            break;

         case ERR_WRAP:

            dbgPrint("Error en cierre\r\n");

            break;

         default:

            dbgPrint("Error no reconocido\r\n");

            break;

      }

   }

   return 0;
}

void * cbled (errorUser_t error_in, void * input)
{
   if(OK != error_in.errorFrm){
      dbgPrint("Error en la ejecucion de formula: ");

      switch(error_in.errorFrm){

         case ERR_INIT:

            dbgPrint("Error en inicializacion\r\n");

            break;

         case ERR_PROC:

            dbgPrint("Error en proceso\r\n");

            break;

         case ERR_GSM:

            dbgPrint("Error del engine GSM --- ");
            dbgPrint(error_in.errorCmd.cmd);
            dbgPrint("(");
            dbgPrint(error_in.errorCmd.par);
            dbgPrint(") ---\r\n");

            break;

         case ERR_WRAP:

            dbgPrint("Error en cierre\r\n");

            break;

         default:

            dbgPrint("Error no reconocido\r\n");

            break;

      }

   }

   dbgPrint("Actualizando LEDs...\r\n");

   uint8_t i = 0;
   smsRec_t * target = (smsRec_t *)input;

   for(i = 0; (target+i)->meta[0] != '\0'; i++){

      if(0 != strstr((target+i)->text,"ledverdeon")){
         Board_LED_Set(LED_VERDE,1);
         dbgPrint("Enciendo LED...\r\n");
      }
      if(0 != strstr((target+i)->text,"ledverdeoff")){
         Board_LED_Set(LED_VERDE,0);
         dbgPrint("Apago LED...\r\n");
      }

   }

   return;
}

void * cbprint (errorUser_t error_in, void * input)
{
   if(OK != error_in.errorFrm){
      dbgPrint("Error en la ejecucion de formula: ");

      switch(error_in.errorFrm){

         case ERR_INIT:

            dbgPrint("Error en inicializacion\r\n");

            break;

         case ERR_PROC:

            dbgPrint("Error en proceso\r\n");

            break;

         case ERR_GSM:

            dbgPrint("Error del engine GSM --- ");
            dbgPrint(error_in.errorCmd.cmd);
            dbgPrint("(");
            dbgPrint(error_in.errorCmd.par);
            dbgPrint(") ---\r\n");

            break;

         case ERR_WRAP:

            dbgPrint("Error en cierre\r\n");

            break;

         default:

            dbgPrint("Error no reconocido\r\n");

            break;

      }

   }

   else{

      dbgPrint("Imprimiendo SMS...\r\n\r\n");

      uint8_t i = 0;
      smsRec_t * target = ((smsListRet_t *)input)->msgs;
      uint8_t noMsg = ((smsListRet_t *)input)->noMsgs;

      uint8_t auxtext[5];

      dbgPrint("Nro de mensajes: ");
      itoa(noMsg, auxtext, 10);
      dbgPrint(auxtext);
      dbgPrint("\r\n");

      for(i = 0; i < noMsg; i++){

         dbgPrint((target+i)->text);
         dbgPrint("\r\n");

      }

      dbgPrint("\r\nFin de los mensajes\r\n");

   }

   return;
}

void * cbgsmgprs (errorUser_t error_in, void * input)
{
   if(OK != error_in.errorFrm){
      dbgPrint("Error en la ejecucion de formula: ");

      switch(error_in.errorFrm){

         case ERR_INIT:

            dbgPrint("Error en inicializacion\r\n");

            break;

         case ERR_PROC:

            dbgPrint("Error en proceso\r\n");

            break;

         case ERR_GSM:

            dbgPrint("Error del engine GSM --- ");
            dbgPrint(error_in.errorCmd.cmd);
            dbgPrint("(");
            dbgPrint(error_in.errorCmd.par);
            dbgPrint(") ---\r\n");

            break;

         case ERR_WRAP:

            dbgPrint("Error en cierre\r\n");

            break;

         default:

            dbgPrint("Error no reconocido\r\n");

            break;

      }

   }

   else{

      if(true == ((connStatus_t *)input)->gsm){
         dbgPrint("\r\n Conectado a red GSM\r\n");
      }
      else{
         dbgPrint("\r\n No conectado a red GSM\r\n");
      }

      if(true == ((connStatus_t *)input)->gprs){
         dbgPrint("\r\n Conectado a servicio GPRS\r\n");
      }
      else{
         dbgPrint("\r\n No conectado a servicio GPRS\r\n");
      }

   }

   return;
}

void cbUrc (uint8_t const * const cmd, uint8_t const * const par)
{
   dbgPrint("\r\nURC received!\r\n");
   dbgPrint("CMD: ");
   dbgPrint(cmd);
   dbgPrint(" PAR: ");
   dbgPrint(par);
   dbgPrint("\r\n\r\n");

   return;
}

void cbData (gsmInterface_t * interface)
{
   uint8_t usbReadBuf[21];
   uint8_t serialReadBuf[21];

   uint8_t nRead = 20;

   uint8_t n;
   static uint8_t nCalls = 0;

   nCalls++;

   /* Read USB UART and store it */

   n = uartRecv(CIAA_UART_USB, usbReadBuf, 20);
   usbReadBuf[n]='\0';

   /* Echo USB UART to screen */

   n = uartSend(CIAA_UART_USB, usbReadBuf, n);

   /* If we type an uppercase X, call the gsmExitDataMode function */

   if(strstr(usbReadBuf,"X")){
      gsmExitDataMode(interface, cbempty);
   }

   /* Write USB UART data to serial port and read from serial port */

   n = gsm232UartSend (usbReadBuf, n);
   nRead = gsm232UartRecv (serialReadBuf, nRead);

   serialReadBuf[nRead]='\0';

   n = gsmCheckDataMode(interface, &serialReadBuf[0], &nRead);

   /* Write data read from serial port to USB UART */

   uartSend(CIAA_UART_USB, serialReadBuf, n);

   return;
}

/*---------------------------------------------------------------------------*/
/*                         Testing console functions                         */
/*---------------------------------------------------------------------------*/

void console_sms (gsmInterface_t * interface)
{
   uint8_t instruction = 0;

   smsOut_t msg = {"1151751809","Hola mundo!"};
   smsConf_t conf;
   smsRec_t msgList[SMS_READ_SIZ];
   smsDelPars_t msgDel = {1, DEL_ALL};
   smsRec_t recMsg;
   smsReadPars_t parRead = {1, NOCHANGE};
   smsListPars_t parList = {ALL_MSG, NOCHANGE, SMS_READ_SIZ};

   while ('S' != instruction){

      if(gsmIsIdle(interface)){

         dbgPrint("\r\n\r\n>>> CONSOLA SMS <<< \r\n\r\n");

         dbgPrint("1) Mandar SMS \r\n");
         dbgPrint("2) Leer todos los SMSs \r\n");
         dbgPrint("3) Borrar todos los SMSs \r\n");
         dbgPrint("4) Leer el primer SMS \r\n\r\n");

         dbgPrint("S) Salir a la consola principal \r\n");

         while(0 == uartRecv(CIAA_UART_USB, &instruction, 1)){
            gsmProcess(interface);
         }

         switch (instruction) {

            case '1':

            gsmSmsSend(interface, &msg, &conf, cbempty);

            break;

            case '2':

            gsmSmsList(interface, &msgList[0], &parList, cbprint);

            break;

            case '3':

            gsmSmsDel(interface, &msgDel, cbempty);

            break;

            case '4':

            gsmSmsRead(interface, &recMsg, &parRead, cbprint);

            break;

            case 'S':

            break;

            default:

            dbgPrint("INSTRUCCION DESCONOCIDA \r\n\r\n");
            break;

         }

         while(!gsmIsIdle(interface)){
            gsmProcess(interface);
         }

      }

   }

   return;

}

void console_gprs (gsmInterface_t * interface)
{
   uint8_t instruction = 0;

   apnUserPwd_t APN = {"datos.personal.com","datos","datos"};
   socket_t port1 = {TCP, "104.236.225.217",2399};
   socket_t port2 = {UDP, "104.236.225.217",2399};

   while ('S' != instruction){

      if(gsmIsIdle(interface)){

         dbgPrint("\r\n\r\n>>> CONSOLA GPRS <<< \r\n\r\n");

         dbgPrint("1) Prender GPRS \r\n");
         dbgPrint("2) Abrir puerto TCP \r\n");
         dbgPrint("3) Abrir puerto UDP \r\n");
         dbgPrint("4) Cerrar puerto TCP o UDP \r\n");
         dbgPrint("5) Apagar GPRS \r\n\r\n");

         dbgPrint("S) Salir a la consola principal \r\n");

         while(0 == uartRecv(CIAA_UART_USB, &instruction, 1)){
            gsmProcess(interface);
         }

         switch (instruction) {

            case '1':

            gsmGprsStart(interface, &APN, cbempty);

            break;

            case '2':

            gsmGprsOpenPort(interface, &port1, cbempty);

            break;

            case '3':

            gsmGprsOpenPort(interface, &port2, cbempty);

            break;

            case '4':

            gsmGprsClosePort(interface, cbempty);

            break;

            case '5':

            gsmGprsStop(interface, cbempty);

            break;

            case 'S':

            break;

            default:

            dbgPrint("INSTRUCCION DESCONOCIDA \r\n\r\n");
            break;

         }

         while(!gsmIsIdle(interface)){
            gsmProcess(interface);
         }

         while(DATA_MODE == gsmGetSerialMode(&interface->engine)){
            gsmProcess(interface);
         }

      }

      else{
         gsmProcess(interface);
      }

   }

   return;
}

void console_gnss (gsmInterface_t * interface)
{
   uint8_t instruction = 0;

   dataGnss_t navInfo;
   pwrGnss_t powerGNSS;

   while ('S' != instruction){

      if(gsmIsIdle(interface)){

         dbgPrint("\r\n\r\n>>> CONSOLA GNSS <<< \r\n\r\n");

         dbgPrint("1) Prender GNSS \r\n");
         dbgPrint("2) Apagar GNSS \r\n");
         dbgPrint("3) Obtener informacion de navegacion GNSS \r\n\r\n");

         dbgPrint("S) Salir a la consola principal \r\n");

         while(0 == uartRecv(CIAA_UART_USB, &instruction, 1)){
            gsmProcess(interface);
         }

         switch (instruction) {

            case '1':

            powerGNSS = ON;

            gsmGnssPwr(interface, &powerGNSS, cbempty);

            break;

            case '2':

            powerGNSS = OFF;

            gsmGnssPwr(interface, &powerGNSS, cbempty);

            break;

            case '3':

            gsmGnssGetData(interface, &navInfo, cbempty);

            break;

            case 'S':

            break;

            default:

            dbgPrint("INSTRUCCION DESCONOCIDA \r\n\r\n");
            break;

         }

         while(!gsmIsIdle(interface)){
            gsmProcess(interface);
         }

      }

   }

   return;
}

void console_urc (gsmInterface_t * interface)
{
   uint8_t instruction = 0;

   urc_t urc = {"\0","\0"};

   while ('S' != instruction){

      if(gsmIsIdle(interface)){

         dbgPrint("\r\n\r\n>>> CONSOLA URC <<< \r\n\r\n");

         dbgPrint("1) Leer URC mas reciente\r\n");
         dbgPrint("2) Poner URC handling en modo callback\r\n");
         dbgPrint("3) Poner URC handling en modo manual\r\n\r\n");

         dbgPrint("S) Salir a la consola principal \r\n");

         while(0 == uartRecv(CIAA_UART_USB, &instruction, 1)){
            gsmProcess(interface);
         }

         switch (instruction) {

            case '1':

            if (gsmReadUrc(interface, &urc)){

               dbgPrint("\r\nURC: ");
               dbgPrint(urc.cmd);
               dbgPrint("(");
               dbgPrint(urc.par);
               dbgPrint(")\r\n");

            }

            else{
               dbgPrint("\r\nNo hay URCs pendientes\r\n");
            }

            break;

            case '2':

            gsmSetUrcMode(interface, CBACK_MODE);

            break;

            case '3':

            gsmSetUrcMode(interface, MANUAL_MODE);

            break;

            case 'S':

            break;

            default:

            dbgPrint("INSTRUCCION DESCONOCIDA \r\n\r\n");
            break;

         }

         while(!gsmIsIdle(interface)){
            gsmProcess(interface);
         }

      }

   }

   return;
}

/*==================[external functions definition]==========================*/

void SysTick_Handler(void)
{
   if(pausemsCount > 0) pausemsCount--;

   gsmSysTickHandler();
}

int main(void)
{
   initHardware();
   ciaaUARTInit();

   pausems(DELAY_INIT);

   uint8_t instruction;

   sigQual_t sigqual;
   connStatus_t status;

   gsmInterface_t interface;
   gsmInitInterface(&interface); /* Initializes the GSM interface */

   gsmSetUrcCback(&interface, cbUrc);
   gsmSetDataCback(&interface, cbData);

   gsmStartUp(&interface, cbempty);

   while(!gsmIsIdle(&interface)){
      gsmProcess(&interface);
   }

   dbgPrint("\r\n>>> INICIALIZANDO MODEM CELULAR <<< \r\n\r\n");

   while (1){

      if(gsmIsIdle(&interface)){

         dbgPrint("\r\n>>> CONSOLA PRINCIPAL <<< \r\n\r\n");

         dbgPrint("1) CONSOLA SMS \r\n");
         dbgPrint("2) CONSOLA GPRS \r\n");
         dbgPrint("3) CONSOLA GNSS \r\n");
         dbgPrint("4) CONSOLA URC \r\n\r\n");

         dbgPrint("5) Ver calidad de señal \r\n");
         dbgPrint("6) Estado red GSM y GPRS \r\n\r\n");

         while(0 == uartRecv(CIAA_UART_USB, &instruction, 1)){;}

         switch (instruction) {

            case '1':

            console_sms(&interface);

            break;

            case '2':

            console_gprs(&interface);

            break;

            case '3':

            console_gnss(&interface);

            break;

            case '4':

            console_urc(&interface);

            break;

            case '5':

            gsmGetSigQual (&interface, &sigqual, cbempty);

            break;

            case '6':

            gsmCheckConn(&interface, &status, cbgsmgprs);

            break;

            default:

            dbgPrint("INSTRUCCION DESCONOCIDA \r\n\r\n");

            break;

         }

         while(!gsmIsIdle(&interface)){
            gsmProcess(&interface);
         }

      }

      gsmProcess(&interface);

   }

   return 0;
}

/** @} doxygen end group definition */

/*==================[end of file]============================================*/
