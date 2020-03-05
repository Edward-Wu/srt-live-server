
/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2019-2020 Edward.Wu
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <fstream>

#include "conf.hpp"
#include "common.hpp"
#include "SLSLog.hpp"



sls_conf_base_t  sls_first_conf = {"", NULL, NULL} ;
sls_runtime_conf_t * sls_runtime_conf_t::first = NULL;

/*
 * runtime conf
 */
sls_runtime_conf_t::sls_runtime_conf_t(char * c, create_conf_func f, sls_conf_cmd_t * cmd, int len)
{
    conf_name      = c;
    create_fn      = f;
    conf_cmd       = cmd;
    conf_cmd_size  = len;
    next           = NULL;

    this->next     = first;
    first          = this;
}

sls_conf_cmd_t * sls_conf_find(const char *n, sls_conf_cmd_t *cmd, int size)
{
    for (int i = 0; i < size; i ++) {
        if (strcmp(n, cmd->name) == 0) {
            return cmd ;
        }
        cmd ++;
    }
    return NULL;
}

const char * sls_conf_set_int(const char *v, sls_conf_cmd_t *cmd, void *conf)
{
    char  * p = (char *)conf;
    int     v1;
    int   * np;
    char  * value;

    np = (int *) (p + cmd->offset);

    v1 = atoi(v);
    if (v1 < cmd->min || v1 > cmd->max)
        return SLS_CONF_OUT_RANGE;
    *np = v1;
    return SLS_CONF_OK;
}

const char * sls_conf_set_string(const char *v, sls_conf_cmd_t *cmd, void *conf)
{
    char  * p = (char *)conf;
    char  * np;
    int     len = strlen(v);

    if (len < cmd->min || len > cmd->max)
        return SLS_CONF_OUT_RANGE;

    np = (char *) (p + cmd->offset);
    memcpy(np, v, len);
    np[len] = 0;
    return SLS_CONF_OK;
}

const char * sls_conf_set_double(const char *v, sls_conf_cmd_t *cmd, void *conf)
{
    char   * p = (char *)conf;
    double   v1;
    double * np;
    char   * value;

    np = (double *) (p + cmd->offset);

    v1 = atof(v);
    if (v1 < cmd->min || v1 > cmd->max)
        return SLS_CONF_OUT_RANGE;
    *np = v1;
    return SLS_CONF_OK;
}

const char * sls_conf_set_bool(const char *v, sls_conf_cmd_t *cmd, void *conf)
{
    char      * p = (char *)conf;
    bool      * np;
    char      * value;

    np = (bool *) (p + cmd->offset);

    if (0 == strcmp(v, "true")) {
        *np = true;
        return SLS_CONF_OK;
    } else if (0 == strcmp(v, "false")) {
        *np = false;
        return SLS_CONF_OK;
    } else {
        return SLS_CONF_WRONG_TYPE;
    }
}


int sls_conf_get_conf_count(sls_conf_base_t *c)
{
    int count = 0;
    while (c) {
        c = c->sibling;
        count ++;
    }
    return count;
}

vector<string> sls_conf_string_split(const string& str, const string& delim)
{
    vector<string> res;
    if("" == str) return res;

    char * strs = new char[str.length() + 1] ;
    strcpy(strs, str.c_str());

    char * d = new char[delim.length() + 1];
    strcpy(d, delim.c_str());

    char *p = strtok(strs, d);
    while(p) {
        string s = p;
        res.push_back(s);
        p = strtok(NULL, d);
    }

    delete[] strs;
    delete[] d;
    return res;
}

string& trim(string &s)
{
    if (s.empty())
    {
        return s;
    }
    s.erase(0,s.find_first_not_of(" "));
    s.erase(s.find_last_not_of(" ") + 1);
    return s;
}

string&  replace_all(string& str, const string& old_value, const string& new_value)
{
    while(true) {
        string::size_type pos(0);
        if((pos=str.find(old_value))!=string::npos )
            str.replace(pos,old_value.length(),new_value);
        else
          break;
    }
    return  str;
}

