/***************************************************************************
 * 
 * Copyright (c) 2015 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file term2query_dict.cc
 * @author nickgu
 * @date 2015/08/27 14:36:06
 * @brief 
 *  
 **/

#include <cmath>
#include <set>
#include <algorithm>
using namespace std;

#include "term2query_dict.h"
#include "helper.h"
#include "lsh.h"

static float 
dot(const float* v1, const float* v2, size_t dim) {
    float res = 0;
    for (size_t i=0; i<dim; ++i) {
        res += v1[i] * v2[i];
    }
    return res;
}


void
_ensure_buffer_size(void** pptr, size_t* psize, size_t ensure_size) {
    static size_t IncBlockSize = 128 * 1024 * 1024;
    if (*psize < ensure_size) {
        size_t delt = ensure_size - *psize;
        if (delt < IncBlockSize) {
            delt = IncBlockSize;
        }

        fprintf(stderr, "cur=%lu en=%lu delt=%lu\n", *psize, ensure_size, delt);

        if (*psize == 0) {
            *psize = delt;
            *pptr = malloc(*psize);
        } else {
            *psize += delt;
            *pptr = realloc(*pptr, *psize);
        }
    }
}

void 
Term2QueryDict_t::debug_vector(const float* vec, size_t dim) {
    for (size_t i=0; i<dim; ++i) {
        if (fabs(vec[i])>=0.1) {
            fprintf(stderr, "%d:%.2f,", i, vec[i]);
        }
    }
    fprintf(stderr, "\n");
}

size_t 
Term2QueryDict_t::_offset(size_t index) const {
    return index * _dim;
}

Term2QueryDict_t::Term2QueryDict_t () :
    _query_embeddings(NULL),
    _query_embeddings_buffer_size(0),
    _term_embeddings(NULL),
    _term_embeddings_buffer_size(0)
{}

Term2QueryDict_t::~Term2QueryDict_t () {
    if (_term_embeddings) {
        free(_term_embeddings);
        _term_embeddings_buffer_size = 0;
    }
    if (_query_embeddings) {
        free(_query_embeddings);
        _query_embeddings_buffer_size = 0;
    }
}

// IO.
void 
Term2QueryDict_t::read(
        const char* filename, 
        size_t build_lsh_num, 
        bool read_query_vector
    ) 
{
    FILE* fd = fopen(filename, "rb");
    if (fd == NULL) {
        throw std::runtime_error(string("open file ") + filename + string(" failed!"));
    }

    fread(&_dim, sizeof(_dim), 1, fd); 
    LOG_NOTICE("loadding dict: [%s] dim=%d", filename, _dim);

    // read term information.
    size_t term_num, term_buffer_size;
    fread(&term_num, sizeof(term_num), 1, fd);
    fread(&term_buffer_size, sizeof(term_buffer_size), 1, fd);

    // load term buffer.
    char* buffer = new char[term_buffer_size];
    fread(buffer, term_buffer_size, 1, fd);
    const char* p = buffer;
    const char* end = buffer + term_buffer_size;
    int id = 0;
    _term2idx[p] = id++;
    _terms.push_back(p);
    while (p < end) {
        if (*p == '\0') {
            if (p+1<=end) {
                _term2idx[p + 1] = id++;
                _terms.push_back(p + 1);
            }
        }
        p ++;
    }
    LOG_NOTICE("Load terms num=%lu(%lu) buffer_size=%lu", term_num, this->term_num(), term_buffer_size);
    delete [] buffer;


    // read query information.
    size_t query_num, query_buffer_size;
    fread(&query_num, sizeof(query_num), 1, fd);
    fread(&query_buffer_size, sizeof(query_buffer_size), 1, fd);

    // load query buffer.
    buffer = new char[query_buffer_size];
    fread(buffer, query_buffer_size, 1, fd);
    p = buffer;
    end = buffer + query_buffer_size;
    if (read_query_vector) {
        id = 0;
        _query2idx[p] = id++;
        _querys.push_back(p);
        while (p < end) {
            if (*p == '\0') {
                if (p+1<=end) {
                    _query2idx[p + 1] = id++;
                    _querys.push_back(p + 1);
                }
            }
            p ++;
        }
        LOG_NOTICE("Load querys num=%lu(%lu) buffer_size=%lu", query_num, this->query_num(), query_buffer_size);
    }
    delete [] buffer;

    size_t buffer_size;
    // load terms' embeddings.
    buffer_size = sizeof(float) * _dim * term_num;
    _term_embeddings = (float*)malloc(buffer_size);
    _term_embeddings_buffer_size = buffer_size;
    fread(_term_embeddings, buffer_size, 1, fd);
    LOG_NOTICE("Load term's embeddings : size=%lu", buffer_size);

    // load query's embeddings.
    buffer_size = sizeof(float) * _dim * query_num;
    if (read_query_vector) {
        _query_embeddings = (float*)malloc(buffer_size);
        _query_embeddings_buffer_size = buffer_size;
        fread(_query_embeddings, buffer_size, 1, fd);
        LOG_NOTICE("Load query's embeddings : size=%lu", buffer_size);
    }
    fclose(fd);

    // whether to build lsh.
    if (build_lsh_num == 0) {
        _lsh = NULL;
    } else {
        _lsh = new LSHIndex_t();
        _lsh->build(build_lsh_num, query_num, _dim, _query_embeddings);
    }
}

