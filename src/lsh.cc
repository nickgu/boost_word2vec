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
    _query_num(0),
    _dim(0),
    _vector_buffer_refer(NULL)
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
LSHIndex_t::build(size_t random_num, size_t query_num, size_t dim, float* vector_buffer) {
    _query_num = query_num;
    _dim = dim;
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
        for (size_t qid=0; qid<_query_num; ++qid) {
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

void 
LSHIndex_t::find_knearest(const float* vec, size_t dim, size_t output_top_N, vector<CompareBlock_t>& results) {
    set<size_t> candidate_set; 
    size_t RecallTimes = 32;
    size_t Range = min(200UL, output_top_N * RecallTimes);
    for (size_t i=0; i<_random_project_vectors.size(); ++i) {
        CompareBlock_t probe;
        probe.score = dot(vec, _random_project_vectors[i], dim);
        probe.index = 0;
        
        int pos = lower_bound(_random_order[i].begin(), _random_order[i].end(), probe) - _random_order[i].begin();
        //fprintf(stderr, "random index %d: pos=%d\n", i, pos);
        for (size_t idx=max(0UL, pos - Range); 
                    idx<min(_query_num, pos + Range); 
                    ++idx) 
        {
            candidate_set.insert(_random_order[i][idx].index);
        }
    }
    // candidate select over.
    //LOG_NOTICE("%d candidate(s) found.", candidate_set.size());

    // finding best match.
    results.clear();

    for (set<size_t>::iterator it = candidate_set.begin(); it!=candidate_set.end(); ++it) {
        CompareBlock_t cb;
        cb.index = *it;
        cb.score = dot(vec, _vector_buffer_refer + cb.index*_dim, _dim);
        results.push_back(cb);
    }

    if (output_top_N >= results.size()) {
        sort(results.begin(), results.end());
    } else {
        partial_sort(results.begin(), results.begin() + output_top_N, results.end());
    }
    return ;
}

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
