/* Copyright 2016, Ezequiel Martinez Vazquez
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

/** @brief This module handles the user interface
 */

/** \addtogroup interface interface
 ** @{ */

/*==================[inclusions]=============================================*/

#include "gsmInterface.h"

/*==================[macros and definitions]=================================*/

#define DEBUG_INTERF
#ifdef DEBUG_INTERF
   #define debug(msg) dbgPrint(msg)
#else
   #define debug(msg)
#endif

/*==================[global data]============================================*/

/*==================[internal data declaration]==============================*/

/** @brief State of the current formula being run
 */

static frmStatus_e frmState = IDLE;

/** @brief Function pointer to formula function for the current formula
 */

static void (*frm) (void);

/** @brief Pointer to inputs for the current formula being run
 */

static void * frmInput;

/** @brief Pointer to outputs for the current formula being run
 */

static void * frmOutput;

/** @brief Function pointer to callback for the current formula being run
 */

static void * (*frmCback) (errorUser_s, void *);

/** @brief used for GSM processing period counter */

static int32_t procCount = DELAY_PROC;

/*==================[internal functions declaration]=========================*/

/*---------------------------------------------------------------------------*/
/*                  General GSM library operation functions                  */
/*---------------------------------------------------------------------------*/

/** @brief Formula to start up the GSM library
 */

static void gsmStartUpF (void);

/** @brief Formula to get signal quality values (RSSI and BER)
 */

static void gsmGetSigQualF (void);

/** @brief Formula to check status of GSM and GPRS connection
 */

static void gsmCheckConnF (void);

/*---------------------------------------------------------------------------*/
/*                              SMS functions                                */
/*---------------------------------------------------------------------------*/

/** @brief Formula to send an SMS
 */

static void gsmSmsSendF (void);

/** @brief Formula to read a single SMS
 */

static void gsmSmsReadF (void);

/** @brief Formula to read several received SMSs into a list
 */

static void gsmSmsListF (void);

/** @brief Formula to delete a single SMS from memory
 */

static void gsmSmsDelF (void);

/*---------------------------------------------------------------------------*/
/*                             GPRS functions                                */
/*---------------------------------------------------------------------------*/

/** @brief Formula to start the GPRS connection
 */

static void gsmGprsStartF (void);

/** @brief Formula to open a TCP or UDP port
 */

static void gsmGprsOpenPortF (void);

/** @brief Formula to close a TCP or UDP port
 */

static void gsmGprsClosePortF (void);

/*---------------------------------------------------------------------------*/
/*                             GNSS functions                                */
/*---------------------------------------------------------------------------*/

/** @brief Formula to turn GNSS power on or off
 */

static void gsmGnssPwrF (void);

/** @brief Formula to get GNSS data
 */

static void gsmGnssGetDataF (void);

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*---------------------------------------------------------------------------*/
/*                  General GSM library operation functions                  */
/*---------------------------------------------------------------------------*/

static void gsmStartUpF (void)
{
   static procStatus_e procState = NOCMD;
   static errorUser_s errorOut;

   fsmEvent_e result;

   switch(frmState) {

      case INIT:

         errorOut.errorFrm = OK;
         errorOut.errorCmd.cmd[0] = '\0';
         errorOut.errorCmd.par[0] = '\0';

         gsmInitEngine();

         procState = ATCMD1;
         frmState = PROC;

         break;

      case PROC:

         switch(procState){

            case ATCMD1:

               result = gsmSendCmd("AT\r");
               if(OK_CMD_SENT == result){procState = ATCMD1RESP;}
               else{errorOut.errorFrm = ERR_PROC; frmState = WRAP;}
               break;

            case ATCMD1RESP:

               result = gsmProcessTkn();
               if(NO_UPDATE != result){
                  if(OK_CMD_ACK <= result && OK_URC >= result){;}
                  else if(OK_CLOSE == result){procState = ATCMD2;}
                  else if(ERR_MSG_CLOSE == result){{errorOut.errorFrm = ERR_GSM; frmState = WRAP;};}
                  else{errorOut.errorFrm = ERR_PROC; frmState = WRAP;}
               }
               break;

            case ATCMD2:

               result = gsmSendCmd("AT+CMEE=2\r");
               if(OK_CMD_SENT == result){procState = ATCMD2RESP;}
               else{errorOut.errorFrm = ERR_PROC; frmState = WRAP;}
               break;

            case ATCMD2RESP:

               result = gsmProcessTkn();
               if(NO_UPDATE != result){
                  if(OK_CMD_ACK <= result && OK_URC >= result){;}
                  else if(OK_CLOSE == result){procState = ATCMD3;}
                  else if(ERR_MSG_CLOSE == result){{errorOut.errorFrm = ERR_GSM; frmState = WRAP;};}
                  else{errorOut.errorFrm = ERR_PROC; frmState = WRAP;}
               }
               break;

            case ATCMD3:

               result = gsmSendCmd("AT+CSCS=\"GSM\"\r");
               if(OK_CMD_SENT == result){procState = ATCMD3RESP;}
               else{errorOut.errorFrm = ERR_PROC; frmState = WRAP;}
               break;

            case ATCMD3RESP:

               result = gsmProcessTkn();
               if(NO_UPDATE != result){
                  if(OK_CMD_ACK <= result && OK_URC >= result){;}
                  else if(OK_CLOSE == result){procState = ATCMD4;}
                  else if(ERR_MSG_CLOSE == result){{errorOut.errorFrm = ERR_GSM; frmState = WRAP;};}
                  else{errorOut.errorFrm = ERR_PROC; frmState = WRAP;}
               }
               break;

            case ATCMD4:

               result = gsmSendCmd("AT+CMGF=1\r");
               if(OK_CMD_SENT == result){procState = ATCMD4RESP;}
               else{errorOut.errorFrm = ERR_PROC; frmState = WRAP;}
               break;

            case ATCMD4RESP:

               result = gsmProcessTkn();
               if(NO_UPDATE != result){
                  if(OK_CMD_ACK <= result && OK_URC >= result){;}
                  else if(OK_CLOSE == result){frmState = WRAP;}
                  else if(ERR_MSG_CLOSE == result){{errorOut.errorFrm = ERR_GSM; frmState = WRAP;};}
                  else{errorOut.errorFrm = ERR_PROC; frmState = WRAP;}
               }
               break;

         }

         break;

      case WRAP:

         if(OK != errorOut.errorFrm){

            if(ERR_GSM == errorOut.errorFrm){

               rsp_t rsp;

               rsp = gsmGetCmdRsp();
               strncpy(errorOut.errorCmd.cmd, rsp.cmd, 19);
               errorOut.errorCmd.cmd[20] = '\0';
               strncpy(errorOut.errorCmd.par, rsp.par, 149);
               errorOut.errorCmd.par[150] = '\0';

            }
         }

         frmCback(errorOut, 0);
         frmState = IDLE;

         break;
   }

   return;

}

