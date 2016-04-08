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
using namespace std;

#include "helper.h"
#include "term2query_dict.h"

static vector<Term2QueryDict_t*> g_dicts;

static Term2QueryDict_t* parse_dict(PyObject* args) {
    int did = PyInt_AsLong(PyTuple_GetItem(args, 0));
    if (did>=0 && did < (int)g_dicts.size()) {
        return g_dicts[did];
    }
    LOG_ERROR("Dict fetch failed! handler=%d", did);
    return NULL;
}

static PyObject*
wrapper_read(PyObject *self, PyObject *args) 
{
    const char* filename = PyString_AsString(PyTuple_GetItem(args, 0));
    fprintf(stderr, "prepare to load model: [%s]\n", filename);

    Term2QueryDict_t* pmodel = new Term2QueryDict_t();
    pmodel->read(filename, 32);
    //pmodel->read(filename);
    int handler = g_dicts.size();
    g_dicts.push_back(pmodel);

    return Py_BuildValue("i", handler);//把c的返回值n转换成python的对象
}

static PyObject*
wrapper_read_terms(PyObject *self, PyObject *args) 
{
    const char* filename = PyString_AsString(PyTuple_GetItem(args, 0));
    fprintf(stderr, "prepare to load model: [%s]\n", filename);

    Term2QueryDict_t* pmodel = new Term2QueryDict_t();
    pmodel->read(filename, 0, false);
    //pmodel->read(filename);
    int handler = g_dicts.size();
    g_dicts.push_back(pmodel);

    return Py_BuildValue("i", handler);//把c的返回值n转换成python的对象
}

static PyObject*
wrapper_vector_query(PyObject* self, PyObject* args) {
    Term2QueryDict_t* dict = parse_dict(args);
    if (dict == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }

    const char* query = PyString_AsString(PyTuple_GetItem(args, 1));
    float *vec = new float[dict->dim()];
    
    bool found = dict->vector_query(query, vec);
    if (!found) {
        delete [] vec;
        Py_INCREF(Py_None);
        return Py_None;
    }

    PyObject* ret = PyTuple_New(dict->dim());
    for (size_t i=0; i<dict->dim(); ++i) {
        PyTuple_SetItem(ret, i, Py_BuildValue("f", vec[i]));
    }
    delete [] vec;
    return ret;
}

static PyObject*
wrapper_vector_terms(PyObject* self, PyObject* args) {
    Term2QueryDict_t* dict = parse_dict(args);
    if (dict == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }

    PyObject* term_list = PyTuple_GetItem(args, 1);
    size_t term_num = PyList_Size(term_list);
    vector<string> terms;
    for (size_t i=0; i<term_num; ++i) {
        const char* term = PyString_AsString(PyList_GetItem(term_list, i));
        terms.push_back(term);
    }

    float *vec = new float[dict->dim()];
    bool found = dict->vector_terms(terms, vec);
    if (!found) {
        delete [] vec;
        Py_INCREF(Py_None);
        return Py_None;
    }

    PyObject* ret = PyTuple_New(dict->dim());
    for (size_t i=0; i<dict->dim(); ++i) {
        PyTuple_SetItem(ret, i, Py_BuildValue("f", vec[i]));
    }
    delete [] vec;
    return ret;
}

static PyObject*
wrapper_dist_query_query(PyObject* self, PyObject* args) {
    Term2QueryDict_t* dict = parse_dict(args);
    if (dict == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }

    const char* query_a = PyString_AsString(PyTuple_GetItem(args, 1));
    const char* query_b = PyString_AsString(PyTuple_GetItem(args, 2));

    float *vec_a = new float[dict->dim()];
    float *vec_b = new float[dict->dim()];
    
    bool found = dict->vector_query(query_a, vec_a);
    if (!found) {
        delete [] vec_a;
        delete [] vec_b;
        Py_INCREF(Py_None);
        return Py_None;
    }
    found = dict->vector_query(query_b, vec_b);
    if (!found) {
        delete [] vec_a;
        delete [] vec_b;
        Py_INCREF(Py_None);
        return Py_None;
    }

    float dist = Term2QueryDict_t::dot_dist(vec_a, vec_b, dict->dim());

    delete [] vec_a;
    delete [] vec_b;
    return Py_BuildValue("f", dist);
}


static PyObject*
wrapper_dist_terms_query(PyObject* self, PyObject* args) {
    Term2QueryDict_t* dict = parse_dict(args);
    if (dict == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }

    // get terms.
    PyObject* term_list = PyTuple_GetItem(args, 1);
    size_t term_num = PyList_Size(term_list);
    vector<string> terms;
    for (size_t i=0; i<term_num; ++i) {
        const char* term = PyString_AsString(PyList_GetItem(term_list, i));
        terms.push_back(term);
    }

    // get query.
    const char* query_b = PyString_AsString(PyTuple_GetItem(args, 2));


    float *vec_a = new float[dict->dim()];
    float *vec_b = new float[dict->dim()];
    
    bool found = dict->vector_terms(terms, vec_a);
    if (!found) {
        delete [] vec_a;
        delete [] vec_b;
        Py_INCREF(Py_None);
        return Py_None;
    }
    found = dict->vector_query(query_b, vec_b);
    if (!found) {
        delete [] vec_a;
        delete [] vec_b;
        Py_INCREF(Py_None);
        return Py_None;
    }

    float dist = Term2QueryDict_t::dot_dist(vec_a, vec_b, dict->dim());

    delete [] vec_a;
    delete [] vec_b;
    return Py_BuildValue("f", dist);
}


