/*****
*
* Copyright (C) 2004 Yoann Vandoorselaere <yoann@prelude-ids.org>
* All Rights Reserved
*
* This file is part of the Prelude program.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; see the file COPYING.  If not, write to
* the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
*
*****/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>

#include "libmissing.h"
#include "prelude-error.h"
#include "prelude-log.h"
#include "prelude-extract.h"
#include "prelude-io.h"
#include "prelude-msgbuf.h"
#include "prelude-client.h"
#include "prelude-message-id.h"
#include "prelude-option.h"
#include "prelude-option-wide.h"
#include "common.h"
#include "config-engine.h"



static int config_save_value(config_t *cfg, int rtype, prelude_option_t *last,
                             char **prev, const char *option, const char *value)
{
        int ret;
        char buf[1024];
        
        if ( ! (prelude_option_get_type(last) & PRELUDE_OPTION_TYPE_CFG) )
                return -1;
        
        if ( rtype != PRELUDE_MSG_OPTION_SET && rtype != PRELUDE_MSG_OPTION_DESTROY )
                return -1;
        
        if ( prelude_option_has_optlist(last) ) {
                
                snprintf(buf, sizeof(buf), "%s=%s", option, (value) ? value : "default");
                
                if ( *prev )
                        free(*prev);
                
                *prev = strdup(buf);
                                
                if ( rtype == PRELUDE_MSG_OPTION_SET )
                        return config_set(cfg, buf, NULL, NULL);
                else
                        return config_del(cfg, buf, NULL);
                        
        }
        
        if ( rtype == PRELUDE_MSG_OPTION_SET ) 
                ret = config_set(cfg, *prev, option, value);
        else
                ret = config_del(cfg, *prev, option);
        
        return ret;
}



static int parse_single(void **context, prelude_option_t **last, int is_last_cmd,
                        int rtype, const char *option, const char *value, prelude_string_t *out)
{
        int ret = 0;
	
        *last = prelude_option_search(*last, option, PRELUDE_OPTION_TYPE_WIDE, 0);
        if ( ! *last ) {
                prelude_string_sprintf(out, "Unknown option: %s.\n", option);
                return -1;
        }

        if ( rtype == PRELUDE_MSG_OPTION_SET )
                ret = prelude_option_invoke_set(context, *last, value, out);
        
        else if ( is_last_cmd ) {
		
                if ( rtype == PRELUDE_MSG_OPTION_DESTROY ) 
                        ret = prelude_option_invoke_destroy(*context, *last, value, out);
                
                else if ( rtype == PRELUDE_MSG_OPTION_GET )
                        ret = prelude_option_invoke_get(*context, *last, value, out);
        
		else if ( rtype == PRELUDE_MSG_OPTION_COMMIT )
			ret = prelude_option_invoke_commit(*context, *last, value, out);
	}
        
        return ret;
}




static int parse_request(prelude_client_t *client, int rtype, char *request, prelude_string_t *out)
{
        config_t *cfg;
        void *context = client;
        char pname[256], iname[256];
        int ret = 0, last_cmd = 0, ent;
        prelude_option_t *last = NULL;
        char *str, *value, *prev = NULL, *ptr = NULL;
        
        ret = config_open(&cfg, prelude_client_get_config_filename(client));
        if ( ret < 0 ) {
                log(LOG_ERR, "error opening %s.\n", prelude_client_get_config_filename(client));
                return -1;
        }
        
        value = request;
        strsep(&value, "=");
        
        while ( (str = (strsep(&request, "."))) ) {
                
                if ( ! request ) {
                        last_cmd = 1;
                        ptr = value;
                }

                *pname = 0;
                *iname = 0;
                
                ent = sscanf(str, "%255[^[][%255[^]]", pname, iname);
                if ( ent <= 0 ) {
                        prelude_string_sprintf(out, "error parsing option path");
                        return -1;
                }

                ret = parse_single(&context, &last, last_cmd, rtype, pname, (ent == 2) ? iname : ptr, out);
                if ( ret < 0 )
                        break;
                
                config_save_value(cfg, rtype, last, &prev, pname, (ent == 2) ? iname : ptr);
        }
        
        config_close(cfg);
        free(prev);
        
        return ret;
}



static int send_msg(prelude_msgbuf_t *msgbuf, prelude_msg_t *msg) 
{
        int ret;
        prelude_io_t *fd = prelude_msgbuf_get_data(msgbuf);

        do {
                ret = prelude_msg_write(msg, fd);
        } while ( ret < 0 && prelude_error_get_code(ret) == PRELUDE_ERROR_EAGAIN );

        return ret;
}



static void send_string(prelude_msgbuf_t *msgbuf, prelude_string_t *out, int type)
{
        size_t len;
        
        len = prelude_string_is_empty(out) ? 0 : (prelude_string_get_len(out) + 1);
        prelude_msgbuf_set(msgbuf, type, len, prelude_string_get_string(out));
}