static void gsmGetSigQualF (void)
{
   static procStatus_e procState = NOCMD;
   static errorUser_s errorOut;

   fsmEvent_e result;

   switch(frmState) {

      case INIT:

         errorOut.errorFrm = OK;
         errorOut.errorCmd.cmd[0] = '\0';
         errorOut.errorCmd.par[0] = '\0';

         procState = ATCMD1;
         frmState = PROC;

         break;

      case PROC:

         switch(procState){

            case ATCMD1:

               result = gsmSendCmd("AT+CSQ\r");
               if(OK_CMD_SENT == result){procState = ATCMD1RESP;}
               else{errorOut.errorFrm = ERR_PROC; frmState = WRAP;}
               break;

            case ATCMD1RESP:

               result = gsmProcessTkn();
               if(NO_UPDATE != result){
                  if(OK_CMD_ACK <= result && OK_URC >= result){;}
                  else if(OK_CLOSE == result){frmState = WRAP;}
                  else if(ERR_MSG_CLOSE == result){{errorOut.errorFrm = ERR_GSM; frmState = WRAP;};}
                  else{errorOut.errorFrm = ERR_PROC; frmState = WRAP;}
               }
               break;

         }

         break;

      case WRAP:

         if(OK == errorOut.errorFrm){

            rsp_t rsp;
            uint8_t auxStr[5];    /* auxiliary string for response parsing*/
            auxStr[0] = '\0';
            uint8_t i;
            uint8_t commaPos = 0; /* position of the comma in the response */

            rsp = gsmGetCmdRsp();  /* get the signal quality response */

            /* Search for the comma in the response, which is in the form RSSI,BER */

            for(i = 0; i < (strlen(rsp.par)-1); i++){
               if(',' == rsp.par[i]){commaPos = i; break;}
            }

            /* Copy the string of RSSI and convert it into a integer. Then, use the
             * table in the SIM808 manual to translate this number into an actual
             * RSSI reading in dBm.
             *
             * If the value is not known or detectable by the module, the function
             * will give a reading of positive 99. This should be interpreted by
             * the user as an error result. */

            strncpy(auxStr,&rsp.par[0],commaPos);
            auxStr[commaPos] = '\0';

            ((sigQual_s *)frmOutput)->rssi = atoi(auxStr);

            if(0 == ((sigQual_s *)frmOutput)->rssi){
               ((sigQual_s *)frmOutput)->rssi = -115;
            }

            else if(1 == ((sigQual_s *)frmOutput)->rssi){
               ((sigQual_s *)frmOutput)->rssi = -111;
            }

            else if((((sigQual_s *)frmOutput)->rssi) >= 2 &&
                    (((sigQual_s *)frmOutput)->rssi) <= 30){
               ((sigQual_s *)frmOutput)->rssi = -110+(((sigQual_s *)frmOutput)->rssi -2)*2;
            }

            else if(31 == ((sigQual_s *)frmOutput)->rssi){
               ((sigQual_s *)frmOutput)->rssi = -52;
            }

            else if(99 == ((sigQual_s *)frmOutput)->rssi){
               ((sigQual_s *)frmOutput)->rssi = 99;
            }

            /* Copy the string of BER and convert it to a number. The module reports
             * RXQUAL values as per table in GSM 05.08 [20] subclause 7.2.4 */

            strncpy(auxStr,&rsp.par[commaPos+1],strlen(rsp.par)-commaPos-1);
            auxStr[strlen(rsp.par)-commaPos-1] = '\0';

            ((sigQual_s *)frmOutput)->ber = atoi(auxStr);

            debug(">>>interf<<<   RSSI: ");
            itoa(((sigQual_s *)frmOutput)->rssi, auxStr, 10);
            debug(auxStr);
            debug(", RXQUAL: ");
            itoa(((sigQual_s *)frmOutput)->ber, auxStr, 10);
            debug(auxStr);
            debug(" \r\n");

            frmCback(errorOut, frmOutput);
            frmState = IDLE;

         }

         else if(ERR_GSM == errorOut.errorFrm){

            rsp_t rsp;

            rsp = gsmGetCmdRsp();
            strncpy(errorOut.errorCmd.cmd, rsp.cmd, 19);
            errorOut.errorCmd.cmd[20] = '\0';
            strncpy(errorOut.errorCmd.par, rsp.par, 149);
            errorOut.errorCmd.par[150] = '\0';

         }

         frmCback(errorOut, frmOutput);
         frmState = IDLE;

         break;
   }

   return;

}

