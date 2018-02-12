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

#ifndef _CIAAMOBILE_ENGINE_H_
#define _CIAAMOBILE_ENGINE_H_

/** \addtogroup engine engine
 ** @{ */

/*==================[inclusions]=============================================*/

#include "lpc_types.h"
#include "string.h"
#include "ciaaUART_T.h"
#include "ciaaMobile_parser.h"
#include "ciaaMobile_commands.h"

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/

/** @brief Size of the URC events vector */
#define URC_VECTOR_SIZE 10

/** @brief State of the GSM engine */

typedef enum _GSMstates
{
   WAITING = 0,  /**< waiting for a command */
   CMD_SENT = 1, /**< cmd sent through serial port */
   CMD_ACK = 2,  /**< cmd echo confirmed */
} GSMstate;

/** @brief State of the current command */

typedef enum _FSMresult
{
   OK_CMD_SENT = 1,      /**< command sent */
   OK_CMD_ACK = 2,       /**< command acknowledged */
   OK_RESP = 3,          /**< non-closing responde received */
   OK_CLOSE = 4,         /**< closing response received */
   OK_URC = 5,           /**< URC proccessed */
   ERR_CMD_UKN = 5,      /**< unknown command sent */
   ERR_CMD_ECHO = 6,     /**< cmd echo erroneous or missing */
   ERR_OOO = 7,          /**< out of order response received */
   ERR_TKN_INV = 8,       /**< toke invalid */
   ERR_FSM_OOR = 9        /**< FSM out of range */
} FSMresult;

typedef struct _URCevent
{
   uint8_t command[150];
   uint8_t parameter[150];
} URCevent;

/*==================[typedef]================================================*/

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

/** @brief Processes a single token
*
*  @return Returns the result of the latest updateFSM invocation
*/

FSMresult processToken(void);

/** @brief Sends an AT command to the GSM module
*
*  @return Returns the result of the updateFSM invocation
*
*  @param cmdstr AT command string including parameters
*/

FSMresult sendATcmd (const uint8_t * cmdstr);

/** @brief Gets a pointer to the next valid command response
*
*  @param index Index of the required response in respVector
*
*  @return Returns a pointer to the required response (0 if no such response)
*/

uint8_t * getCmdResp (uint8_t index);

/** @brief Gets the number of responses generated by the command
*
*  @return Returns the number of responses generated by the command
*/

uint8_t getNoCmdResp (void);

/** @brief Reads the oldest URC event in the URC event vector
*
*  @param command   Main part of the read URC
*  @param parameter Parameter part of the read URC
*
*  @return Returns 1 if successful or 0 if vector is empty
*/

uint8_t readURC (uint8_t * const command, uint8_t * const parameter);

/*==================[cplusplus]==============================================*/

#ifdef __cplusplus
}
#endif

/** @} doxygen end group definition */
/*==================[end of file]============================================*/
#endif /* #ifndef _MAIN_H_ */
