/***************************************************************************
 * 
 * Copyright (c) 2014 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file dump_target_vector.c
 * @author gusimiu(com@baidu.com)
 * @date 2014/12/24 11:18:25
 * @brief 
 *      dump the target term's vector to file.
 *
 **/

#include <cstdio>
#include <cstdlib>

#include "helper.h"
#include "word2vec_dict.h"

int main(int argc, const char** argv) {
    // check param.
    const char* dict_filename = "./gsm_binary.w2v";
    if (argc > 1) {
        dict_filename = argv[1];
    }
    fprintf(stderr, "DICT_NAME: [[ %s ]]\n", dict_filename);
   
    // load dict.
    Word2VecDict_t word2vec_dict;
    word2vec_dict.load_bin(dict_filename);

    size_t term_count = word2vec_dict.term_count();
    size_t width = word2vec_dict.dim();

    fprintf(stdout, "%d %d\n", term_count, width);

    float* vec = new float[width];
    for (size_t wid=0; wid<term_count; ++wid) {
        const char* term = word2vec_dict.get_term(wid);
        bool res = word2vec_dict.vector_syn1neg(wid, vec);
        if (!res) {
            LOG_ERROR("Fetch vector of term_id[%d] failed!", wid);
        }

        fprintf(stdout, "%s", term);
        for (size_t d=0; d<width; ++d) {
            fprintf(stdout, " %.3f", vec[d]);
        }
        fprintf(stdout, "\n");
    } 
    delete [] vec;

    return 0;
}

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