void gsmCheckConnF (void)
{
   static procStatus_e procState = NOCMD;
   static errorUser_s errorOut;

   fsmEvent_e result;

   static rsp_t rspGsm;
   static rsp_t rspGprs;

   switch(frmState) {

      case INIT:

         errorOut.errorFrm = OK;
         errorOut.errorCmd.cmd[0] = '\0';
         errorOut.errorCmd.par[0] = '\0';

         procState = ATCMD1;
         frmState = PROC;

         break;

      case PROC:

         switch(procState){

            case ATCMD1:

               result = gsmSendCmd("AT+CREG?\r");
               if(OK_CMD_SENT == result){procState = ATCMD1RESP;}
               else{errorOut.errorFrm = ERR_PROC; frmState = WRAP;}
               break;

            case ATCMD1RESP:

               result = gsmProcessTkn();
               if(NO_UPDATE != result){
                  if(OK_CMD_ACK <= result && OK_URC >= result){;}
                  else if(OK_CLOSE == result){
                     rspGsm = gsmGetCmdRsp(); /* get the GSM response */
                     procState = ATCMD2;
                  }
                  else if(ERR_MSG_CLOSE == result){{errorOut.errorFrm = ERR_GSM; frmState = WRAP;};}
                  else{errorOut.errorFrm = ERR_PROC; frmState = WRAP;}
               }
               break;

            case ATCMD2:

               result = gsmSendCmd("AT+CGATT?\r");
               if(OK_CMD_SENT == result){procState = ATCMD2RESP;}
               else{errorOut.errorFrm = ERR_PROC; frmState = WRAP;}
               break;

            case ATCMD2RESP:

               result = gsmProcessTkn();
               if(NO_UPDATE != result){
                  if(OK_CMD_ACK <= result && OK_URC >= result){;}
                  else if(OK_CLOSE == result){
                     rspGprs = gsmGetCmdRsp(); /* get the GPRS response */
                     frmState = WRAP;
                  }
                  else if(ERR_MSG_CLOSE == result){{errorOut.errorFrm = ERR_GSM; frmState = WRAP;};}
                  else{errorOut.errorFrm = ERR_PROC; frmState = WRAP;}
               }
               break;

         }

         break;

      case WRAP:

         if(OK == errorOut.errorFrm){

            /* Copy the GSM info string to the provided output */

            if('1' == rspGsm.par[3]){
               ((connStatus_s *)frmOutput)->gsm = true;
            }
            else{
               ((connStatus_s *)frmOutput)->gsm = false;
            }

            /* Print out the GSM string */

            debug(">>>interf<<<   GSM String:");
            debug(&rspGsm.par[0]);
            debug(" \r\n");

            /* Copy the GPRS info string to the provided output */

            if('1' == rspGprs.par[1]){
               ((connStatus_s *)frmOutput)->gprs = true;
            }
            else{
               ((connStatus_s *)frmOutput)->gprs = false;
            }

            /* Print out the GPRS string */

            debug(">>>interf<<<   GPRS String:");
            debug(&rspGprs.par[0]);
            debug(" \r\n");

         }

         else if(ERR_GSM == errorOut.errorFrm){

            rsp_t rsp;

            rsp = gsmGetCmdRsp();
            strncpy(errorOut.errorCmd.cmd, rsp.cmd, 19);
            errorOut.errorCmd.cmd[20] = '\0';
            strncpy(errorOut.errorCmd.par, rsp.par, 149);
            errorOut.errorCmd.par[150] = '\0';

         }

         frmCback(errorOut, frmOutput);
         frmState = IDLE;

         break;
   }

   return;

}

/*---------------------------------------------------------------------------*/
/*                              SMS functions                                */
/*---------------------------------------------------------------------------*/

static void gsmSmsSendF (void)
{
   static procStatus_e procState = NOCMD;
   static errorUser_s errorOut;

   /* smsCmd is "AT+CMGS="XXX"\r" where XXX is the phone number. Maximum phone
    * number length under the ITU-T standard E.164 is 15 digits plus the "+"
      symbol if present. */

   static uint8_t smsCmd[9+16+3];

   /* Maximum SMS size according to 3GPP standard TS 23.038 is 160 chars using
    * GSM 7-bit alphabet */

   static uint8_t smsText[160+1];

   fsmEvent_e result;

   switch(frmState) {

      case INIT:

         smsCmd[0] = '\0';
         smsText[0] = '\0';

         errorOut.errorFrm = OK;
         errorOut.errorCmd.cmd[0] = '\0';
         errorOut.errorCmd.par[0] = '\0';

         /* If the SMS text has '\r' or '\n' characters it could confuse the
          * tokenizer. Therefore, we check that neither of these chars are
          * present.
          */

         if ( (strchr(((smsOut_s *)frmInput)->text, '\n') != NULL) &&
              (strchr(((smsOut_s *)frmInput)->text, '\r') != NULL) ){

            debug(">>>interf<<<   ERROR: The SMS text contains the \\r and/or \\n characters\r\n");
            errorOut.errorFrm = ERR_INIT;
            frmState = WRAP;

         }

         strncat(smsCmd, "AT+CMGS=\"", 9);
         strncat(smsCmd, ((smsOut_s *)frmInput)->dest,strlen(((smsOut_s *)frmInput)->dest));
         strncat(smsCmd, "\"\r", 2);

         strncpy(smsText, ((smsOut_s *)frmInput)->text, strlen(((smsOut_s *)frmInput)->text));
         smsText[strlen(((smsOut_s *)frmInput)->text)] = '\0';

         procState = ATCMD1;
         frmState = PROC;

         debug(">>>interf<<<   SEND SMS CMD: ");
         debug(smsCmd);
         debug("\r\n>>>interf<<<   SMS TEXT: ");
         debug(smsText);
         debug("\r\n");

         break;

      case PROC:

         switch(procState){

            case ATCMD1:

               result = gsmSendCmd(smsCmd);
               if(OK_CMD_SENT == result){procState = ATCMD1RESP;}
               else{errorOut.errorFrm = ERR_PROC; frmState = WRAP;}
               break;

            case ATCMD1RESP:

               result = gsmProcessTkn();
               if(NO_UPDATE != result){
                  if(OK_CMD_ACK <= result && OK_URC >= result){;}
                  else if(OK_CLOSE == result){procState = ATCMD2;}
                  else if(ERR_MSG_CLOSE == result){{errorOut.errorFrm = ERR_GSM; frmState = WRAP;};}
                  else{errorOut.errorFrm = ERR_PROC; frmState = WRAP;}
               }
               break;

            case ATCMD2:

               result = gsmSendCmd(smsText);
               if(OK_CMD_SENT == result){procState = ATCMD2RESP;}
               else{errorOut.errorFrm = ERR_PROC; frmState = WRAP;}
               break;

            case ATCMD2RESP:

               result = gsmProcessTkn();
               if(NO_UPDATE != result){
                  if(OK_CMD_ACK <= result && OK_URC >= result){;}
                  else if(OK_CLOSE == result){frmState = WRAP;}
                  else if(ERR_MSG_CLOSE == result){{errorOut.errorFrm = ERR_GSM; frmState = WRAP;};}
                  else{errorOut.errorFrm = ERR_PROC; frmState = WRAP;}
               }
               break;

         }

         break;

      case WRAP:

         if(OK != errorOut.errorFrm){
            if(ERR_GSM == errorOut.errorFrm){

               rsp_t rsp;

               rsp = gsmGetCmdRsp();
               strncpy(errorOut.errorCmd.cmd, rsp.cmd, 19);
               errorOut.errorCmd.cmd[20] = '\0';
               strncpy(errorOut.errorCmd.par, rsp.par, 149);
               errorOut.errorCmd.par[150] = '\0';

            }
         }
         else{

               /* If the SMS is successfully sent we are given a <mr> number
                * which represents the number assigned in the TE to the sent
                * SMS. We store it in the smsConf_s structure.
                */

               rsp_t rsp;

               rsp = gsmGetCmdRsp(); /* discard the final OK response */
               rsp = gsmGetCmdRsp(); /* get the "+CMGS:<mr>\r\n" response */

               atoi(((smsConf_s *)frmOutput)->mr, rsp.par, 10);

         }

         frmCback(errorOut, frmOutput);
         frmState = IDLE;

         break;
   }

   return;

}

