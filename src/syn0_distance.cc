/***************************************************************************
 * 
 * Copyright (c) 2015 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file syn0_distance.cc
 * @author gusimiu(com@baidu.com)
 * @date 2015/08/24 19:52:44
 * @brief 
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

    // predict. 
    printf("input words(split by ' ') to find nearest term(without REC_ prefix.)\n> ");
    char line[1024];
    while (fgets(line, sizeof(line), stdin)) {
        // chomp.
        line[strlen(line)-1] = 0;

        size_t selected_N = 15;
        vector<Word2VecDict_t::Result_t> output_list = word2vec_dict.nearest_syn0(line, selected_N);
        for (size_t i=0; i<output_list.size(); ++i) {
            printf("%s\t%.3f\n", 
                    output_list[i].query.c_str(),
                    output_list[i].score
                );
        }
        printf("\n");
        fflush(stdout);
    }


    return 0;
}

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