static int read_option_request(prelude_client_t *client, prelude_msgbuf_t *msgbuf, prelude_msg_t *msg)
{
        void *buf;
        uint8_t tag;
        char *request;
        uint32_t len, hop;
        int ret, type = -1;
        uint32_t request_id;
        prelude_string_t *out;

        out = prelude_string_new();
        if ( ! out )
                return -1;
        
        while ( prelude_msg_get(msg, &tag, &len, &buf) == 0 ) {
                
                switch (tag) {
                        
                case PRELUDE_MSG_OPTION_SET:
                case PRELUDE_MSG_OPTION_GET:
                case PRELUDE_MSG_OPTION_COMMIT:
                case PRELUDE_MSG_OPTION_DESTROY:
                        type = tag;
                        break;

                case PRELUDE_MSG_OPTION_HOP:
                        ret = prelude_extract_uint32_safe(&hop, buf, len);
                        if ( ret < 0 )
                                return ret;
                        
                        hop = htonl(hop - 1);
                        prelude_msgbuf_set(msgbuf, tag, len, &hop);
                        break;
                        
                case PRELUDE_MSG_OPTION_TARGET_ID:                        
                        prelude_msgbuf_set(msgbuf, tag, len, buf);
                        break;
                                                
                case PRELUDE_MSG_OPTION_REQUEST_ID:
                        ret = prelude_extract_uint32_safe(&request_id, buf, len);
                        if ( ret < 0 )
                                return ret;
                        
                        prelude_msgbuf_set(msgbuf, PRELUDE_MSG_OPTION_REQUEST_ID, len, buf);
                        break;
                        
                case PRELUDE_MSG_OPTION_LIST:
                        return prelude_option_wide_send_msg(client, msgbuf); 

                case PRELUDE_MSG_OPTION_VALUE:
                        ret = prelude_extract_characters_safe((const char **) &request, buf, len);
                        if (ret < 0 )
                                return ret;
                                                
                        if ( type < 0 ) {
                                prelude_string_sprintf(out, "no request type specified.\n");
                                send_string(msgbuf, out, PRELUDE_MSG_OPTION_ERROR);
                                return -1;
                        }
                        
                        ret = parse_request(client, type, request, out);
                        if ( ret < 0 )
                                send_string(msgbuf, out, PRELUDE_MSG_OPTION_ERROR);
				
                        else send_string(msgbuf, out, PRELUDE_MSG_OPTION_VALUE);
                        
                        break;
                        
                default:
                        prelude_string_sprintf(out, "unknown option tag: %d.\n", tag);
                        send_string(msgbuf, out, PRELUDE_MSG_OPTION_ERROR);
                        return -1;
                }
        }
        
        return 0;
}



static int handle_option_request(prelude_client_t *client, prelude_io_t *fd, prelude_msg_t *msg)
{
        int ret = 0;
        prelude_msgbuf_t *msgbuf;
            
        ret = prelude_msgbuf_new(&msgbuf);
        if ( ret < 0 ) 
                return ret;
        
        prelude_msgbuf_set_data(msgbuf, fd);
        prelude_msgbuf_set_callback(msgbuf, send_msg);
        prelude_msg_set_tag(prelude_msgbuf_get_msg(msgbuf), PRELUDE_MSG_OPTION_REPLY);
        
        ret = read_option_request(client, msgbuf, msg);
        prelude_msgbuf_destroy(msgbuf);
        
        return ret;
}



static int read_option_list(prelude_msg_t *msg, prelude_option_t *opt, uint64_t *source_id) 
{
        int ret;
        void *buf;
        uint32_t dlen;
        const char *tmp;
        uint8_t tag, tmpint;
        
        if ( ! opt )
                return -1;
        
        while ( (ret = prelude_msg_get(msg, &tag, &dlen, &buf)) == 0 ) {
                
                switch (tag) {
                
                case PRELUDE_MSG_OPTION_START:
                        read_option_list(msg, prelude_option_new(opt), source_id);
                        break;
                        
                case PRELUDE_MSG_OPTION_END:
                        return 0;
                        
                case PRELUDE_MSG_OPTION_VALUE:
                        ret = prelude_extract_characters_safe(&tmp, buf, dlen);
                        if ( ret < 0 ) 
                                return ret;

                        prelude_option_set_value(opt, tmp);
                        break;

                case PRELUDE_MSG_OPTION_NAME:
                        ret = prelude_extract_characters_safe(&tmp, buf, dlen);
                        if ( ret < 0 )
                                return ret;

                        prelude_option_set_longopt(opt, tmp);
                        break;
                        
                case PRELUDE_MSG_OPTION_DESC:
                        ret = prelude_extract_characters_safe(&tmp, buf, dlen);
                        if ( ret < 0 )
                                return ret;

                        prelude_option_set_description(opt, tmp);
                        break;
                        
                case PRELUDE_MSG_OPTION_HELP:
                        ret = prelude_extract_characters_safe(&tmp, buf, dlen);
                        if ( ret < 0 )
                                return ret;
                        
                        prelude_option_set_help(opt, tmp);
                        break;
                        
                case PRELUDE_MSG_OPTION_INPUT_VALIDATION:
                        ret = prelude_extract_characters_safe(&tmp, buf, dlen);
                        if ( ret < 0 )
                                return ret;
                        
                        prelude_option_set_input_validation_regex(opt, tmp);
                        break;

                case PRELUDE_MSG_OPTION_HAS_ARG:
                        ret = prelude_extract_uint8_safe(&tmpint, buf, dlen);
                        if ( ret < 0 )
                                return ret;

                        prelude_option_set_has_arg(opt, tmpint);
                        break;

                case PRELUDE_MSG_OPTION_TYPE:
                        ret = prelude_extract_uint8_safe(&tmpint, buf, dlen);
                        if ( ret < 0 )
                                return ret;

                        prelude_option_set_type(opt, tmpint);
                        break;
                        
                case PRELUDE_MSG_OPTION_INPUT_TYPE:
                        ret = prelude_extract_uint8_safe(&tmpint, buf, dlen);
                        if ( ret < 0 )
                                return ret;
                        
                        break;
                        
                default:
                        /*
                         * for compatibility purpose, don't return an error on unknow tag.
                         */
                        log(LOG_ERR, "unknow option tag %d.\n", tag);
                }
        }

        return 0;
}




