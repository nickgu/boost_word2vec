/***************************************************************************
 * 
 * Copyright (c) 2014 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file word2vec_predict.c
 * @author gusimiu(com@baidu.com)
 * @date 2014/12/24 11:18:25
 * @brief 
 *  a simple demo to predict word by word2vec algorithm.
 *      - use cbow output.
 *      - you need to use word2vec_gsm to train the model.
 *          original program doesn't output layer-2 weight.
 *  predict the next word of input window: [n-1:-1]
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
    printf("input words(split by ' ') to predict next word.\n> ");
    char line[1024];
    while (fgets(line, sizeof(line), stdin)) {
        // chomp.
        line[strlen(line)-1] = 0;

        vector<string> sentence;
        split(line, " ", sentence);

        size_t selected_N = 100;
        vector<Word2VecDict_t::Result_t> output_list = word2vec_dict.predict_neg(sentence, selected_N);
        printf("Predict output[%u/%d]:\n", output_list.size(), (int)selected_N);
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
