/***************************************************************************
 * 
 * Copyright (c) 2015 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file convert_word2vec_term2query.cc
 * @author nickgu
 * @date 2015/08/27 15:35:09
 * @brief 
 *  
 **/

#include <cstdio>
#include "helper.h"

#include "word2vec_dict.h"
#include "term2query_dict.h"

int main(int argc, const char** argv) {
    if (argc!=3) {
        fprintf(stderr, "Usage:\n\t%s <input_word2vec_dict> <output_term2query_dict>\n");
        return -1;
    }

    Term2QueryDict_t t2q_dict; 
    Word2VecDict_t w2v_dict;
    w2v_dict.load_bin(argv[1]);
    size_t term_count = w2v_dict.term_count();
    size_t dim = w2v_dict.dim();

    string QueryPrefix = "REC_";
    float *emb = new float[dim];
    size_t tc=0, qc=0;
    t2q_dict.set_dim(w2v_dict.dim());
    for (size_t wid=0; wid<term_count; ++wid) {
        if (wid>0 && wid%100000 == 0) {
            LOG_NOTICE("Process: %lu", wid);
        }
        string term = w2v_dict.get_term(wid);
        if (term.substr(0, QueryPrefix.length()) == QueryPrefix) {
            w2v_dict.vector_syn1neg(wid, emb);
            t2q_dict.add_query(
                    term.substr(
                        QueryPrefix.length(), 
                        term.length() - QueryPrefix.length()), 
                    emb);
            qc++;
        } else {
            w2v_dict.vector_syn0(wid, emb);
            t2q_dict.add_term(term, emb);
            tc++;
        }
    }
    
    fprintf(stderr, "tc=%lu qc=%lu\n", tc, qc);
    delete [] emb;
    t2q_dict.write(argv[2]);
    return 0;
}


/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