void 
Term2QueryDict_t::write(const char* filename) {
    FILE* fd = fopen(filename, "wb");
    if (fd == NULL) {
        throw std::runtime_error(string("open file ") + filename + string(" failed!"));
    }

    fwrite(&_dim, sizeof(_dim), 1, fd); 
    LOG_NOTICE("Write dim=%d", _dim);

    // write term information.
    size_t term_num = this->term_num();
    size_t term_buffer_size = 0;
    for (size_t i=0; i<_terms.size(); ++i) {
        term_buffer_size += _terms[i].length() + 1;
    }

    fwrite(&term_num, sizeof(term_num), 1, fd);
    fwrite(&term_buffer_size, sizeof(term_buffer_size), 1, fd);

    for (size_t i=0; i<_terms.size(); ++i) {
        fwrite(_terms[i].c_str(), _terms[i].length()+1, 1, fd);
    }
    LOG_NOTICE("Write terms num=%lu buffer_size=%lu", term_num, term_buffer_size);

    // write query information.
    size_t query_num = this->query_num();
    size_t query_buffer_size = 0;
    for (size_t i=0; i<_querys.size(); ++i) {
        query_buffer_size += _querys[i].length() + 1;
    }

    fwrite(&query_num, sizeof(query_num), 1, fd);
    fwrite(&query_buffer_size, sizeof(query_buffer_size), 1, fd);

    for (size_t i=0; i<_querys.size(); ++i) {
        fwrite(_querys[i].c_str(), _querys[i].length()+1, 1, fd);
    }
    LOG_NOTICE("Write querys num=%lu buffer_size=%lu", query_num, query_buffer_size);

    size_t buffer_size;
    // load terms' embeddings.
    buffer_size = sizeof(float) * _dim * term_num;
    fwrite(_term_embeddings, buffer_size, 1, fd);
    LOG_NOTICE("Write term's embeddings : size=%lu", buffer_size);

    // load query's embeddings.
    buffer_size = sizeof(float) * _dim * query_num;
    fwrite(_query_embeddings, buffer_size, 1, fd);
    LOG_NOTICE("Write query's embeddings : size=%lu", buffer_size);

    fclose(fd);
}

bool 
Term2QueryDict_t::vector_query(const string& query, float* out_embeddings) const
{
    map<string, int>::const_iterator it = _query2idx.find(query);
    if (it!=_query2idx.end()) {
        int index = it->second;
        memcpy(out_embeddings, _query_embeddings + _offset(index), sizeof(float)*_dim);
        return true;
    }
    return false;
}

bool
Term2QueryDict_t::vector_terms(const vector<string>& terms, float* out_embeddings) const 
{
    memset(out_embeddings, 0, sizeof(float)*_dim);
    size_t tc_count = 0;
    for (size_t i=0; i<terms.size(); ++i) {
        map<string, int>::const_iterator it = _term2idx.find(terms[i]);
        if (it!=_term2idx.end()) {
            tc_count ++;
            int index = it->second;

            float* v = _term_embeddings+_offset(index);
            for (size_t d=0; d<_dim; ++d) {
                out_embeddings[d] += v[d];
            }
        }
    }
    if (tc_count == 0) {
        return false;
    }
    for (size_t i=0; i<_dim; ++i) {
        out_embeddings[i] /= tc_count;
    }
    return true;
}

