/***************************************************************************
 * 
 * Copyright (c) 2015 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file lsh.h
 * @author gusimiu(com@baidu.com)
 * @date 2015/11/19 19:17:23
 * @brief 
 *  
 **/

#ifndef  _LSH_H_
#define  _LSH_H_

#include <vector>
using namespace std;

#include "helper.h"

struct CompareBlock_t {
    float score;
    size_t index;

    bool operator< (const CompareBlock_t& o) const {
        return score > o.score;
    }
};

class LSHIndex_t {
    public:
        LSHIndex_t();
        ~LSHIndex_t();

        void build(size_t random_num, size_t query_num, size_t dim, float* vector_buffer);
        void find_knearest(const float* vec, size_t dim, size_t output_top_N, vector<CompareBlock_t>& results);

    private:
        vector<float*> _random_project_vectors;

        size_t   _query_num;
        size_t   _dim;
        const float* _vector_buffer_refer;

        vector< vector<CompareBlock_t> > _random_order;

        void _clear_random_vectors();
};


#endif  // _LSH_H_

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