sls_conf_base_t * sls_conf_create_block_by_name(string n, sls_runtime_conf_t *& p_runtime)
{
    sls_conf_base_t * p = NULL;
    p_runtime = sls_runtime_conf_t::first;
    while (p_runtime) {
        if (strcmp(n.c_str(), p_runtime->conf_name) == 0) {
            p = p_runtime->create_fn();
            //sls_add_conf_to_runtime(p, p_runtime);
            break;
        }
        p_runtime = p_runtime->next;
    }
    return p;

}
//#define new_conf(n, i)\
//        sls_conf_##n conf_##n##_##i##_info = new sls_conf_##n ;
int sls_conf_parse_block(ifstream& ifs, int& line, sls_conf_base_t * b, bool& child, sls_runtime_conf_t * p_runtime, int brackets_layers)
{
    int ret = SLS_ERROR;
    sls_conf_base_t * block = NULL;
    string str_line, str_line_last;
    string n, v, line_end_flag;
    int index ;

    while(getline(ifs, str_line))
    {
        line ++;
        sls_log(SLS_LOG_TRACE, "line:%d='%s'", line, str_line.c_str());
        //remove #
        index = str_line.find('#');
        if (index != -1) {
            str_line = str_line.substr(0, index);
        }
         //trim and replace '\t'
        str_line = replace_all(str_line, "\t", "");
        str_line = trim(str_line);
        if (str_line.length() == 0) {
            sls_log(SLS_LOG_TRACE, "line:%d='%s', is comment.", line, str_line.c_str());
            continue;
        }

        //check if the last char is ';', '{', '}'
        line_end_flag = str_line.substr(str_line.length() - 1);

        if (line_end_flag == ";") {
            if (!b) {
                sls_log(SLS_LOG_ERROR, "line:%d='%s', not found block.", line, str_line.c_str());
                ret = SLS_ERROR;
                break;
            }
            //key value
            str_line = str_line.substr(0, str_line.length() - 1);

            str_line = replace_all(str_line, "\t", "");
            str_line = trim(str_line);

            //split by space
            int index = str_line.find(' ');
            if (index == -1) {
                sls_log(SLS_LOG_ERROR, "line:%d='%s', no space separator.", line, str_line.c_str());
                ret = SLS_ERROR;
                break;
            }
            n = str_line.substr(0, index);
            v = str_line.substr(index + 1, str_line.length() - (index + 1));
            v = trim(v);

            sls_conf_cmd_t * it = sls_conf_find(n.c_str(), p_runtime->conf_cmd, p_runtime->conf_cmd_size);
            if (!it) {
                sls_log(SLS_LOG_ERROR, "line:%d='%s', wrong name='%s'.", line, str_line.c_str() ,n.c_str());
                ret = SLS_ERROR;
                break;
            }
            const char * r = it->set(v.c_str(), it, b);
            if (r != SLS_CONF_OK) {
                sls_log(SLS_LOG_ERROR, "line:%d, set failed, %s, name='%s', value='%s'.", line, r ,n.c_str(), v.c_str());
                ret = SLS_ERROR;
                break;
            }
            sls_log(SLS_LOG_TRACE, "line:%d, set name='%s', value='%s'.", line, n.c_str(), v.c_str());

        } else  if (line_end_flag == "{") {
            str_line = str_line.substr(0, str_line.length() - 1);
            str_line = replace_all(str_line, "\t", "");
            str_line = trim(str_line);

            n = str_line;
            if (n.length() == 0) {
                if (str_line_last.length() == 0) {
                    sls_log(SLS_LOG_ERROR, "line:%d, no name found.", line);
                    ret = SLS_ERROR;
                    break;
                }
                n = str_line_last;
                str_line_last = "";
            }
            // new block
            block = sls_conf_create_block_by_name(n, p_runtime);
            if (!block) {
                sls_log(SLS_LOG_ERROR, "line:%d, name='%s' not found.", line, n.c_str());
                ret = SLS_ERROR;
                break;
            }
            if (child)
                b->child = block;
            else
                b->sibling = block;
            b = block;
            child = true;
            brackets_layers ++;
            ret = sls_conf_parse_block(ifs, line, b, child, p_runtime, brackets_layers);
            if (ret != SLS_OK) {
                sls_log(SLS_LOG_ERROR, "line:%d, parse block=‘%s’ failed.", line, block->name);
                ret = SLS_ERROR;
                break;
            }
        } else if (line_end_flag == "}" ) {
            if (str_line != line_end_flag) {
                sls_log(SLS_LOG_ERROR, "line:%d=‘%s’, end indicator ‘}’ with more info.", str_line.c_str(), line);
                ret = SLS_ERROR;
                break;
            }
            brackets_layers --;
            ret = SLS_OK;
            child = false;
            break;

        } else {
            sls_log(SLS_LOG_ERROR, "line:%d='%s', invalid end flag, except ';', '{', '}',", str_line.c_str(), line);
            ret = SLS_ERROR;
            break;
        }
        str_line_last = str_line;
    }
    return ret;

}