static void gsmSmsReadF (void)
{
   static procStatus_e procState = NOCMD;
   static errorUser_s errorOut;

   fsmEvent_e result;

   static uint8_t strSmsRead[15]; /* string to assemble the SMS read command*/
   uint8_t strAux[5];             /* auxiliary string */

   switch(frmState) {

      case INIT:

         errorOut.errorFrm = OK;
         errorOut.errorCmd.cmd[0] = '\0';
         errorOut.errorCmd.par[0] = '\0';

         procState = ATCMD1;
         frmState = PROC;

         strSmsRead[0] = '\0';

         strncat(strSmsRead, "AT+CMGR=", 8);
         itoa(((smsReadPars_s *)frmInput)->idx, strAux, 10);
         strncat(strSmsRead, strAux, strlen(strAux));
         strncat(strSmsRead, ",", 1);
         itoa(((smsReadPars_s *)frmInput)->mode, strAux, 10);
         strncat(strSmsRead, strAux, strlen(strAux));
         strncat(strSmsRead, "\r", 1);

         debug(">>>interf<<<   strSmsRead: ");
         debug(strSmsRead);
         debug("\r\n");

         break;

      case PROC:

         switch(procState){

            case ATCMD1:

               result = gsmSendCmd("AT+CSDH=1\r");
               if(OK_CMD_SENT == result){procState = ATCMD1RESP;}
               else{errorOut.errorFrm = ERR_PROC; frmState = WRAP;}
               break;

            case ATCMD1RESP:

               result = gsmProcessTkn();
               if(NO_UPDATE != result){
                  if(OK_CMD_ACK <= result && OK_URC >= result){;}
                  else if(OK_CLOSE == result){procState = ATCMD2;}
                  else if(ERR_MSG_CLOSE == result){{errorOut.errorFrm = ERR_GSM; frmState = WRAP;};}
                  else{errorOut.errorFrm = ERR_PROC; frmState = WRAP;}
               }
               break;

            case ATCMD2:

               result = gsmSendCmd(strSmsRead);
               if(OK_CMD_SENT == result){procState = ATCMD2RESP;}
               else{errorOut.errorFrm = ERR_PROC; frmState = WRAP;}
               break;

            case ATCMD2RESP:

               result = gsmProcessTkn();
               if(NO_UPDATE != result){
                  if(OK_CMD_ACK <= result && OK_URC >= result){;}
                  else if(OK_CLOSE == result){frmState = WRAP;}
                  else if(ERR_MSG_CLOSE == result){{errorOut.errorFrm = ERR_GSM; frmState = WRAP;};}
                  else{errorOut.errorFrm = ERR_PROC; frmState = WRAP;}
               }
               break;

         }

         break;

      case WRAP:

         if(OK == errorOut.errorFrm){

            rsp_t rsp;

            rsp = gsmGetCmdRsp();
            strncpy(((smsRec_s *)frmOutput)->meta, rsp.par, strlen(rsp.par));
            ((smsRec_s *)frmOutput)->meta[strlen(rsp.par)] = '\0';

            rsp = gsmGetCmdRsp();
            strncpy(((smsRec_s *)frmOutput)->text, rsp.par, strlen(rsp.par));
            ((smsRec_s *)frmOutput)->text[strlen(rsp.par)] = '\0';

         }

         else if(ERR_GSM == errorOut.errorFrm){

            rsp_t rsp;

            rsp = gsmGetCmdRsp();
            strncpy(errorOut.errorCmd.cmd, rsp.cmd, 19);
            errorOut.errorCmd.cmd[20] = '\0';
            strncpy(errorOut.errorCmd.par, rsp.par, 149);
            errorOut.errorCmd.par[150] = '\0';

         }

         smsListRet_s msgVector; /* Message vector includes the actual array
                                    and the number of messages */

         msgVector.msgs = frmOutput;
         msgVector.noMsgs = 1;

         frmCback(errorOut, &msgVector);
         frmState = IDLE;

         break;
   }

   return;

}

