//
// Created by paolo on 10/05/2021.
//

#ifndef LAB4_COORDINATOR_H
#define LAB4_COORDINATOR_H

#define WITH_SELECT 0

#include <functional>
#include <map>
#include <vector>
#include <list>
#include <fstream>
#include <iostream>
#include "stringSerial.h"
#include <initializer_list>

using namespace std;

template<class MapperInputT, class KeyT,  class ResultT, class AccumulatorT>
class Coordinator {
private:
    function<vector<tuple<KeyT, ResultT>>(MapperInputT&)> Mapper;
    function<tuple<KeyT, AccumulatorT>(tuple<KeyT, ResultT, AccumulatorT>)> Reducer;
public:
    explicit Coordinator(function<vector<tuple<KeyT, ResultT>>(MapperInputT&)> Mapper,
    function<tuple<KeyT, AccumulatorT>(tuple<KeyT, ResultT, AccumulatorT>)> Reducer): Mapper(Mapper), Reducer(Reducer){}


    map<KeyT, ResultT> compute(ifstream& MyReadFile){
        map<KeyT, AccumulatorT> result{};
        int pipe_map[2];
        int pipe_map_back[2];
        int pipe_red[2];
        int pipe_red_back[2];
        pipe(pipe_map);
        pipe(pipe_red);
        pipe(pipe_map_back);
        pipe(pipe_red_back);

        if(!fork())
            map_proc(pipe_map, pipe_map_back);
        if(!fork())
            red_proc(pipe_red, pipe_red_back);
        string buf;
#if WITH_SELECT
        list<tuple<KeyT, ResultT>> mapperResult{};
        list<tuple<KeyT, ResultT>> tempResult{};
        int counter = 0;
        while(getline(MyReadFile, buf)){
            write_to_map_pipe(pipe_map, MapperInputT(buf));
            counter++;
        }
        for(int i=0; i<counter; i++){
            tempResult = read_from_map_pipe(pipe_map_back);
            mapperResult.splice(mapperResult.end(), tempResult);
        }
        writeTerminator(pipe_map);
        counter = 0;
        for(tuple<KeyT, ResultT> elem: mapperResult){
            write_to_reducer_pipe(pipe_red,get<0>(elem), get<1>(elem), result[get<0>(elem)]);
            auto temp = read_from_reducer_pipe(pipe_red_back);
            result[get<0>(temp)] = get<1>(temp);
        }
        writeTerminator(pipe_red);
        return result;
#endif
        list<tuple<KeyT, ResultT>> mapperResult{};
        list<tuple<KeyT, ResultT>> tempResult{};
        bool wait_result = false;
        bool start = true;
        while (true) {
            int counter = 0;
            fd_set read_set;
            fd_set write_set;
            FD_SET(pipe_map[1], &write_set);
            FD_SET(pipe_red[1], &write_set);
            FD_SET(*pipe_map_back, &read_set);
            FD_SET(*pipe_red_back, &read_set);
            timeval time{};
            time.tv_sec = 0;
            time.tv_usec = 0;
            int max = std::max({pipe_map[1], pipe_red[1], *pipe_red_back, *pipe_map_back});
            int ret = select(max + 1, &read_set, &write_set, nullptr, &time);
            switch (ret) {
                case -1:
                    cout << "FATAL ERROR" << '\n';
                    exit(-2);
                case 0:
                    break;
                default:
                    if(FD_ISSET(*pipe_map_back, &read_set)){
                        tempResult = read_from_map_pipe(pipe_map_back);
                        mapperResult.splice(mapperResult.end(), tempResult);
                        start = false;
                    }
                    if(FD_ISSET(*pipe_red_back, &read_set) && wait_result){
                        auto temp = read_from_reducer_pipe(pipe_red_back);
                        result[get<0>(temp)] = get<1>(temp);
                        wait_result = false;
                    }
                    if(FD_ISSET(pipe_map[1], &write_set)){
                        if(getline(MyReadFile, buf)) {
                            write_to_map_pipe(pipe_map, MapperInputT(buf));
                        }
                    }
                    if(FD_ISSET(pipe_red[1], &write_set) && !wait_result && !mapperResult.empty()){
                        counter++;
                        auto elem = mapperResult.front();
                        mapperResult.pop_front();
                        write_to_reducer_pipe(pipe_red, get<0>(elem), get<1>(elem), result[get<0>(elem)]);
                        wait_result = true;
                    }
            }
            if(!wait_result && MyReadFile.eof() && !start){
                break;
            }
        }
        writeTerminator(pipe_map);
        writeTerminator(pipe_red);
        return result;
    }