int sls_conf_open(const char * conf_file)
{
    ifstream    ifs(conf_file);
    int         ret = 0;
    int         line = 0;
    bool        child = true;
    int         brackets_layers = 0;

    sls_runtime_conf_t * p_runtime = NULL;

    sls_log(SLS_LOG_INFO, "sls_conf_open, parsing conf file='%s'.", conf_file);
    if (!ifs.is_open()) {
        sls_log(SLS_LOG_FATAL, "open conf file='%s' failed, please check if the file exist.", conf_file);
        return SLS_ERROR;
    }

    ret = sls_conf_parse_block(ifs, line, &sls_first_conf, child, p_runtime, brackets_layers);
    if (ret != SLS_OK) {
        if (0 == brackets_layers) {
            sls_log(SLS_LOG_FATAL, "parse conf file='%s' failed.", conf_file);
        } else {
            sls_log(SLS_LOG_FATAL, "parse conf file='%s' failed, please check count of '{' and '}'.", conf_file);
        }
    }
    return ret;
}

void sls_conf_release(sls_conf_base_t * c)
{
    sls_conf_base_t * c_b;
    if (c->child != NULL) {
        c_b = c->child;
        sls_conf_release(c_b);
        c->child = NULL;
    }
    if (c->sibling != NULL) {
        c_b = c->sibling;
        sls_conf_release(c_b);
        c->sibling = NULL;
    }
    if (c->child == NULL && c->sibling == NULL) {
        sls_log(SLS_LOG_DEBUG, "sls_conf_release, delete '%s'.", c->name);
        delete c;
        return ;
    }
}

void sls_conf_close()
{
    sls_conf_base_t * c = sls_first_conf.child;
    sls_conf_release(c);
}

sls_conf_base_t * sls_conf_get_root_conf()
{
    return sls_first_conf.child;
}


int sls_parse_argv(int argc, char * argv[], sls_opt_t * sls_opt, sls_conf_cmd_t *conf_cmd_opt, int cmd_size)
{
    char opt_name[256] = {0};
    char opt_value[256] = {0};
    char temp[256] = {0};

    int ret  = SLS_OK;
    int i    = 1;//skip
    int len  = cmd_size;

    //special for '-h'
    if (argc == 2) {
        strcpy(temp, argv[1]);
        sls_remove_marks(temp);
        if (strcmp(temp, "-h") == 0) {
            printf("option help info:\n");
            for (i=0; i<len; i++) {
                printf("-%s, %s, range: %.0f-%.0f.\n",
                    conf_cmd_opt[i].name,
                    conf_cmd_opt[i].mark,
                    conf_cmd_opt[i].min,
                    conf_cmd_opt[i].max);
            }
        } else {
            printf("wrong parameter, '%s'.\n", argv[1]);
        }
        return SLS_ERROR;
    }
    while (i < argc) {
        strcpy(temp, argv[i]);
        len = strlen(temp);
        if (len ==0) {
        	printf("wrong parameter, is ''.");
            ret = SLS_ERROR;
            return ret;
        }
        sls_remove_marks(temp);
        if (temp[0] != '-') {
        	printf("wrong parameter '%s', the first character must be '-'.", opt_name);
            ret = SLS_ERROR;
            return ret;
        }
        strcpy(opt_name, temp + 1);

        sls_conf_cmd_t * it = sls_conf_find(opt_name, conf_cmd_opt, cmd_size);
        if (!it) {
        	printf("wrong parameter '%s'.", argv[i]);
            ret = SLS_ERROR;
            return ret;
        }
        i ++;
        strcpy(opt_value, argv[i++]);
        sls_remove_marks(opt_value);
        const char * r = it->set(opt_value, it, sls_opt);
        if (r != SLS_CONF_OK) {
        	printf("parameter set failed, %s, name='%s', value='%s'.", r, opt_name, opt_value);
            ret = SLS_ERROR;
            return ret;
        }
    }
    return ret;
}

