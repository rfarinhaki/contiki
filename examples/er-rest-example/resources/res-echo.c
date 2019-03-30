/*
 * Copyright (c) 2013, Institute for Pervasive Computing, ETH Zurich
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 */

/**
 * \file
 *      Example resource
 * \author
 *      Matthias Kovatsch <kovatsch@inf.ethz.ch>
 */

#include "contiki.h"

#if PLATFORM_HAS_LEDS

#include <string.h>
#include <stdlib.h>
#include "contiki.h"
#include "rest-engine.h"
#include "er-coap.h"
#include "dev/leds.h"

uint8_t msg_buffer[3][REST_MAX_CHUNK_SIZE];
uint8_t msg_len[3];

static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_post_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);


/* A simple actuator example. Toggles the red led */
RESOURCE(res_echo,
         "title=\"Echo\";rt=\"Control\"",
         res_get_handler,
         res_post_put_handler,
         res_post_put_handler,
         NULL);

static void
res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
    uint32_t i, buffernum=-1;
    const char *cbuffernum;
    uint8_t etag=0;

    //configura o tipo de conteudo da mensagem
    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    //etag eh uma propriedade que eh utilizada pelos servidores de cache para saber se uma mensagem mudou
    //duas mensagens com o mesmo valor devem ter o mesmo etag
    if(REST.get_query_variable(request,"buffernumber",&cbuffernum))
        buffernum = atoi(cbuffernum);

    if(buffernum >= 0 && buffernum < 3){
        for(i=0;i<msg_len[buffernum];i++){
            //nestecasoutilizamosumchecksumsimplescomoetag,masousuariopodeusaroquequiser
            etag += msg_buffer[buffernum][i];
        }

        REST.set_header_etag(response, (uint8_t *)&etag, 1);
        //configuraopayloadaserretornado
        REST.set_response_payload(response, msg_buffer[buffernum], msg_len[buffernum]);
    }else
        REST.set_response_status(response, REST.status.BAD_REQUEST);

}

static void
res_post_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
    int bn;
    const char *cbuffernum, *value;
    //converteopayloadrecebidoporPUTemumpacoteCoAP
    coap_packet_t *const coap_req = (coap_packet_t *)request;
    uint8_t buffer_ptr = 0;//verificaseopayloadenviadonaoehmuitograndeparaarequisicao
    if(coap_req->payload_len > REST_MAX_CHUNK_SIZE || !REST.get_post_variable(request,"buffernumber",&cbuffernum)){
        //caso for muito grande, simplesmente configura a resposta como BAD_REQUEST e retorna
        REST.set_response_status(response, REST.status.BAD_REQUEST);
        return;
    }else{
            bn = atoi(cbuffernum);
            if(bn>3 || bn < 0){
                REST.set_response_status(response, REST.status.BAD_REQUEST);
                return;
            }
            REST.get_post_variable(request,"value",&value);


        //casocontrario,copiaamensagemenviadaparaobuffercriado
        memcpy((void*)msg_buffer[bn], (void*)value, coap_req->payload_len);
        //salvatambemotamanhodamensagemrecebida(parausofuturo)
        msg_len[bn]= strlen(value);
    }

}

#endif /* PLATFORM_HAS_LEDS */
