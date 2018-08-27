/***************************************************************************//**
**
**  @file       fifo.c
**  @ingroup    utils
**  @brief      A general purpose first-in/first-out buffer (FIFO).
**  @copyright  Copyright (C) 2012-2018 Tuomas Terho. All rights reserved.
**
**  The maximum capacity of this FIFO is 65535 data items, 65535 bytes each.
**
*******************************************************************************/
/*
**  BSD 3-Clause License
**
**  COPYRIGHT (c) 2012-2018, Tuomas Terho
**  All rights reserved.
**
**  Redistribution and use in source and binary forms, with or without
**  modification, are permitted provided that the following conditions are met:
**  
**  * Redistributions of source code must retain the above copyright notice, 
**    this list of conditions and the following disclaimer.
**  
**  * Redistributions in binary form must reproduce the above copyright notice,
**    this list of conditions and the following disclaimer in the documentation
**    and/or other materials provided with the distribution.
**  
**  * Neither the name of the copyright holder nor the names of its
**    contributors may be used to endorse or promote products derived from
**    this software without specific prior written permission.
**  
**  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
**  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
**  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
**  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
**  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
**  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
**  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
**  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
**  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
**  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
**  POSSIBILITY OF SUCH DAMAGE.
**
*******************************************************************************/

#include "types.h"
#include "fifo.h"

#include <string.h>

/******************************************************************************\
**
**  PUBLIC FUNCTION DECLARATIONS
**
\******************************************************************************/

/*------------------------------------------------------------------------------
**  Initializes a FIFO.
*/
Result_t
FIFO_Init(
        FIFO_t *fifo,
        void *data,
        uint32_t size,
        uint16_t dsize
)
{
        // Check the parameters.
        if(!fifo||!data){
                return FIFO_ERROR_INVALID_POINTER;
        }
        if(!size||!dsize||size<dsize){
                return FIFO_ERROR_INVALID_PARAMETER;
        }
        // Setup the FIFO.
        fifo->d=data;
        fifo->sz=size;
        fifo->dsz=dsize;
        // Calculate the FIFO size in items.
        fifo->max=fifo->sz/fifo->dsz;
        // Reset the FIFO.
        return FIFO_Reset(fifo);
}

/*------------------------------------------------------------------------------
**  Resets a FIFO
*/
Result_t
FIFO_Reset(
        FIFO_t *fifo
)
{
        if(!fifo){
                return FIFO_ERROR_INVALID_POINTER;
        }        
        // Reset the write and read pointers and the item counter.
        fifo->wr=0;
        fifo->rd=0;
        fifo->cnt=0;
        fifo->wrp=fifo->d;
        fifo->rdp=fifo->d;
        return RESULT_OK;
}

/*------------------------------------------------------------------------------
**  Puts a byte to the FIFO
*/
Result_t
FIFO_Put(
        FIFO_t *fifo,
        void *data
)
{
        if(!fifo||!data){
                return FIFO_ERROR_INVALID_POINTER;
        }
        // If there is no room for data, return an error.
        if(fifo->cnt==fifo->max){
                return FIFO_ERROR_FULL; 
        }        
        // Write data to the buffer and advance the write pointers.
        FIFO_ENTER_CRITICAL();
        memcpy(fifo->wrp,data,fifo->dsz);
        fifo->wr++;
        fifo->cnt++;
        fifo->wrp=(uint8_t*)fifo->wrp+fifo->dsz;
        FIFO_EXIT_CRITICAL();
        // Write index and pointer wrap-around.
        if(fifo->wr==fifo->max){
                fifo->wr=0;
                fifo->wrp=fifo->d;
        }  
        return RESULT_OK;
}

/*------------------------------------------------------------------------------
**  Gets a byte from the FIFO
*/
Result_t
FIFO_Get(
        FIFO_t *fifo,
        void *data
)
{
        if(!fifo||!data){
                return FIFO_ERROR_INVALID_POINTER;
        }        
        // If there is no data in the FIFO, return error.
        if(!fifo->cnt){
                return FIFO_ERROR_EMPTY;
        }        
        // Read data from the buffer and advance the read pointers.
        memcpy(data,fifo->rdp,fifo->dsz);
        FIFO_ENTER_CRITICAL();
        fifo->rd++;
        fifo->cnt--;
        fifo->rdp=(uint8_t*)fifo->rdp+fifo->dsz;
        FIFO_EXIT_CRITICAL();
        // Read index and pointer wrap-around.
        if(fifo->rd==fifo->max){
                fifo->rd=0;
                fifo->rdp=fifo->d;
        }  
        return RESULT_OK;
}

/* EOF */