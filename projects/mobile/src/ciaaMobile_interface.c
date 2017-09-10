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

/** @brief This is a simple UART example
 */

/** \addtogroup uart Bare-metal example
 ** @{ */

/*==================[inclusions]=============================================*/

#include "ciaaMobile_interface.h"

/*==================[macros and definitions]=================================*/

/*==================[global data]============================================*/

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

void cb1 (void * input);

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/

void ciaaMobile_sendSMS (void * msg, void (*cback) (void *))
{
   static frmStatus frmState = INIT;
   static uint8_t runState = 0;

   uint8_t dest[20];
   uint8_t text[150];

   switch(frmState) {

      case INIT:

         /*sprintf(dest, "\"%s\"", ((SMS_send *)msg)->dest);   VER SI SE PUEDE INCORPORAR MAS ADELANTE */
         dest[0] = '"';
         dest[1] = '\0';
         strncat(dest, ((SMS_send *)msg)->dest,strlen(((SMS_send *)msg)->dest));
         strncat(dest, "\"", 1);

         strncpy(text, ((SMS_send *)msg)->text, strlen(((SMS_send *)msg)->text));
         text[strlen(((SMS_send *)msg)->text)] = '\0';

         frmState = RUNNING;

         dbgPrint("Destino: ");
         dbgPrint(dest);
         dbgPrint("\r\nMensaje: ");
         dbgPrint(text);
         dbgPrint("\r\n");

         break;

      case RUNNING:

         switch(runState){

            case 0:

               sendATcmd(0,0,AUTOBAUD);
               runState = 1;
               break;

            case 1:

               if(cmdClosed == processToken()){runState = 2;}
               break;

            case 2:

               sendATcmd("CMGF","1",EXT_WRITE);
               runState = 3;
               break;

            case 3:

               if(cmdClosed == processToken()){runState = 4;}
               break;

            case 4:

               sendATcmd("CSCS","\"GSM\"",EXT_WRITE);
               runState = 5;
               break;

            case 5:

               if(cmdClosed == processToken()){runState = 6;}
               break;

            case 6:

               sendATcmd("CMGS",dest,EXT_WRITE);
               runState = 7;
               break;

            case 7:

               if(cmdClosed == processToken()){runState = 8;}
               break;

            case 8:

               sendATcmd("SMS_BODY",text,SMS);
               runState = 9;
               break;

            case 9:

               if(cmdClosed == processToken()){runState = 0; frmState = FINISHED;}
               break;

         }

         break;

      case FINISHED:

         cb1("input");
         break;
   }

   return;

}

void cb1 (void * input){

   SMS_send_ret result = {1,1};

   dbgPrint("CB1 EXECUTED\r\n");

   return;
}

void ciaaMobile_sysUpdate (void)
{
   static SMS_send mensaje = {"1151751809","Hola mundo!"};

   ciaaMobile_sendSMS(&mensaje, &cb1);
}

/** @} doxygen end group definition */

/*==================[end of file]============================================*/