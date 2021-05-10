//
// Created by paolo on 08/05/2021.
//

#ifndef LAB4_MAPREDUCE_H
#define LAB4_MAPREDUCE_H

#include <vector>
#include <map>
#include <functional>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>


using namespace std;

template<class MapperInputT, class ResultT, class KeyT, class acc>
map<KeyT, acc> mapreduce(MapperInputT& input,
                         function<vector<tuple<KeyT, ResultT>>(MapperInputT&)> Mapper,
                         function<tuple<KeyT, acc>(tuple<KeyT&, ResultT&, acc&>)> Reducer){
    map<KeyT, acc> accumulator = map<KeyT, acc>();
    KeyT key;
    ResultT mapres;
    vector<tuple<KeyT, ResultT>> res = Mapper(input);
    for(tuple<KeyT, ResultT> t: res){
        key = get<0>(t);
        mapres = get<1>(t);
        tuple<KeyT, acc> r = Reducer(tuple<KeyT&, ResultT&, acc&>(key, mapres, accumulator[key]));
        accumulator[get<0>(r)] = get<1>(r);
    }
    return accumulator;
}

template<class MapperInputT, class ResultT, class KeyT, class acc>
map<KeyT, acc> maprecuceMulti(MapperInputT& input,
                              function<vector<tuple<KeyT, ResultT>>(MapperInputT&)> Mapper,
                              function<tuple<KeyT, acc>(tuple<KeyT&, ResultT&, acc&>)> Reducer){
}

void test_multi(){
    if(fork()){
        cout << "Hi I'm 1" << '\n';
        wait(nullptr);
        cout << "Back Together" << '\n';
    }
    else{
        cout << "Hi I'm 2" << '\n';
        return;
    }
}

#endif //LAB4_MAPREDUCE_H
