//
// Created by paolo on 10/05/2021.
//

#ifndef LAB4_COORDINATOR_H
#define LAB4_COORDINATOR_H

#include <functional>
#include <map>
#include <vector>

using namespace std;

template<class MapperInputT, class KeyT,  class ResultT>
class Coordinator {
private:
    function<vector<tuple<KeyT, ResultT>>(MapperInputT&)> Mapper;
    function<tuple<KeyT, ResultT>(tuple<KeyT&, ResultT&, ResultT&>)> Reducer;
public:
    Coordinator(): Mapper(Mapper), Reducer(Reducer){}

    map<KeyT, ResultT> compute(){
        map<KeyT, ResultT> result{};
        return result;
    }
};


#endif //LAB4_COORDINATOR_H