    void write_to_map_pipe(int *fd, const MapperInputT& in ){
        string js = in.serialize();
        writeStringToPipe(fd, js);
    }

    void write_to_reducer_pipe(int *fd, KeyT key, ResultT& reducerInput, AccumulatorT accumulator){
        writeStringToPipe(fd, key.serialize());
        writeStringToPipe(fd, reducerInput.serialize());
        writeStringToPipe(fd, accumulator.serialize());
    }

    list<tuple<KeyT, ResultT>> read_from_map_pipe(int *pInt) {
        list<tuple<KeyT, ResultT>> result{};
        while(true) {
            int size = readSize(pInt);
            if(size == -1 || size == 0)
                return result;
            char buf1[size];
            string temp;
            read(pInt[0], buf1, (size + 1) * sizeof(char));
            temp = string(buf1);
            KeyT key = KeyT::deserialize(temp);
            size = readSize(pInt);
            char buf2[size + 10];
            read(pInt[0], buf2, (size + 1) * sizeof(char));
            temp = string(buf2);
            ResultT res = ResultT::deserialize(temp);
            result.push_back(tuple<KeyT, ResultT>(key, res));
        }
    }

    tuple<KeyT, AccumulatorT> read_from_reducer_pipe(int *pInt){
        int size = readSize(pInt);
        char buf1[size];
        string temp;
        read(pInt[0], buf1, (size + 1) * sizeof(char));
        temp = string(buf1);
        KeyT key = KeyT::deserialize(temp);
        size = readSize(pInt);
        char buf2[size];
        read(pInt[0], buf2, (size + 1) * sizeof(char));
        temp = string(buf2);
        AccumulatorT res = AccumulatorT::deserialize(temp);
        return tuple(key, res);
    }

    void map_proc(const int *fin, const int *fout){
        unsigned char buf[255];
        string wbuf;
        int size;
        while(true) {
            size = readSize(fin);
            if(size == -1|| size == 0){
                exit(0);
            }
            read(fin[0], buf, sizeof(char)*(size+1));
            auto obj = string((char*)buf);
            MapperInputT m = MapperInputT::deserialize(obj);
            vector<tuple<KeyT, ResultT>> res_vec = this->Mapper(m);
            for(tuple<KeyT, ResultT> res: res_vec) {
                KeyT key = get<0>(res);
                ResultT value = get<1>(res);
                string json_key = key.serialize();
                string json_value = value.serialize();
                writeStringToPipe(fout, json_key);
                writeStringToPipe(fout, json_value);
            }
            writeTerminator(fout);
        }
    }
    void red_proc(int *fin, int *fout){
        unsigned char buf[255];
        int size;
        while(true){
            size = readSize(fin);
            if(size == -1 || size == 0)
                exit(0);
            read(fin[0], buf, sizeof(char)*(size+1));
            KeyT key = KeyT::deserialize(string((char*)buf));
            read(fin[0], buf, sizeof(char)*(readSize(fin)+1));
            ResultT res = ResultT::deserialize(string((char*)buf));
            read(fin[0], buf, sizeof(char)*(readSize(fin)+1));
            AccumulatorT acc = AccumulatorT::deserialize(string((char*)buf));
            tuple<KeyT, AccumulatorT> result = this->Reducer(tuple(key, res, acc));
            writeStringToPipe(fout, get<0>(result).serialize());
            writeStringToPipe(fout, get<1>(result).serialize());
        }

    }

    void writeStringToPipe(const int *pInt, const string json){
        string out = "<"+to_string(json.length())+">"+json;
        write(pInt[1], out.c_str(), sizeof(char)*(out.length()+1));
    }

    void writeTerminator(const int *pInt){
        string s = "<0>";
        write(pInt[1], s.c_str(), (s.length())*sizeof(char));
    }

    int readSize(const int *pInt){
        char buf[255];
        string size = "";
        do {
            read(pInt[0], buf, sizeof(char));
        } while (buf[0] != '<');
        read(pInt[0], buf, sizeof(char));
        do{
            char n = buf[0];
            size.push_back(n);
            read(pInt[0], buf, sizeof(char));
        } while(buf[0] != '>');
        return stoi(size);
    }

};


#endif //LAB4_COORDINATOR_H
