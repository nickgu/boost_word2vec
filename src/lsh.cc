/***************************************************************************
 * 
 * Copyright (c) 2015 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file lsh.cc
 * @author gusimiu(com@baidu.com)
 * @date 2015/11/19 19:20:39
 * @brief 
 *  
 **/

#include "lsh.h"

#include <set>
using namespace std;

static float 
dot(const float* v1, const float* v2, size_t dim) {
    float res = 0;
    for (size_t i=0; i<dim; ++i) {
        res += v1[i] * v2[i];
    }
    return res;
}

LSHIndex_t::LSHIndex_t() :
    _vector_num(0),
    _dim(0),
    _vector_buffer_refer(NULL),
    _dense_buffer_refer(NULL)
{
    _random_project_vectors.clear();
    _random_order.clear();
}

void LSHIndex_t::_clear_random_vectors() {
    for (size_t i=0; i<_random_project_vectors.size(); ++i) {
        free(_random_project_vectors[i]);
    }
    _random_project_vectors.clear();
}

LSHIndex_t::~LSHIndex_t() {
    _clear_random_vectors();
}

void 
LSHIndex_t::build(size_t random_num, size_t vector_num, size_t dim, float* vector_buffer) {
    _vector_num = vector_num;
    _dim = dim;
    _dense_buffer_refer = NULL;
    _vector_buffer_refer = vector_buffer;

    _clear_random_vectors();
    _random_order.clear();

    int EPS = 100000;
    int HALF_EPS = EPS / 2;
    for (size_t i=0; i<random_num; ++i) {
        vector<CompareBlock_t> order;
        LOG_NOTICE("Building No.%d random vectors.", i+1);
        float* v = (float*)malloc(sizeof(float) * dim);
        for (size_t d=0; d<dim; ++d) {
            v[d] = (rand() % EPS - HALF_EPS) * 1. / (EPS);
        }

        _random_project_vectors.push_back(v);
        for (size_t qid=0; qid<_vector_num; ++qid) {
            CompareBlock_t cb;
            cb.score = dot(v, vector_buffer + qid*dim, dim);
            cb.index = qid;

            order.push_back(cb);
        }
        sort(order.begin(), order.end());
        _random_order.push_back(order);
    }
    return ;
}

/**
 * Dense vector version.
 */
void 
LSHIndex_t::build(size_t random_num, size_t dim, FArray_t<DenseVector_t>* dense_vector_refer) {
    _vector_num = dense_vector_refer->size();
    _dim = dim;
    _dense_buffer_refer = dense_vector_refer;
    _vector_buffer_refer = NULL;

    _clear_random_vectors();
    _random_order.clear();

    int EPS = 100000;
    int HALF_EPS = EPS / 2;
    for (size_t i=0; i<random_num; ++i) {
        vector<CompareBlock_t> order;
        LOG_NOTICE("Building No.%d random vectors.", i+1);
        float* v = (float*)malloc(sizeof(float) * dim);
        for (size_t d=0; d<dim; ++d) {
            v[d] = (rand() % EPS - HALF_EPS) * 1. / (EPS);
        }

        _random_project_vectors.push_back(v);
        for (size_t qid=0; qid<_vector_num; ++qid) {
            CompareBlock_t cb;
            cb.score = (*dense_vector_refer)[qid].dot(v, dim);
            cb.index = qid;
            //LOG_NOTICE("%d : qid=%d score=%.3f", i, cb.index, cb.score);

            order.push_back(cb);
        }
        sort(order.begin(), order.end());
        _random_order.push_back(order);
    }
    return ;
}


void 
LSHIndex_t::find_knearest(const float* vec, size_t dim, size_t output_top_N, vector<CompareBlock_t>& results) {
    set<size_t> candidate_set; 

    /* Recall range = [ -max(N*RecallTimes, 200),  max(N*RecallTimes, 200) )*/
    size_t RecallTimes = 32;
    int Range = min(200UL, output_top_N * RecallTimes);
    for (size_t i=0; i<_random_project_vectors.size(); ++i) {
        CompareBlock_t probe;
        probe.score = dot(vec, _random_project_vectors[i], dim);
        probe.index = 0;
        
        int pos = lower_bound(_random_order[i].begin(), _random_order[i].end(), probe) - _random_order[i].begin();
        /*
        fprintf(stderr, "random index %d: pos=%d, score=%.4f range=[%u, %u)\n", 
                i, pos, probe.score, max(0, pos-Range), min((int)_vector_num, pos+Range));
        */

        for (int idx=max(0, pos - Range); 
                    idx<min((int)_vector_num, pos + Range); 
                    ++idx) 
        {
            candidate_set.insert(_random_order[i][idx].index);
        }
    }
    // candidate select over.
    //LOG_NOTICE("%d candidate(s) found.", candidate_set.size());

    // finding best match.
    vector<CompareBlock_t> temp_results;

    for (set<size_t>::iterator it = candidate_set.begin(); it!=candidate_set.end(); ++it) {
        CompareBlock_t cb;
        cb.index = *it;
        if (_vector_buffer_refer!=NULL) {
            cb.score = dot(vec, _vector_buffer_refer + cb.index*_dim, _dim);
        } else {
            DenseVector_t& dv = (*_dense_buffer_refer)[cb.index];
            cb.score = dv.dot(vec, _dim);
            // NOTICE this.
            cb.score /= dv.norm2();
        }
        temp_results.push_back(cb);
    }

    if (output_top_N >= temp_results.size()) {
        sort(temp_results.begin(), temp_results.end());
    } else {
        partial_sort(temp_results.begin(), temp_results.begin() + output_top_N, temp_results.end());
    }

    results.clear();
    for (size_t i=0; i<output_top_N && i<temp_results.size(); ++i) {
        results.push_back(temp_results[i]);
    }
    return ;
}

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
