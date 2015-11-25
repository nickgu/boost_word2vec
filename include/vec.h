/***************************************************************************
 * 
 * Copyright (c) 2015 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file vec.h
 * @author gusimiu(com@baidu.com)
 * @date 2015/11/24 19:27:16
 * @brief 
 *  
 **/

#ifndef  __VEC_H_
#define  __VEC_H_

#include <map>
using namespace std;

#include "helper.h"

struct IndValue_t {
    int index;
    float value;

    IndValue_t() {}
    IndValue_t(int ind, float val) :
        index(ind),
        value(val)
    {}
};


struct DenseVector_t {
    FArray_t<IndValue_t> v;

    void push_back(size_t ind, float value) {
        IndValue_t iv(ind, value);
        v.push_back(iv);
    }

    float dot(const DenseVector_t& o) const {
        map<int, float> d;
        for (size_t i=0; i<v.size(); ++i) {
            d[ v[i].index ] = v[i].value;
        }

        float ans = 0;
        for (size_t i=0; i<o.v.size(); ++i) {
            int oi = o.v[i].index;
            float ov = o.v[i].value;
            ans += d[oi] * ov;
        }
        return ans;
    }

    float dot(const float* o, size_t dim) const {
        float ans = 0;
        for (size_t i=0; i<v.size(); ++i) {
            ans += o[v[i].index] * v[i].value;
        }
        return ans;
    }
};


#endif  //__VEC_H_

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