static PyObject*
wrapper_dist_terms_terms(PyObject* self, PyObject* args) {
    Term2QueryDict_t* dict = parse_dict(args);
    if (dict == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }

    // get terms.
    PyObject* term_list = PyTuple_GetItem(args, 1);
    size_t term_num = PyList_Size(term_list);
    vector<string> terms_a;
    for (size_t i=0; i<term_num; ++i) {
        const char* term = PyString_AsString(PyList_GetItem(term_list, i));
        terms_a.push_back(term);
    }

    term_list = PyTuple_GetItem(args, 2);
    term_num = PyList_Size(term_list);
    vector<string> terms_b;
    for (size_t i=0; i<term_num; ++i) {
        const char* term = PyString_AsString(PyList_GetItem(term_list, i));
        terms_b.push_back(term);
    }

    float *vec_a = new float[dict->dim()];
    float *vec_b = new float[dict->dim()];
    
    bool found = dict->vector_terms(terms_a, vec_a);
    if (!found) {
        delete [] vec_a;
        delete [] vec_b;
        Py_INCREF(Py_None);
        return Py_None;
    }
    found = dict->vector_terms(terms_b, vec_b);
    if (!found) {
        delete [] vec_a;
        delete [] vec_b;
        Py_INCREF(Py_None);
        return Py_None;
    }

    float dist = Term2QueryDict_t::dot_dist(vec_a, vec_b, dict->dim());

    delete [] vec_a;
    delete [] vec_b;
    return Py_BuildValue("f", dist);
}

static PyObject*
wrapper_knearest_query(PyObject* self, PyObject* args) {
    Term2QueryDict_t* dict = parse_dict(args);
    if (dict == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }

    const char* query = PyString_AsString(PyTuple_GetItem(args, 1));
    size_t K = PyInt_AsLong(PyTuple_GetItem(args, 2));

    float *vec= new float[dict->dim()];

    vector<Term2QueryDict_t::Result_t> results;
    if ( !dict->n_nearest(query, results, K) ) {
        delete [] vec;
        Py_INCREF(Py_None);
        return Py_None;
    }
   
    PyObject* ret = PyTuple_New(results.size());
    for (size_t i=0; i<results.size(); ++i) {
        PyTuple_SetItem(ret, i, Py_BuildValue("sf", results[i].query.c_str(), results[i].score));
    }
    delete [] vec;
    return ret;
}


static PyObject*
wrapper_knearest_terms(PyObject* self, PyObject* args) {
    Term2QueryDict_t* dict = parse_dict(args);
    if (dict == NULL) {
        Py_INCREF(Py_None);
        return Py_None;
    }

    // get terms.
    PyObject* term_list = PyTuple_GetItem(args, 1);
    size_t term_num = PyList_Size(term_list);
    vector<string> terms;
    for (size_t i=0; i<term_num; ++i) {
        const char* term = PyString_AsString(PyList_GetItem(term_list, i));
        terms.push_back(term);
    }

    size_t K = PyInt_AsLong(PyTuple_GetItem(args, 2));

    float *vec= new float[dict->dim()];

    vector<Term2QueryDict_t::Result_t> results;
    if ( !dict->n_nearest(terms, results, K) ) {
        delete [] vec;
        Py_INCREF(Py_None);
        return Py_None;
    }
   
    PyObject* ret = PyTuple_New(results.size());
    for (size_t i=0; i<results.size(); ++i) {
        PyTuple_SetItem(ret, i, Py_BuildValue("sf", results[i].query.c_str(), results[i].score));
    }
    delete [] vec;
    return ret;
}


// 3 方法列表
static PyMethodDef Term2QueryDict_Func[] = {
    // 读取文件到词典，可选是否是内存结构
    { "read", wrapper_read, METH_VARARGS, "load models.\n\t\tpy_term2query_dict.read(file)"},
    { "read_terms", wrapper_read_terms, METH_VARARGS, "load models (only terms).\n\t\tpy_term2query_dict.read_terms(file)"},

    // get vectors.
    { "vector_query", wrapper_vector_query, METH_VARARGS, "get vector of query."},
    { "vector_terms", wrapper_vector_terms, METH_VARARGS, "get vector of terms."},

    // dist.
    { "dist_query_query", wrapper_dist_query_query, METH_VARARGS, "dist of query and query."},
    { "dist_terms_query", wrapper_dist_terms_query, METH_VARARGS, "dist of terms and query."},
    { "dist_terms_terms", wrapper_dist_terms_terms, METH_VARARGS, "dist of terms and terms."},

    // k-nearest.
    {"knearest_terms", wrapper_knearest_terms, METH_VARARGS, "find the k-nearest querys of input term list."},
    {"knearest_query", wrapper_knearest_query, METH_VARARGS, "find the k-nearest querys of input query."},

    { NULL, NULL, 0, NULL }
};


// 4 模块初始化方法
PyMODINIT_FUNC initpy_term2query_dict(void) {
    //初始模块，把CKVDictFunc初始到c_kvdict中
    PyObject *m = Py_InitModule("py_term2query_dict", Term2QueryDict_Func);
    if (m == NULL) {
        fprintf(stderr, "Init module failed!\n");
        return;
    }
}


/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
