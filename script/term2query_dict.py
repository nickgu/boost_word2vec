# -*- coding: utf-8 -*-
# gusimiu@baidu.com
# 

import py_term2query_dict

class Term2Query_TermDict_t:
    def __init__(self, filename):
        self.__handler = py_term2query_dict.read_terms(filename)

    def vector_terms(self, terms):
        return py_term2query_dict.vector_terms(self.__handler, terms)

class Term2Query_Dict_t:
    def __init__(self, filename):
        self.__handler = py_term2query_dict.read(filename)

    def vector_query(self, query):
        return py_term2query_dict.vector_query(self.__handler, query)

    def vector_terms(self, terms):
        return py_term2query_dict.vector_terms(self.__handler, terms)

    def dist_query_query(self, query_A, query_B):
        return py_term2query_dict.dist_query_query(self.__handler, query_A, query_B)

    def dist_terms_query(self, terms_A, query_B):
        return py_term2query_dict.dist_terms_query(self.__handler, terms_A, query_B)

    def dist_terms_terms(self, terms_A, terms_B):
        return py_term2query_dict.dist_terms_terms(self.__handler, terms_A, terms_B)

    def knearest(self, query_or_terms, k=20):
        if isinstance(query_or_terms, str):
            return py_term2query_dict.knearest_query(self.__handler, query_or_terms, k)
        elif isinstance(query_or_terms, list) or isinstance(query_or_terms, tuple):
            return py_term2query_dict.knearest_terms(self.__handler, query_or_terms, k)

    def vector_brief(self, v):
        s = ', '.join(map(lambda x:'%d:%.2f', sorted(filter(lambda x:abs(x[1])>.1, enumerate(v)), key=lambda x:-x[1])))
        return s

if __name__ == '__main__':
    import sys
    import pydev
    import wseg

    if len(sys.argv)<2:
        print >> sys.stderr, 'Usage: %s <dict> [--server <worddict>]' % (sys.argv[0])
        sys.exit(-1)
    tq_dict = Term2Query_Dict_t(sys.argv[1])
    worddict = None

    def process(query):
        arr = query.split(':')
        cmd = arr[0]
        info = ':'.join(arr[1:])
        if cmd == 'tv':
            # terms vector.
            terms = list(worddict.seg_phrase(info))
            v = tq_dict.vector_terms(terms)
            s = tq_dict.vector_brief(v)
            out = 'TERMS\t%s\n' % (','.join(terms))
            out += 'V\t%s\n' % s
            return out

        elif cmd == 'qv':
            v = tq_dict.vector_query(info)
            s = tq_dict.vector_brief(v)
            out = 'TERMS\t%s\n' % (','.join(terms))
            out += 'V\t%s\n' % s
            return out

        elif cmd == 'qq':
            a, b = info.split(':')
            dist = tq_dict.dist_query_query(a, b)
            return 'dist of [%s] and [%s] : %.4f' % (a, b, dist)

        elif cmd == 'tq':
            a, b = info.split(':')
            ta = list(worddict.seg_phrase(a))
            dist = tq_dict.dist_terms_query(ta, b)
            return 'dist of [%s]=[%s] and [%s] : %.4f' % (a, ','.join(ta), b, dist)

        elif cmd == 'tt':
            a, b = info.split(':')
            ta = list(worddict.seg_phrase(a))
            tb = list(worddict.seg_phrase(b))
            dist = tq_dict.dist_terms_terms(ta, tb)
            return 'dist of [%s]=[%s] and [%s]=[%s] : %.4f' % (a, ','.join(ta), b, ','.join(tb), dist)

        elif cmd == 't':
            terms = list(worddict.seg_phrase(info))
            out = 'TERMS\t%s\n' % (','.join(terms))
            res = tq_dict.knearest(terms, k=300)
            if res is None:
                return 'Not found.'
            out = '\n'.join(map(lambda x:'%s\t%.3f'%(x[0], x[1]), res))
            return out

        elif cmd == 'q':
            res = tq_dict.knearest(info, k=300)
            if res is None:
                return 'Not found.'
            out = '\n'.join(map(lambda x:'%s\t%.3f'%(x[0], x[1]), res))
            return out
           
        s = ''
        s += 'tv:<query>\tsegment query and get terms vector.\n'
        s += 'qv:<query>\tget query vector.\n'
        s += 'qq:<query>:<query>\tdist between query and query (no segment).\n'
        s += 'tq:<query>:<query>\tdist between terms and query\n'
        s += 'tt:<query>:<query>\tdist between terms and terms\n'
        s += 't:<query>\tsegment query and find k-nearest querys.\n'
        s += 'q:<query>\tdirectly search query to find k-nearest querys.\n'
        return s


    if len(sys.argv)>3 and sys.argv[2]=='--server':
        worddict = wseg.WordSeg(sys.argv[3])

        svr = pydev.BasicService()
        svr.set_process(process)
        svr.run_with_name(ip='127.0.0.1', port=25538, name='Term2QueryDict')

    else:
        print >> sys.stderr, 'input query to get res.'
        while 1:
            query = sys.stdin.readline()
            if query == '':
                break
            query = query.strip('\n')

            res = tq_dict.knearest(query)
            for query, score in res:
                print query, score


