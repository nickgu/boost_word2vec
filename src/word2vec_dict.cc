/***************************************************************************
 * 
 * Copyright (c) 2015 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file word2vec_dict.cc
 * @author nickgu
 * @date 2015/08/24 14:28:29
 * @brief 
 *  
 **/

#include <string>
#include <cmath>

#include "word2vec_dict.h"
#include "helper.h"

#define IDX(word_id, pos) (word_id * _hidden_size + pos)
#define OFFSET(word_id)   (word_id * _hidden_size)

vector<int> 
Word2VecDict_t::_word2term_id(const vector<string>& sentence) const {
    vector<int> output_list;
    for (size_t i=0; i<sentence.size(); ++i) {
        map<string, int>::const_iterator it = _word2idx_dict.find(sentence[i]);
        if (it != _word2idx_dict.end()) {
            output_list.push_back( it->second );
        } else {
            LOG_NOTICE("word [%s] not found.", sentence[i].c_str());
        }
    }
    return output_list;
}

void Word2VecDict_t::_clear() {
#define SAVE_FREE(p) {if (p != NULL) {free(p); p=NULL;} }
    SAVE_FREE(_word2vec);
    SAVE_FREE(_syn1_neg);
    SAVE_FREE(_syn1_hs);
    //SAVE_FREE(_word_tree);
    SAVE_FREE(_vocab);

    _word2idx_dict.clear();
}

void
Word2VecDict_t::load_bin(const char* filename) {
    _clear();

    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        throw runtime_error(string("Cannot load file [") + string(filename) + string("]"));
    }
    fread(&_vocab_size, sizeof(_vocab_size), 1, fp);
    fread(&_hidden_size, sizeof(_hidden_size), 1, fp);
    LOG_NOTICE("vocab_size: %lld, hidden_size: %lld", _vocab_size, _hidden_size);

    fread(&_hs_mode, sizeof(_hs_mode), 1, fp);
    fread(&_neg_mode, sizeof(_neg_mode), 1, fp);
    LOG_NOTICE("hierarchical softmax mode: %d", _hs_mode);
    LOG_NOTICE("negative sampling mode: %d", _neg_mode);

    // read vocab info.
    _vocab = (VocabWord_t*)malloc(sizeof(VocabWord_t) * _vocab_size);
    fread(_vocab, sizeof(VocabWord_t) * _vocab_size, 1, fp);
    LOG_NOTICE( "load vocab buffer over size:%.1fM.", sizeof(VocabWord_t)*_vocab_size/1e6 );

    for (int i=0; i<_vocab_size; ++i) {
        _word2idx_dict[_vocab[i].word] = i;
    }
    LOG_NOTICE( "insert word into word2index dict completed." );

    // read syn0 / word2vec.
    size_t sz = sizeof(float) * (_vocab_size * _hidden_size);
    _word2vec = (float*)malloc(sz);
    fread(_word2vec, sz, 1, fp);
    LOG_NOTICE( "load word2vec completed." );

    if (_neg_mode > 0) {
        size_t sz = sizeof(float) * (_vocab_size * _hidden_size);
        _syn1_neg = (float*)malloc(sz);
        fread(_syn1_neg, sz, 1, fp);
        LOG_NOTICE("load syn1 of negative mode over.");
    }
    if (_hs_mode) {
        LOG_ERROR("HS_MODE: not yet support.");
    }


    LOG_NOTICE(" ===> LOAD WORD2VEC DICT[%s] SUCCESS <=== ", filename);
    fclose(fp);
    return;
}

vector<Word2VecDict_t::Result_t> 
Word2VecDict_t::predict_neg(const vector<string>& sentence, size_t output_top_N) const {
    vector<Word2VecDict_t::Result_t> output_list;
    vector<int> words_id = _word2term_id(sentence);
    vector<CompareBlock_t> all_score;
    float *context_vector = new float[_hidden_size];

    // construct input vector.
    for (int i=0; i<_hidden_size; ++i) {
        context_vector[i] = 0.0f;
        for (size_t j=0; j<words_id.size(); ++j) {
            size_t wid = words_id[j];
            context_vector[i] += _word2vec[IDX(wid, i)];
        }
        context_vector[i] /= float(words_id.size());

    }

    // finding best match.
    for (int i=0; i<_vocab_size; ++i) {

        // trick filter: 
        //  only output term begin with 'REC_'
        if (strstr(_vocab[i].word, "REC_")==NULL) {
            continue;
        }

        CompareBlock_t cb;
        cb.wordid = i;
        cb.score = 0.0f;
        for (long long j=0; j<_hidden_size; ++j) {
            cb.score += context_vector[j] * _syn1_neg[IDX(i, j)];
        }

        all_score.push_back(cb);
        //fprintf(stderr, "[%s][%.3f]\n", _vocab[i].word, cb.score);
    }
    partial_sort(all_score.begin(), all_score.begin() + output_top_N, all_score.end());
    for (size_t i=0; i<output_top_N && i<all_score.size(); ++i) {
        Result_t res;
        res.query = string( _vocab[all_score[i].wordid].word );
        res.score = all_score[i].score;
        output_list.push_back( res );
    }

    delete [] context_vector;
    return output_list;
}

vector<Word2VecDict_t::Result_t> 
Word2VecDict_t::nearest_syn0(const char* term, size_t output_top_N) const {
    vector<Word2VecDict_t::Result_t> output_list;

    int word_id;
    map<string, int>::const_iterator it = _word2idx_dict.find(term);
    if (it != _word2idx_dict.end()) {
        word_id = it->second;
    } else {
        LOG_NOTICE("word [%s] not found.", term);
        return output_list;
    }

    vector<CompareBlock_t> all_score;
    float *context_vector = new float[_hidden_size];

    // construct input vector.
    for (int i=0; i<_hidden_size; ++i) {
        context_vector[i] = _word2vec[IDX(word_id, i)];
    }

    // finding best match.
    for (int i=0; i<_vocab_size; ++i) {
        // trick filter: 
        //  only output term begin with 'REC_'
        if (strstr(_vocab[i].word, "REC_")!=NULL) {
            continue;
        }

        CompareBlock_t cb;
        cb.wordid = i;
        cb.score = 0.0f;
        for (long long j=0; j<_hidden_size; ++j) {
            cb.score += context_vector[j] * _word2vec[IDX(i, j)];
        }

        all_score.push_back(cb);
        //fprintf(stderr, "[%s][%.3f]\n", _vocab[i].word, cb.score);
    }
    partial_sort(all_score.begin(), all_score.begin() + output_top_N, all_score.end());
    for (size_t i=0; i<output_top_N && i<all_score.size(); ++i) {
        Result_t res;
        res.query = string( _vocab[all_score[i].wordid].word );
        res.score = all_score[i].score;
        output_list.push_back( res );
    }

    delete [] context_vector;
    return output_list;
}


bool
Word2VecDict_t::vector_syn0(size_t word_id, float* output_buffer) const 
{
    memcpy(output_buffer, _word2vec + OFFSET(word_id), sizeof(float) * _hidden_size);
    return true;
}


bool 
Word2VecDict_t::vector_syn1neg(size_t word_id, float* output_buffer) const
{
    memcpy(output_buffer, _syn1_neg + OFFSET(word_id), sizeof(float) * _hidden_size);
    return true;
}

const char* 
Word2VecDict_t::get_term(size_t word_id) const {
    return _vocab[word_id].word;
}


/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
