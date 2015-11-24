/***************************************************************************
 * 
 * Copyright (c) 2015 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file py_term2query.cc
 * @author nickgu
 * @date 2015/08/28 16:54:24
 * @brief 
 *  
 **/
#include <Python.h> //包含python的头文件

#include <vector>
#include <string>
using namespace std;

#include "helper.h"
#include "lsh.h"

struct VectorDict_t {
    LSHIndex_t      _lsh_index;
    vector<string>  _names;
    float*          _big_buffer;
    int             _dim;

    void read(const char* filename, int dim) {
        FILE* fp = fopen(filename, "r");
        vector<string> fields;
        vector<float*> vectors;
        LOG_NOTICE("Try to create lsh(dim=%d) on file (%s)", dim, filename);

        _dim = dim;
        char line[10240];
        while (fgets(line, sizeof(line), fp) != NULL) {
            line[strlen(line)-1] = 0;
            split(line, "\t", fields);

            if (fields.size()==2) {
                vector<string> inner_fields;
                split( (char*)fields[1].c_str(), ",", inner_fields );
                float* buf = new float[_dim];
                memset(buf, 0, sizeof(float)*_dim);

                for (size_t i=0; i<inner_fields.size(); ++i) {
                    vector<string> kv;
                    split( (char*)inner_fields[i].c_str(), ":", kv );
                    if (kv.size()==2) {
                        int idx = atoi(kv[0].c_str());
                        float value = atof(kv[1].c_str());

                        buf[idx]  = value;
                    }
                }

                _names.push_back(fields[0]);
                vectors.push_back(buf);
            }
        }

        _big_buffer = new float[vectors.size() * _dim * sizeof(float)];
        for (size_t i=0; i<vectors.size(); ++i) {
            memcpy(_big_buffer + (i * _dim), vectors[i], sizeof(float)*_dim);
            delete [] vectors[i];
        }

        _lsh_index.build(50, _names.size(), _dim, _big_buffer);
        fclose(fp);
    }
};

static vector<VectorDict_t*> g_dicts;

static PyObject*
wrapper_read(PyObject *self, PyObject *args) 
{
    const char* filename = PyString_AsString(PyTuple_GetItem(args, 0));
    int dim = PyInt_AsLong(PyTuple_GetItem(args, 1));
    fprintf(stderr, "prepare to load vectors in : [%s]\n", filename);

    VectorDict_t* pvdict = new VectorDict_t();
    pvdict->read(filename, dim);

    int handler = g_dicts.size();
    g_dicts.push_back(pvdict);

    return Py_BuildValue("i", handler);//把c的返回值n转换成python的对象
}


static VectorDict_t* parse_dict(PyObject* args) {
    int did = PyInt_AsLong(PyTuple_GetItem(args, 0));
    if (did>=0 && did < (int)g_dicts.size()) {
        return g_dicts[did];
    }
    LOG_ERROR("Dict fetch failed! handler=%d", did);
    return NULL;
}


static PyObject*
wrapper_knearest(PyObject* self, PyObject* args) {
    VectorDict_t* dict = parse_dict(args);
    if (dict == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }

    size_t K = PyInt_AsLong(PyTuple_GetItem(args, 2));

    // get vectors.
    PyObject* value_list = PyTuple_GetItem(args, 1);
    size_t value_list_size = PyList_Size(value_list);
    float *vec = new float[ dict->_dim ];
    for (size_t i=0; i<value_list_size; ++i) {
        vec[i] = PyFloat_AsDouble(PyList_GetItem(value_list, i));
    }

    vector<CompareBlock_t> results;
    dict->_lsh_index.find_knearest(vec, dict->_dim, K, results);

    PyObject* ret = PyTuple_New(results.size());
    for (size_t i=0; i<results.size(); ++i) {
        PyTuple_SetItem(ret, i, 
                Py_BuildValue("sf", 
                    dict->_names[results[i].index].c_str(), 
                    results[i].score)
                );
    }
    delete [] vec;
    return ret;
}


// 3 方法列表
static PyMethodDef LSH_Func[] = {
    // 读取文件到词典，可选是否是内存结构
    { "read", wrapper_read, METH_VARARGS, "load models.\n\t\tpy_lsh.read(filename, dim)"},

    // k-nearest.
    { "knearest", wrapper_knearest, METH_VARARGS, "find the k-nearest vectors in the vector set."},

    { NULL, NULL, 0, NULL }
};


// 4 模块初始化方法
PyMODINIT_FUNC initpy_lsh(void) {
    //初始模块，把CKVDictFunc初始到c_kvdict中
    PyObject *m = Py_InitModule("py_lsh", LSH_Func);
    if (m == NULL) {
        fprintf(stderr, "Init module failed!\n");
        return;
    }
}


/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