static void gsmSmsListF (void)
{
   static procStatus_e procState = NOCMD;
   static errorUser_s errorOut;

   fsmEvent_e result;

   static uint8_t strSmsList[24]; /* string to assemble the SMS list command*/
   static uint8_t strAux[13];      /* auxiliary string */

   switch(frmState) {

      case INIT:

         errorOut.errorFrm = OK;
         errorOut.errorCmd.cmd[0] = '\0';
         errorOut.errorCmd.par[0] = '\0';

         procState = ATCMD1;
         frmState = PROC;

         strSmsList[0] = '\0';

         strncat(strSmsList, "AT+CMGL=", 8);

         switch(((smsListPars_s *)frmInput)->stat){

            case REC_UNREAD:

               strncpy(strAux,"\"REC UNREAD\"",strlen("\"REC UNREAD\""));

               break;

            case REC_READ:

               strncpy(strAux,"\"REC READ\"",strlen("\"REC READ\""));

               break;

            case STO_UNSENT:

               strncpy(strAux,"\"STO UNSENT\"",strlen("\"STO UNSENT\""));

               break;

            case STO_SENT:

               strncpy(strAux,"\"STO SENT\"",strlen("\"STO SENT\""));

               break;

            case ALL:

               strncpy(strAux,"\"ALL\"",strlen("\"ALL\""));

               break;

         }

         strncat(strSmsList, strAux, strlen(strAux));
         strncat(strSmsList, ",", 1);
         itoa(((smsListPars_s *)frmInput)->mode, strAux, 10);
         strncat(strSmsList, strAux, strlen(strAux));
         strncat(strSmsList, "\r", 1);

         debug(">>>interf<<<   strSmsList: ");
         debug(strSmsList);
         debug("\r\n");

         break;

      case PROC:

         switch(procState){

            case ATCMD1:

               result = gsmSendCmd("AT+CSDH=1\r");
               if(OK_CMD_SENT == result){procState = ATCMD1RESP;}
               else{errorOut.errorFrm = ERR_PROC; frmState = WRAP;}
               break;

            case ATCMD1RESP:

               result = gsmProcessTkn();
               if(NO_UPDATE != result){
                  if(OK_CMD_ACK <= result && OK_URC >= result){;}
                  else if(OK_CLOSE == result){procState = ATCMD2;}
                  else if(ERR_MSG_CLOSE == result){{errorOut.errorFrm = ERR_GSM; frmState = WRAP;};}
                  else{errorOut.errorFrm = ERR_PROC; frmState = WRAP;}
               }
               break;

            case ATCMD2:

               result = gsmSendCmd(strSmsList);
               if(OK_CMD_SENT == result){procState = ATCMD2RESP;}
               else{errorOut.errorFrm = ERR_PROC; frmState = WRAP;}
               break;

            case ATCMD2RESP:

               result = gsmProcessTkn();
               if(NO_UPDATE != result){
                  if(OK_CMD_ACK <= result && OK_URC >= result){;}
                  else if(OK_CLOSE == result){frmState = WRAP;}
                  else if(ERR_MSG_CLOSE == result){{errorOut.errorFrm = ERR_GSM; frmState = WRAP;};}
                  else{errorOut.errorFrm = ERR_PROC; frmState = WRAP;}
               }
               break;

         }

         break;

      case WRAP:

         ;
         uint8_t rspNo = 0;

         if(OK == errorOut.errorFrm){

            uint8_t i = 0;
            uint8_t auxtext[4];

            rsp_t rsp;
            smsRec_s * target = (smsRec_s *)frmOutput;
            rspNo = gsmGetNoCmdRsp();

            // DEBUG ERASE LATER

            debug(">>>interf<<<   NO OF RESPONSES: ");
            itoa(rspNo, auxtext, 10);
            debug(auxtext);
            debug("\r\n");

            debug(">>>interf<<<   VECTOR SIZE: ");
            itoa(((smsListPars_s *)frmInput)->listSize, auxtext, 10);
            debug(auxtext);
            debug("\r\n");
            debug(">>>interf<<<   NO OF SMSs: ");
            itoa((rspNo-1)/2, auxtext, 10);
            debug(auxtext);
            debug("\r\n");

            // DEBUG ERASE LATER

            if(((smsListPars_s *)frmInput)->listSize < ((rspNo-1)/2)){
               errorOut.errorFrm = ERR_WRAP;
               debug(">>>interf<<<   ERROR: Not enough space for available SMSs\r\n");
            }

            else {

               if(1 == rspNo){debug(">>>interf<<<   NO SMSs TO READ\r\n");}

               else{

                  for(i = 0; i < (rspNo-1)/2; i++){

                     rsp = gsmGetCmdRsp();
                     strncpy((target+i)->meta, rsp.par, strlen(rsp.par));
                     (target+i)->meta[strlen(rsp.par)] = '\0';

                     rsp = gsmGetCmdRsp();
                     strncpy((target+i)->text, rsp.par, strlen(rsp.par));
                     (target+i)->text[strlen(rsp.par)] = '\0';

                  }

               }

            }

         }

         else if(ERR_GSM == errorOut.errorFrm){

            rsp_t rsp;

            rsp = gsmGetCmdRsp();
            strncpy(errorOut.errorCmd.cmd, rsp.cmd, 19);
            errorOut.errorCmd.cmd[20] = '\0';
            strncpy(errorOut.errorCmd.par, rsp.par, 149);
            errorOut.errorCmd.par[150] = '\0';

         }

         smsListRet_s msgVector; /* Message vector includes the actual array and
                                   the number of messages */

         msgVector.msgs = frmOutput;
         msgVector.noMsgs = (rspNo-1)/2;

         frmCback(errorOut, &msgVector);
         frmState = IDLE;

         break;
   }

   return;

}

static void gsmSmsDelF (void)
{
   static procStatus_e procState = NOCMD;
   static errorUser_s errorOut;

   uint8_t aux[5]; /* aux buffer */
   static uint8_t smsDel[20];  /* holds the str of the sms del cmd including paramenters */

   fsmEvent_e result;

   switch(frmState) {

      case INIT:

         aux[0] = '\0';
         smsDel[0] = '\0';

         errorOut.errorFrm = OK;
         errorOut.errorCmd.cmd[0] = '\0';
         errorOut.errorCmd.par[0] = '\0';

         strncat(smsDel, "AT+CMGD=", 8);
         itoa(((smsDel_s *)frmInput)->idx, aux, 10);
         strncat(smsDel, aux, strlen(aux));
         strncat(smsDel, ",", 1);
         itoa(((smsDel_s *)frmInput)->mode, aux, 10);
         strncat(smsDel, aux, strlen(aux));
         strncat(smsDel, "\r", 1);

         procState = ATCMD1;
         frmState = PROC;

         break;

      case PROC:

         switch(procState){

            case ATCMD1:

               result = gsmSendCmd(smsDel);
               if(OK_CMD_SENT == result){procState = ATCMD1RESP;}
               else{errorOut.errorFrm = ERR_PROC; frmState = WRAP;}
               break;

            case ATCMD1RESP:

               result = gsmProcessTkn();
               if(NO_UPDATE != result){
                  if(OK_CMD_ACK <= result && OK_URC >= result){;}
                  else if(OK_CLOSE == result){frmState = WRAP;}
                  else if(ERR_MSG_CLOSE == result){{errorOut.errorFrm = ERR_GSM; frmState = WRAP;};}
                  else{errorOut.errorFrm = ERR_PROC; frmState = WRAP;}
               }
               break;

         }

         break;

      case WRAP:

         if(OK != errorOut.errorFrm){
            if(ERR_GSM == errorOut.errorFrm){

               rsp_t rsp;

               rsp = gsmGetCmdRsp();
               strncpy(errorOut.errorCmd.cmd, rsp.cmd, 19);
               errorOut.errorCmd.cmd[20] = '\0';
               strncpy(errorOut.errorCmd.par, rsp.par, 149);
               errorOut.errorCmd.par[150] = '\0';

            }
         }

         frmCback(errorOut, 0);
         frmState = IDLE;
         break;
   }

   return;
}

/*---------------------------------------------------------------------------*/
/*                             GPRS functions                                */
/*---------------------------------------------------------------------------*/

