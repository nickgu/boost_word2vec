/***************************************************************************
 * 
 * Copyright (c) 2015 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file word2vec_dict.h
 * @author gusimiu(com@baidu.com)
 * @date 2015/08/24 14:23:41
 * @brief 
 *  
 **/

#ifndef  __WORD2VEC_DICT_H_
#define  __WORD2VEC_DICT_H_

#include <vector>
#include <algorithm>
#include <map>
#include <stdexcept>
using namespace std;

class Word2VecDict_t {
public:
    struct Result_t {
        string query;
        float score;
    };

    // macro defined in word2vec.c
    static const size_t MaxStringLength = 100;
    static const size_t MaxCodeLength = 40;

    Word2VecDict_t():
        _word2vec(NULL),
        _syn1_hs(NULL),
        _syn1_neg(NULL),
        //_word_tree(NULL),
        _vocab(NULL)
    {}

    ~Word2VecDict_t() {
        _clear();
    }

    /**
     *  load binary file.
     */
    void load_bin(const char* filename);

    //vector<string> predict_hs(const vector<string>& sentence, size_t output_top_N=1) const;
    vector<Result_t> predict_neg(const vector<string>& sentence, size_t output_top_N=1) const;
    vector<Result_t> nearest_syn0(const char* term, size_t output_top_N=1) const;

    size_t term_count() const { return _vocab_size; }
    size_t dim() const { return _hidden_size; } 
    const char* get_term(size_t word_id) const;

    bool vector_syn0(size_t word_id, float* output_buffer) const;
    bool vector_syn1neg(size_t word_id, float* output_buffer) const;

private:
    struct CompareBlock_t {
        float score;
        int wordid;

        bool operator< (const CompareBlock_t& o) const {
            return score > o.score;
        }
    };

    /**
     * structure from word2vec.c
     */
    struct VocabWord_t {
      long long freq;
      int point[MaxCodeLength];
      char word[MaxStringLength];
      char code[MaxCodeLength];
      char codelen;
    };

    float *_word2vec;       // word->vec buffer = syn1
    float *_syn1_hs;        // syn1 buffer
    float *_syn1_neg;       // syn1 negative
    //TreeNode_t*             _word_tree;     // Haffuman tree node.
    VocabWord_t*            _vocab;

    long long           _vocab_size;    // count of word.
    long long           _hidden_size;   // sizeof(hidden layer) = sizeof(word vector)
    int                 _hs_mode;       // hierarchical softmax mode.
    int                 _neg_mode;      // negative sampling mode.

    map<string, int>    _word2idx_dict; // word to index mapping.

    void    _clear();
    vector<int> _word2term_id(const vector<string>& sentence) const;

};

#endif  //__WORD2VEC_DICT_H_

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
