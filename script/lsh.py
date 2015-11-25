# -*- coding: utf-8 -*-
# gusimiu@baidu.com
# 

import py_lsh
import sys

class LSHDict:
    def __init__(self, filename, dim, aux_num=50):
        print >> sys.stderr, 'Building LSH of (%s) with dim=%d, aux=%d' % (filename, dim, aux_num)
        self.__dict_handle = py_lsh.read(filename, dim, aux_num)

    def search(self, name):
        ans_vec = py_lsh.search(self.__dict_handle, name)
        return ans_vec

    '''
        return a list sorted by score in desc-order:
        [(key, score), (key, score), ..]
    '''
    def knearest(self, vec, return_num=20):
        ans = py_lsh.knearest(self.__dict_handle, vec, return_num)
        return ans


if __name__ == '__main__':
    if len(sys.argv)!=3:
        print >> sys.stderr, 'Usage:\n\t%s <dense_vector_file> <dim>\n\t\tinput name to search 10-nearest.'
        sys.exit(-1)
    
    lsh_dict = LSHDict(sys.argv[1], int(sys.argv[2]))
    while 1:
        line = sys.stdin.readline()
        if line == '':
            break
        name = line.strip('\n')
        vec = lsh_dict.search(name)
        if vec is not None:
            print >> sys.stderr, 'Search over. vec=%s' % (str(vec))
            result_list = lsh_dict.knearest(vec)
            for idx, (key, score) in enumerate(result_list):
                print '%d.\t%s\t%.4f' % (idx+1, key, score)
        else:
            print >> sys.stderr, 'vector of Key=%s is not found.' % name