static void gsmGprsStartF (void)
{
   static procStatus_e procState = NOCMD;
   static errorUser_s errorOut;

   static uint8_t APNstring[100];  /* holds the str for the AT+CSTT command */

   fsmEvent_e result;

   switch(frmState) {

      case INIT:

         APNstring[0] = '\0';

         errorOut.errorFrm = OK;
         errorOut.errorCmd.cmd[0] = '\0';
         errorOut.errorCmd.par[0] = '\0';

         strncat(APNstring, "AT+CSTT=\"", 9);
         strncat(APNstring, ((apnUserPwd_s *)frmInput)->apn, 30);
         strncat(APNstring, "\",\"", 3);
         strncat(APNstring, ((apnUserPwd_s *)frmInput)->user, 30);
         strncat(APNstring, "\",\"", 3);
         strncat(APNstring, ((apnUserPwd_s *)frmInput)->pwd, 30);
         strncat(APNstring, "\"\r", 2);

         procState = ATCMD1;
         frmState = PROC;

         break;

      case PROC:

         switch(procState){

            case ATCMD1:

               result = gsmSendCmd("AT+CIPSHUT\r");
               if(OK_CMD_SENT == result){procState = ATCMD1RESP;}
               else{errorOut.errorFrm = ERR_PROC; frmState = WRAP;}
               break;

            case ATCMD1RESP:

               result = gsmProcessTkn();
               if(NO_UPDATE != result){
                  if(OK_CMD_ACK <= result && OK_URC >= result){;}
                  else if(OK_CLOSE == result){procState = ATCMD2;}
                  else if(ERR_MSG_CLOSE == result){{errorOut.errorFrm = ERR_GSM; frmState = WRAP;};}
                  else{errorOut.errorFrm = ERR_PROC; frmState = WRAP;}
               }
               break;

            case ATCMD2:

               result = gsmSendCmd("AT+CIPMODE=1\r");
               if(OK_CMD_SENT == result){procState = ATCMD2RESP;}
               else{errorOut.errorFrm = ERR_PROC; frmState = WRAP;}
               break;

            case ATCMD2RESP:

               result = gsmProcessTkn();
               if(NO_UPDATE != result){
                  if(OK_CMD_ACK <= result && OK_URC >= result){;}
                  else if(OK_CLOSE == result){procState = ATCMD3;}
                  else if(ERR_MSG_CLOSE == result){{errorOut.errorFrm = ERR_GSM; frmState = WRAP;};}
                  else{errorOut.errorFrm = ERR_PROC; frmState = WRAP;}
               }
               break;

            case ATCMD3:

               result = gsmSendCmd(APNstring);
               if(OK_CMD_SENT == result){procState = ATCMD3RESP;}
               else{errorOut.errorFrm = ERR_PROC; frmState = WRAP;}
               break;

            case ATCMD3RESP:

               result = gsmProcessTkn();
               if(NO_UPDATE != result){
                  if(OK_CMD_ACK <= result && OK_URC >= result){;}
                  else if(OK_CLOSE == result){procState = ATCMD4;}
                  else if(ERR_MSG_CLOSE == result){{errorOut.errorFrm = ERR_GSM; frmState = WRAP;};}
                  else{errorOut.errorFrm = ERR_PROC; frmState = WRAP;}
               }
               break;

            case ATCMD4:

               result = gsmSendCmd("AT+CIICR\r");
               if(OK_CMD_SENT == result){procState = ATCMD4RESP;}
               else{errorOut.errorFrm = ERR_PROC; frmState = WRAP;}
               break;

            case ATCMD4RESP:

               result = gsmProcessTkn();
               if(NO_UPDATE != result){
                  if(OK_CMD_ACK <= result && OK_URC >= result){;}
                  else if(OK_CLOSE == result){procState = ATCMD5;}
                  else if(ERR_MSG_CLOSE == result){{errorOut.errorFrm = ERR_GSM; frmState = WRAP;};}
                  else{errorOut.errorFrm = ERR_PROC; frmState = WRAP;}
               }
               break;

            case ATCMD5:

               result = gsmSendCmd("AT+CIFSR\r");
               if(OK_CMD_SENT == result){procState = ATCMD5RESP;}
               else{errorOut.errorFrm = ERR_PROC; frmState = WRAP;}
               break;

            case ATCMD5RESP:

               result = gsmProcessTkn();
               if(NO_UPDATE != result){
                  if(OK_CMD_ACK <= result && OK_URC >= result){;}
                  else if(OK_CLOSE == result){frmState = WRAP;}
                  else if(ERR_MSG_CLOSE == result){{errorOut.errorFrm = ERR_GSM; frmState = WRAP;};}
                  else{errorOut.errorFrm = ERR_PROC; frmState = WRAP;}
               }
               break;

         }

         break;

      case WRAP:

         if(OK != errorOut.errorFrm){
            if(ERR_GSM == errorOut.errorFrm){

               rsp_t rsp;

               rsp = gsmGetCmdRsp();
               strncpy(errorOut.errorCmd.cmd, rsp.cmd, 19);
               errorOut.errorCmd.cmd[20] = '\0';
               strncpy(errorOut.errorCmd.par, rsp.par, 149);
               errorOut.errorCmd.par[150] = '\0';

            }
         }

         frmCback(errorOut, 0);
         frmState = IDLE;

         break;
   }

   return;
}

static void gsmGprsOpenPortF (void)
{
   static procStatus_e procState = NOCMD;
   static errorUser_s errorOut;

   static uint8_t port_string[100];  /* str for the AT+CIPSTART command */

   fsmEvent_e result;

   switch(frmState) {

      case INIT:

         port_string[0] = '\0';

         errorOut.errorFrm = OK;
         errorOut.errorCmd.cmd[0] = '\0';
         errorOut.errorCmd.par[0] = '\0';

         strncat(port_string, "AT+CIPSTART=\"", 13);

         if(TCP == ((port_s *)frmInput)->type){
            strncat(port_string, "TCP\",\"", 6);
         }
         else if(UDP == ((port_s *)frmInput)->type){
            strncat(port_string, "UDP\",\"", 6);
         }

         strncat(port_string, ((port_s *)frmInput)->address, 100);
         strncat(port_string, "\",\"", 3);
         strncat(port_string, ((port_s *)frmInput)->port, 6);
         strncat(port_string, "\"\r", 2);

         procState = ATCMD1;
         frmState = PROC;

         break;

      case PROC:

         switch(procState){

            case ATCMD1:

               result = gsmSendCmd("AT+CIPCLOSE=0\r");
               if(OK_CMD_SENT == result){procState = ATCMD1RESP;}
               else{errorOut.errorFrm = ERR_PROC; frmState = WRAP;}
               break;

            case ATCMD1RESP:

               result = gsmProcessTkn();
               if(NO_UPDATE != result){
                  if(OK_CMD_ACK <= result && OK_URC >= result){;}
                  else if(OK_CLOSE == result){procState = ATCMD2;}
                  else if(ERR_MSG_CLOSE == result){procState = ATCMD2;}
                  /* Operation is not stopped if AT+CIPCLOSE returns an error code,
                   * as there may simply be no open port to close. The command is
                   * sent as a precaution. */
                  else{errorOut.errorFrm = ERR_PROC; frmState = WRAP;}
               }
               break;

            case ATCMD2:

               result = gsmSendCmd(port_string);
               if(OK_CMD_SENT == result){procState = ATCMD2RESP;}
               else{errorOut.errorFrm = ERR_PROC; frmState = WRAP;}
               break;

            case ATCMD2RESP:

               result = gsmProcessTkn();
               if(NO_UPDATE != result){
                  if(OK_CMD_ACK <= result && OK_URC >= result){;}
                  else if(OK_CLOSE == result){frmState = WRAP;}
                  else if(ERR_MSG_CLOSE == result){{errorOut.errorFrm = ERR_GSM; frmState = WRAP;};}
                  else{errorOut.errorFrm = ERR_PROC; frmState = WRAP;}
               }
               break;

         }

         break;

      case WRAP:

         if(OK != errorOut.errorFrm){
            if(ERR_GSM == errorOut.errorFrm){

               rsp_t rsp;

               rsp = gsmGetCmdRsp();
               strncpy(errorOut.errorCmd.cmd, rsp.cmd, 19);
               errorOut.errorCmd.cmd[20] = '\0';
               strncpy(errorOut.errorCmd.par, rsp.par, 149);
               errorOut.errorCmd.par[150] = '\0';

            }
         }

         else{

            gsmSetSerialMode(DATA_MODE);

         }

         frmCback(errorOut, 0);
         frmState = IDLE;

         break;
   }

   return;
}

