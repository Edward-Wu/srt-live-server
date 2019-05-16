/*
 * This file is part of SLS Live Server.
 *
 * SLS Live Server is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * SLS Live Server is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with SLS Live Server;
 * if not, please contact with the author: Edward.Wu(edward_email@126.com)
 */

#ifndef _CONF_INCLUDE_
#define _CONF_INCLUDE_


#include <stddef.h>
#include <stdint.h>
#include <vector>
#include <string>

using namespace std;

/*
 * conf file structure
 * srt[root]
 *  |_____ server[child]
 *      |    |_____ app[child]
 *      |       |__ app[sibling]
 *      |_ server[sibling]
 *           |_____ app[child]
 *              |__ record[sibling]
 */

#define SLS_CONF_OK                 NULL
#define SLS_CONF_ERROR              (void *) -1
#define SLS_CONF_OUT_RANGE          "out of range"
#define SLS_CONF_NAME_NOT_EXISTS    "name not exist"
#define SLS_CONF_WRONG_TYPE         "wrong type"

/*
 * conf cmd for set value by name dynamically
 */
struct sls_conf_cmd_t {
    const char          *name;
    const char          *mark;
    int                  offset;
    const char         * (*set)(const char* v, sls_conf_cmd_t *cmd, void *conf);
    double               min;                 ///< minimum valid value for the option
    double               max;                 ///< maximum valid value for the option
};

/*
 * set conf macro
 */
#define SLS_SET_CONF(conf, type, n, m, min, max)\
{ #n,\
  #m,\
  offsetof(sls_conf_##conf##_t, n),\
  sls_conf_set_##type,\
  min,\
  max,\
  }

const char * sls_conf_set_int   (const char *v, sls_conf_cmd_t *cmd, void *conf);
const char * sls_conf_set_string(const char *v, sls_conf_cmd_t *cmd, void *conf);
const char * sls_conf_set_double(const char *v, sls_conf_cmd_t *cmd, void *conf);
const char * sls_conf_set_bool  (const char *v, sls_conf_cmd_t *cmd, void *conf);

/**
 * runtime conf
 * all conf runtime classes are linked, such as frist->next->next->next.
 */
typedef struct sls_conf_base_t sls_conf_base_s;
typedef sls_conf_base_t * (*create_conf_func)();
struct sls_runtime_conf_t {
    char                    * conf_name;
//    char                    * higher_conf_names;//if allow existing in one than one higher conf , split with '|'
    create_conf_func          create_fn;
    sls_conf_cmd_t          * conf_cmd;
    int                       conf_cmd_size;

    sls_runtime_conf_t        * next;
    static sls_runtime_conf_t * first;
    sls_runtime_conf_t(char * c, create_conf_func f, sls_conf_cmd_t * cmd, int len);
};

/*
 * conf base, each actual conf must inherit from it,
 * decare a new conf please use macro SLS_CONF_DYNAMIC_DECLARE_BEGIN
 */
struct sls_conf_base_t {
    char            * name;
    sls_conf_base_t * sibling;
    sls_conf_base_t * child;
};

/**
 * conf dynamic macro
 */
#define SLS_CONF_DYNAMIC_DECLARE_BEGIN(c_n)\
struct sls_conf_##c_n##_t : public sls_conf_base_t {\
    static sls_runtime_conf_t runtime_conf;\
    static sls_conf_base_t * create_conf();

#define SLS_CONF_DYNAMIC_DECLARE_END \
};

#define SLS_CONF_DYNAMIC_IMPLEMENT(c_n)\
sls_conf_base_t * sls_conf_##c_n##_t::create_conf()\
{\
    sls_conf_base_t * p = new sls_conf_##c_n##_t ;\
    memset(p, 0, sizeof(sls_conf_##c_n##_t));\
    p->child     = NULL;\
    p->sibling   = NULL;\
    p->name = sls_conf_##c_n##_t::runtime_conf.conf_name;\
    return p;\
}\
sls_runtime_conf_t   sls_conf_##c_n##_t::runtime_conf(\
    #c_n,\
    sls_conf_##c_n##_t::create_conf,\
    conf_cmd_##c_n,\
    sizeof(conf_cmd_##c_n)/sizeof(sls_conf_cmd_t)\
    );

/*
 * conf cmd dynamic macro
 */
#define SLS_CONF_CMD_DYNAMIC_DECLARE_BEGIN(c_n)\
static sls_conf_cmd_t  conf_cmd_##c_n[] = {

#define SLS_CONF_CMD_DYNAMIC_DECLARE_END \
};

#define SLS_CONF_GET_CONF_INFO(c_name)\
(sls_conf_##c_name *)sls_conf_##c_name::runtime_conf_##c_name.conf_name;

/*
 * conf api functions
 */
int  sls_conf_get_conf_count(sls_conf_base_t *c);
int  sls_conf_open (const char * conf_file);
void sls_conf_close();

sls_conf_cmd_t  * sls_conf_find(const char *n, sls_conf_cmd_t *cmd, int size);
sls_conf_base_t * sls_conf_get_root_conf();
vector<string>    sls_conf_string_split(const string& str, const string& delim);



#endif
