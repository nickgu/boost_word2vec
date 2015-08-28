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

#include "term2query_dict.h"

enum RunMode_t {
    SearchByQuery,
    SearchByTerm,
    CalcDistTerm2Query,
};

int main(int argc, const char** argv) {
    if (argc<2) {
        fprintf(stderr, "Usage:\n\t%s <term2query_dict> [query|term|dist]\n");
        return -1;
    }

    RunMode_t run_mode = SearchByQuery;
    if (argc>2) {
        if (strcmp(argv[2], "term")==0) {
            run_mode = SearchByTerm;
        } else if (strcmp(argv[2], "dist")==0) {
            run_mode = CalcDistTerm2Query;
        }
    }
    fprintf(stderr, "run_mode = [%d]\n", run_mode);

    Term2QueryDict_t t2q_dict; 
    t2q_dict.read(argv[1]);

    printf("Input query to search k-nearest query. (k=20)\n");
    char query[1024];
    vector<Term2QueryDict_t::Result_t> res;
    vector<string> terms;

    while (1) {
        fgets(query, sizeof(query), stdin);
        query[ strlen(query)-1 ] = 0;

        if (run_mode == SearchByQuery || run_mode == SearchByTerm) {
            if (run_mode == SearchByQuery) {
                t2q_dict.n_nearest(query, res);
            } else {
                split(query, " ", terms);
                for (size_t i=0; i<terms.size(); ++i) {
                    printf("%s,", terms[i].c_str());
                }
                printf("\n");
                t2q_dict.n_nearest(terms, res);
            }
            printf("-- Search results [%lu] --\n", res.size());
            for (size_t i=0; i<res.size(); ++i) {
                printf(" [%8.3f] [%s]\n", res[i].score, res[i].query.c_str());
            }
        } else if ( run_mode == CalcDistTerm2Query ) {
            split(query, " ", terms);
            float* v1 = new float [t2q_dict.dim()];
            float* v2 = new float [t2q_dict.dim()];

            string query = terms[terms.size()-1];
            terms.pop_back();

            printf("calc_dist: [");
            for (size_t i=0; i<terms.size(); ++i) {
                printf("%s,", terms[i].c_str());
            }
            printf("] => [%s]\n", query.c_str());

            bool v1_found = t2q_dict.vector_terms(terms, v1);
            bool v2_found = t2q_dict.vector_query(terms[1].c_str(), v2);

            if (v1_found && v2_found) {
                fprintf(stderr, "terms_vector:\n");
                Term2QueryDict_t::debug_vector(v1, t2q_dict.dim());
                fprintf(stderr, "querys_vector:\n");
                Term2QueryDict_t::debug_vector(v2, t2q_dict.dim());
                float dist = Term2QueryDict_t::dot_dist(v1, v2, t2q_dict.dim());
                printf("Dist : %.3f\n", dist);

            } else {
                LOG_ERROR("[%s] found=%d", terms[0].c_str(), v1_found);
                LOG_ERROR("[%s] found=%d", terms[1].c_str(), v2_found);
            }

            delete [] v1;
            delete [] v2;
        }
    }
    
    return 0;
}


/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