static void gsmGprsClosePortF (void)
{
   static procStatus_e procState = NOCMD;
   static errorUser_s errorOut;

   fsmEvent_e result;

   switch(frmState) {

      case INIT:

         errorOut.errorFrm = OK;
         errorOut.errorCmd.cmd[0] = '\0';
         errorOut.errorCmd.par[0] = '\0';

         procState = ATCMD1;
         frmState = PROC;

         break;

      case PROC:

         switch(procState){

            case ATCMD1:

               result = gsmSendCmd("AT+CIPCLOSE=0\r");
               if(OK_CMD_SENT == result){procState = ATCMD1RESP;}
               else{errorOut.errorFrm = ERR_PROC; frmState = WRAP;}
               break;

            case ATCMD1RESP:

               result = gsmProcessTkn();
               if(NO_UPDATE != result){
                  if(OK_CMD_ACK <= result && OK_URC >= result){;}
                  else if(OK_CLOSE == result){frmState = WRAP;}
                  else if(ERR_MSG_CLOSE == result){{errorOut.errorFrm = ERR_GSM; frmState = WRAP;};}
                  else{errorOut.errorFrm = ERR_PROC; frmState = WRAP;}
               }
               break;

         }

         break;

      case WRAP:

         if(OK != errorOut.errorFrm){
            if(ERR_GSM == errorOut.errorFrm){

               rsp_t rsp;

               rsp = gsmGetCmdRsp();
               strncpy(errorOut.errorCmd.cmd, rsp.cmd, 19);
               errorOut.errorCmd.cmd[20] = '\0';
               strncpy(errorOut.errorCmd.par, rsp.par, 149);
               errorOut.errorCmd.par[150] = '\0';

            }
         }

         gsmSetSerialMode(COMMAND_MODE);

         frmCback(errorOut, 0);
         frmState = IDLE;

         break;
   }

   return;
}

/*---------------------------------------------------------------------------*/
/*                             GNSS functions                                */
/*---------------------------------------------------------------------------*/

static void gsmGnssPwrF (void)
{
   static procStatus_e procState = NOCMD;
   static errorUser_s errorOut;

   static uint8_t cmdStr[14]; /* string to determine ON or OFF command */

   fsmEvent_e result;

   switch(frmState) {

      case INIT:

         errorOut.errorFrm = OK;
         errorOut.errorCmd.cmd[0] = '\0';
         errorOut.errorCmd.par[0] = '\0';

         procState = ATCMD1;
         frmState = PROC;

         if(ON == *((pwrGnss_e *)frmInput)){
            strncpy(cmdStr,"AT+CGNSPWR=1\r",14);
            debug(cmdStr);
         }
         else if(OFF == *((pwrGnss_e *)frmInput)){
            strncpy(cmdStr,"AT+CGNSPWR=0\r",14);
            debug(cmdStr);
         }

         break;

      case PROC:

         switch(procState){

            case ATCMD1:

               result = gsmSendCmd(cmdStr);
               if(OK_CMD_SENT == result){procState = ATCMD1RESP;}
               else{errorOut.errorFrm = ERR_PROC; frmState = WRAP;}
               break;

            case ATCMD1RESP:

               result = gsmProcessTkn();
               if(NO_UPDATE != result){
                  if(OK_CMD_ACK <= result && OK_URC >= result){;}
                  else if(OK_CLOSE == result){frmState = WRAP;}
                  else if(ERR_MSG_CLOSE == result){{errorOut.errorFrm = ERR_GSM; frmState = WRAP;};}
                  else{errorOut.errorFrm = ERR_PROC; frmState = WRAP;}
               }
               break;

         }

         break;

      case WRAP:

         if(OK != errorOut.errorFrm){
            if(ERR_GSM == errorOut.errorFrm){

               rsp_t rsp;

               rsp = gsmGetCmdRsp();
               strncpy(errorOut.errorCmd.cmd, rsp.cmd, 19);
               errorOut.errorCmd.cmd[20] = '\0';
               strncpy(errorOut.errorCmd.par, rsp.par, 149);
               errorOut.errorCmd.par[150] = '\0';

            }
         }

         frmCback(errorOut, 0);
         frmState = IDLE;

         break;
   }

   return;

}