int prelude_option_process_request(prelude_client_t *client, prelude_io_t *fd, prelude_msg_t *msg)
{
        uint8_t tag;
        
        tag = prelude_msg_get_tag(msg);
        
        if ( tag != PRELUDE_MSG_OPTION_REQUEST )
                return -1;
        
        return handle_option_request(client, fd, msg);
}




int prelude_option_push_request(prelude_msgbuf_t *msg, int type, const char *request)
{        
        prelude_msgbuf_set(msg, type, 0, 0);
        
        if ( request ) 
                prelude_msgbuf_set(msg, PRELUDE_MSG_OPTION_VALUE, strlen(request) + 1, request);
                
        return 0;
}



int prelude_option_new_request(prelude_msgbuf_t *msgbuf,
                               uint32_t request_id, uint64_t *target_id, size_t size)
{
        int i;
        uint32_t hop;
                                
        prelude_msg_set_tag(prelude_msgbuf_get_msg(msgbuf), PRELUDE_MSG_OPTION_REQUEST);

        /*
         * the caller is supposed to provide a full path,
         * from him to the destination, to the original hop is 1.
         */
        hop = htonl(1);

        for ( i = 0; i < size; i++ )
                target_id[i] = prelude_hton64(target_id[i]);
        
        request_id = htonl(request_id);
        prelude_msgbuf_set(msgbuf, PRELUDE_MSG_OPTION_REQUEST_ID, sizeof(request_id), &request_id);
        prelude_msgbuf_set(msgbuf, PRELUDE_MSG_OPTION_TARGET_ID, i * sizeof(*target_id), target_id);
        prelude_msgbuf_set(msgbuf, PRELUDE_MSG_OPTION_HOP, sizeof(hop), &hop);
        
        return 0;
}



int prelude_option_recv_reply(prelude_msg_t *msg, uint64_t *source_id, uint32_t *request_id, void **value)
{
        void *buf;
        uint8_t tag;
        uint32_t dlen;
        int ret, type = -1;
        
        *value = NULL;

        while ( (ret = prelude_msg_get(msg, &tag, &dlen, &buf)) == 0 ) {

                switch (tag) {
                case PRELUDE_MSG_OPTION_HOP:
                        break;
                        
                case PRELUDE_MSG_OPTION_REQUEST_ID:
                        type = PRELUDE_OPTION_REPLY_TYPE_SET;
                        
                        ret = prelude_extract_uint32_safe(request_id, buf, dlen);
                        if ( ret < 0 )
                                return ret;
                        
                        break;
                        
                case PRELUDE_MSG_OPTION_VALUE:
                        type = PRELUDE_OPTION_REPLY_TYPE_GET;
                        
                        ret = prelude_extract_characters_safe((const char **) value, buf, dlen);                        
                        if ( ret < 0 )
                                return ret;
                        break;

                case PRELUDE_MSG_OPTION_ERROR:
                        type = PRELUDE_OPTION_REPLY_TYPE_ERROR;
                        if ( ! dlen ) {
                                *value = "No error message";	
                                break;
                        }
			
                        ret = prelude_extract_characters_safe((const char **) value, buf, dlen);
                        if ( ret < 0 )
                                return ret;
                        break;
                                                
                case PRELUDE_MSG_OPTION_TARGET_ID:
                        break;

                case PRELUDE_MSG_OPTION_LIST:                        
                        type = PRELUDE_OPTION_REPLY_TYPE_LIST;
                        *value = prelude_option_new(NULL);
                        
                        ret = read_option_list(msg, *value, NULL);
                        if ( ret < 0 )
                                return ret;
                        break;
                        
                default:
                        log(LOG_ERR, "unknow tag : %d.\n", tag);
                        return -1;
                }
        }
        
        return type;
}
