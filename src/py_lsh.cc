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
    size_t          _dim;
    map<string, size_t> _key2idx;
    FArray_t<DenseVector_t> _vector_pool;

    void read(const char* filename, int dim, int aux) {
        FILE* fp = fopen(filename, "r");
        vector<string> fields;
        vector<DenseVector_t> vectors;
        LOG_NOTICE("Try to create lsh(dim=%d) on file (%s)", dim, filename);

        _dim = dim;
        char line[10240];
        int lc = 0;
        while (fgets(line, sizeof(line), fp) != NULL) {
            line[strlen(line)-1] = 0;
            split(line, "\t", fields);

            if (fields.size()==2) {
                vector<string> inner_fields;
                split( (char*)fields[1].c_str(), ",", inner_fields );
                DenseVector_t dv;

                for (size_t i=0; i<inner_fields.size(); ++i) {
                    int idx;
                    float value;
                    sscanf(inner_fields[i].c_str(), "%d:%f", &idx, &value);
                    dv.push_back(idx, value);
                }

                _key2idx[fields[0]] = _names.size();
                _names.push_back(fields[0]);
                _vector_pool.push_back( dv );
            }

            lc += 1;
            if (lc % 100000 == 0) {
                LOG_NOTICE("LSH load %d line(s)", lc);
            }
        }
        fclose(fp);
        LOG_NOTICE("LSH Load over. begin to build index.");

        _lsh_index.build(aux, _dim, &_vector_pool);
        LOG_NOTICE("LSH build over.");
    }
};

static vector<VectorDict_t*> g_dicts;

static PyObject*
wrapper_read(PyObject *self, PyObject *args) 
{
    const char* filename = PyString_AsString(PyTuple_GetItem(args, 0));
    int dim = PyInt_AsLong(PyTuple_GetItem(args, 1));
    size_t aux_num = PyInt_AsLong(PyTuple_GetItem(args, 2));

    fprintf(stderr, "prepare to load vectors in : [%s] dim=%d, aux=%d\n", filename, dim, aux_num);

    VectorDict_t* pvdict = new VectorDict_t();
    Timer reader_timer;
    reader_timer.begin();
    pvdict->read(filename, dim, aux_num);
    reader_timer.end();
    LOG_NOTICE("cost time of load lsh dict: %.3f(s)", reader_timer.cost_time());

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
    //LOG_NOTICE("dict=%p k=%d dim=%d", dict, K, dict->_dim);

    // get vectors.
    PyObject* indval_list = PyTuple_GetItem(args, 1);
    size_t indval_list_size = PyList_Size(indval_list);
    //LOG_NOTICE("sizeof(indval_list)=%d", indval_list_size);

    float *vec = new float[ dict->_dim ];
    memset(vec, 0, sizeof(float)*dict->_dim);
    for (size_t i=0; i<indval_list_size; ++i) {
        int idx = PyInt_AsLong(PyTuple_GetItem(PyList_GetItem(indval_list, i), 0));
        float value = PyFloat_AsDouble(PyTuple_GetItem(PyList_GetItem(indval_list, i), 1));
        if (idx<0 || idx>=(int)dict->_dim) {
            LOG_ERROR("VectorDim[%d:%f] out of dim range.", idx, value);
            continue;
        }
        vec[idx] = value;
    }

    vector<CompareBlock_t> results;
    dict->_lsh_index.find_knearest(vec, dict->_dim, K, results);

    PyObject* ret = PyTuple_New(results.size());
    for (size_t i=0; i<results.size(); ++i) {
        /*
        LOG_NOTICE("%s : %f",                     
                    dict->_names[results[i].index].c_str(), 
                    results[i].score);
        */
        PyTuple_SetItem(ret, i, 
                Py_BuildValue("sf", 
                    dict->_names[results[i].index].c_str(), 
                    results[i].score)
                );
    }
    delete [] vec;
    return ret;
}

static PyObject*
wrapper_search(PyObject* self, PyObject* args) {
    VectorDict_t* dict = parse_dict(args);
    if (dict == NULL) {
        // dict is invalid.
        Py_INCREF(Py_None);
        return Py_None;
    }

    const char* key = PyString_AsString(PyTuple_GetItem(args, 1));
    if (dict->_key2idx.find(key) == dict->_key2idx.end()) {
        // key not found.
        Py_INCREF(Py_None);
        return Py_None;
    }

    const DenseVector_t& dv = dict->_vector_pool[dict->_key2idx[key]];
    PyObject* ret_list = PyList_New(dv.v.size());
    for (size_t i=0; i<dv.v.size(); ++i) {
        PyList_SetItem(ret_list, i, 
                Py_BuildValue("if",
                    dv.v[i].index,
                    dv.v[i].value)
                );
    }
    return ret_list;
}

// 3 方法列表
static PyMethodDef LSH_Func[] = {
    // 读取文件到词典，可选是否是内存结构
    { "read", wrapper_read, METH_VARARGS, "load models.\n\t\tpy_lsh.read(filename, dim, aux_num)"},

    // k-nearest.
    { "knearest", wrapper_knearest, METH_VARARGS, 
        "knearest(dict_handle, [[idx, value], [idx, value], ..], \
return_list_num): find the k-nearest vectors in the vector set."},

    // k-nearest.
    { "search", wrapper_search, METH_VARARGS, "search(dict_handle, key): get the vector with input key."},

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
