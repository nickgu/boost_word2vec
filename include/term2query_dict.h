/***************************************************************************
 * 
 * Copyright (c) 2015 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file ../include/term2query_dict.h
 * @author nickgu
 * @date 2015/08/27 14:13:43
 * @brief 
 *      Use to predict query by term's vector.
 **/

#ifndef  ____TERM2QUERY_DICT_H_
#define  ____TERM2QUERY_DICT_H_

#include <string>
#include <vector>
#include <map>
#include <stdexcept>
using namespace std;


class LSHIndex_t;

class Term2QueryDict_t {
    public:
        struct Result_t {
            string query;
            float score;
        };

    public:
        Term2QueryDict_t ();
        ~Term2QueryDict_t ();

        // IO.
        void read(const char* filename, size_t build_lsh_num=0);
        void write(const char* filename);

        void set_dim(size_t dim) {
            if (_query_embeddings!=NULL || _term_embeddings!=NULL) {
                throw std::runtime_error("The Term2QueryDict is initialized. You cannot set dim.");
            }
            _dim = dim;
        }

        // Information.
        size_t dim() const { return _dim; }
        size_t query_num() const { return _query2idx.size(); }
        size_t term_num() const { return _term2idx.size(); }

        bool vector_query(const string& query, float* out_embeddings) const;
        bool vector_terms(const vector<string>& terms, float* out_embeddings) const;

        // Add terms' and querys' embeddings.
        void add_term(const string& term, float* embeddings);
        void add_query(const string& query, float* embeddings);

        // Search and calculation..
        static float dot_dist(float* emb_a, float* emb_b, size_t dim);

        bool n_nearest(const string& query, vector<Result_t>& results, size_t output_top_N=20) const;
        bool n_nearest(const vector<string>& terms, vector<Result_t>& results, size_t output_top_N=20) const;

        static void debug_vector(const float* vec, size_t dim);

    private:
        LSHIndex_t* _lsh;

        size_t _dim;
        map<string, int> _term2idx;
        map<string, int> _query2idx;
        vector<string> _terms;
        vector<string> _querys;

        float* _query_embeddings;
        size_t _query_embeddings_buffer_size;

        float* _term_embeddings;
        size_t _term_embeddings_buffer_size;

        size_t _offset(size_t index) const;
};


#endif  //____TERM2QUERY_DICT_H_

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