static void gsmGnssGetDataF (void)
{
   static procStatus_e procState = NOCMD;
   static errorUser_s errorOut;

   fsmEvent_e result;

   switch(frmState) {

      case INIT:

         errorOut.errorFrm = OK;
         errorOut.errorCmd.cmd[0] = '\0';
         errorOut.errorCmd.par[0] = '\0';

         procState = ATCMD1;
         frmState = PROC;

         break;

      case PROC:

         switch(procState){

            case ATCMD1:

               result = gsmSendCmd("AT+CGNSINF\r");
               if(OK_CMD_SENT == result){procState = ATCMD1RESP;}
               else{errorOut.errorFrm = ERR_PROC; frmState = WRAP;}
               break;

            case ATCMD1RESP:

               result = gsmProcessTkn();
               if(NO_UPDATE != result){
                  if(OK_CMD_ACK <= result && OK_URC >= result){;}
                  else if(OK_CLOSE == result){frmState = WRAP;}
                  else if(ERR_MSG_CLOSE == result){{errorOut.errorFrm = ERR_GSM; frmState = WRAP;};}
                  else{errorOut.errorFrm = ERR_PROC; frmState = WRAP;}
               }
               break;

         }

         break;

      case WRAP:

         if(OK == errorOut.errorFrm){

            rsp_t rsp;

            rsp = gsmGetCmdRsp(); /* Get the navigation info string */

            /* Copy the navigation info string to the provided output */

            strncpy((uint8_t *)frmOutput,&rsp.par[0],95);
            ((uint8_t *)frmOutput)[94] = '\0';

            debug(">>>interf<<<   NavInfo String:");
            debug(((uint8_t *)frmOutput));
            debug(" \r\n");

         }

         else if(ERR_GSM == errorOut.errorFrm){

            rsp_t rsp;

            rsp = gsmGetCmdRsp();
            strncpy(errorOut.errorCmd.cmd, rsp.cmd, 19);
            errorOut.errorCmd.cmd[20] = '\0';
            strncpy(errorOut.errorCmd.par, rsp.par, 149);
            errorOut.errorCmd.par[150] = '\0';

         }

         frmCback(errorOut, frmOutput);
         frmState = IDLE;

         break;
   }

   return;

}

/*==================[external functions definition]==========================*/

/*---------------------------------------------------------------------------*/
/*                  General GSM library operation functions                  */
/*---------------------------------------------------------------------------*/

void gsmStartUp (void * (*cback) (errorUser_s, void *))
{
   frm = gsmStartUpF;
   frmCback = cback;
   frmState = INIT;

   return;
}

void gsmSysTickHandler (void)
{
   if(!gsmToutCntZero) gsmDecToutCnt();
   if(procCount > 0) procCount--;

   return;
}

/** This function needs to be called periodically to process commands and
 *  their responses. The user determines the frequency with which the function
 *  is called by modifying the DELAY_SYSUPD constant. As a general rule, each
 *  invocation of this function processes a single token from the serial port
 *  stream. This keeps the execution short and helps the user decide how much
 *  processor time he wants to allot to the GSM processing engine.
 */

void gsmProcess (void)
{
   if(0 == procCount){

      if (IDLE == frmState){

         if(COMMAND_MODE == gsmGetSerialMode()){
            gsmProcessTkn();
         }

         else if (DATA_MODE == gsmGetSerialMode()){
            gsmPrintData(); // PARA PRUEBA; CAMBIAR DESPUES A ALGO QUE LE PASE EL STREAM AL USUARIO
         }

      }
      else {frm();}

      procCount = DELAY_PROC;

      return;

   }

   else{

      return;

   }

}

uint8_t gsmIsIdle (void)
{
   return(frmState == IDLE);
}

void gsmGetSigQual (sigQual_s * sigQual, void * (*cback) (errorUser_s, void *))
{
   frm = gsmGetSigQualF;
   frmOutput = sigQual;
   frmCback = cback;
   frmState = INIT;

   return;
}

void gsmCheckConn (connStatus_s * status, void * (*cback) (errorUser_s, void *))
{
   frm = gsmCheckConnF;
   frmOutput = status;
   frmCback = cback;
   frmState = INIT;

   return;
}

void gsmSetUrcCbackMode (void (*cback) (uint8_t const * const cmd, uint8_t const * const par))
{
   gsmSetUrcMode(CBACK_MODE);
   gsmSetUrcCback(cback);
   return;
}

void gsmSetUrcManualMode (void)
{
   gsmSetUrcMode(MANUAL_MODE);
   gsmSetUrcCback(0);
   return;
}

/*---------------------------------------------------------------------------*/
/*                              SMS functions                                */
/*---------------------------------------------------------------------------*/

void gsmSmsSend (smsOut_s * msg, smsConf_s * conf, void * (*cback) (errorUser_s, void *))
{
   frm = gsmSmsSendF;
   frmInput = msg;
   frmOutput = conf;
   frmCback = cback;
   frmState = INIT;

   return;
}

void gsmSmsRead (smsRec_s * msg, smsReadPars_s * pars, void * (*cback) (errorUser_s, void *))
{
   frm = gsmSmsReadF;
   frmInput = pars;
   frmOutput = msg;
   frmCback = cback;
   frmState = INIT;

   return;
}

void gsmSmsList (smsRec_s * list, smsListPars_s * pars, void * (*cback) (errorUser_s, void *))
{
   frm = gsmSmsListF;
   frmInput = pars;
   frmOutput = list;
   frmCback = cback;
   frmState = INIT;

   return;
}

void gsmSmsDel (smsDel_s * msgdel, void * (*cback) (errorUser_s, void *))
{
   frm = gsmSmsDelF;
   frmInput = msgdel;
   frmCback = cback;
   frmState = INIT;

   return;
}

/*---------------------------------------------------------------------------*/
/*                             GPRS functions                                */
/*---------------------------------------------------------------------------*/

void gsmGprsStart (apnUserPwd_s * apn, void * (*cback) (errorUser_s, void *))
{
   frm = gsmGprsStartF;
   frmInput = apn;
   frmCback = cback;
   frmState = INIT;

   return;
}

void gsmGprsOpenPort (port_s * port, void * (*cback) (errorUser_s, void *))
{
   frm = gsmGprsOpenPortF;
   frmInput = port;
   frmCback = cback;
   frmState = INIT;

   return;
}

void gsmGprsClosePort (void * (*cback) (errorUser_s, void *))
{
   frm = gsmGprsClosePortF;
   frmCback = cback;
   frmState = INIT;

   return;
}

/*---------------------------------------------------------------------------*/
/*                             GNSS functions                                */
/*---------------------------------------------------------------------------*/

void gsmGnssPwr (pwrGnss_e * cmd, void * (*cback) (errorUser_s, void *))
{
   frm = gsmGnssPwrF;
   frmInput = cmd;
   frmCback = cback;
   frmState = INIT;

   return;
}

void gsmGnssGetData (uint8_t * gnssData, void * (*cback) (errorUser_s, void *))
{
   frm = gsmGnssGetDataF;
   frmOutput = gnssData;
   frmCback = cback;
   frmState = INIT;

   return;
}

/** @} doxygen end group definition */

/*==================[end of file]============================================*/