// Add terms' and querys' embeddings.
void 
Term2QueryDict_t::add_term(const string& term, float* embeddings) {
    map<string, int>::const_iterator it = _term2idx.find(term);
    // update old term.
    if (it!=_term2idx.end()) {
        int ind = it->second;
        memcpy(_term_embeddings+_offset(ind), embeddings, sizeof(float)*_dim);
        return ;
    }
   
    // add new term.
    int ind = term_num();
    _terms.push_back(term);
    _term2idx[term] = ind;

    _ensure_buffer_size((void**)&_term_embeddings, &_term_embeddings_buffer_size, sizeof(float)*_dim*(ind+1) );
    memcpy(_term_embeddings+_offset(ind), embeddings, sizeof(float)*_dim);
}

void 
Term2QueryDict_t::add_query(const string& query, float* embeddings) {
    map<string, int>::const_iterator it = _query2idx.find(query);
    // update old query.
    if (it!=_query2idx.end()) {
        int ind = it->second;
        memcpy(_query_embeddings+_offset(ind), embeddings, sizeof(float)*_dim);
        return ;
    }
   
    // add new query.
    int ind = query_num();
    _querys.push_back(query);
    _query2idx[query] = ind;

    _ensure_buffer_size((void**)&_query_embeddings, &_query_embeddings_buffer_size, sizeof(float)*_dim*(ind+1) );
    memcpy(_query_embeddings+_offset(ind), embeddings, sizeof(float)*_dim);
}

// Search and calculation..
float 
Term2QueryDict_t::dot_dist(float* emb_a, float* emb_b, size_t dim) {
    float res = 0;
    for (size_t i=0; i<dim; ++i) {
        res += emb_a[i] * emb_b[i];
    }
    return res;
}

bool 
Term2QueryDict_t::n_nearest(const string& query, vector<Result_t>& results, size_t output_top_N) const
{
    vector<CompareBlock_t> all_score;
    float *context_vector = new float[_dim];

    results.clear();

    if ( !vector_query(query, context_vector) ) {
        // query not found.
        return false;
    }

    // finding best match.
    if (_lsh) {
        _lsh->find_knearest(context_vector, _dim, output_top_N, all_score);

    } else {
        for (size_t qid=0; qid<query_num(); ++qid) {
            CompareBlock_t cb;
            cb.index = qid;
            cb.score = dot(context_vector, _query_embeddings+_offset(qid), _dim);
            all_score.push_back(cb);
        }

        if (output_top_N >= all_score.size()) {
            sort(all_score.begin(), all_score.end());
        } else {
            partial_sort(all_score.begin(), all_score.begin() + output_top_N, all_score.end());
        }

    }

    for (size_t i=0; i<output_top_N && i<all_score.size(); ++i) {
        Result_t res;
        res.query = _querys[ all_score[i].index ];
        res.score = all_score[i].score;
        results.push_back( res );
    }

    delete [] context_vector;
    return true;
}

bool 
Term2QueryDict_t::n_nearest(const vector<string>& terms, vector<Result_t>& results, size_t output_top_N) const
{
    vector<CompareBlock_t> all_score;
    float *context_vector = new float[_dim];
    float *mid_term = new float[_dim];

    results.clear();

    if ( !vector_terms(terms, context_vector) ) {
        return false;
    }

    // finding best match.
    if (_lsh) {
        _lsh->find_knearest(context_vector, _dim, output_top_N, all_score);

    } else {
        for (size_t qid=0; qid<query_num(); ++qid) {
            CompareBlock_t cb;
            cb.index = qid;
            cb.score = dot(context_vector, _query_embeddings+_offset(qid), _dim);
            all_score.push_back(cb);
        }

        if (output_top_N >= all_score.size()) {
            sort(all_score.begin(), all_score.end());
        } else {
            partial_sort(all_score.begin(), all_score.begin() + output_top_N, all_score.end());
        }
    }

    for (size_t i=0; i<output_top_N && i<all_score.size(); ++i) {
        Result_t res;
        res.query = _querys[ all_score[i].index ];
        res.score = all_score[i].score;
        results.push_back( res );
    }

    delete [] context_vector;
    delete [] mid_term;
    return true;
}


/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
