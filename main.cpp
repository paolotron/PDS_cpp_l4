#include <iostream>
#include <fstream>
#include <map>
#include <functional>
#include <regex>
#include "mapreduce.h"
#include <set>
#include <vector>

using namespace std;

template<typename key, typename value>
void print_map(map<key, value> &m){
    for(auto el: m)
        cout << get<0>(el) << ": " << get<1>(el) << '\n';
}

void count_ip(ifstream& MyReadFile){

    function<vector<tuple<string, int>>(ifstream&)> mapper = [](ifstream& is)->vector<tuple<string, int>>{
        vector<tuple<string, int>> res;
        string s;
        while (getline (is, s)) {
            res.emplace_back(s.substr(0, s.find(' ')), 1);
        }
        return res;
    };
    function<tuple<string, int>(tuple<string&, int&, int&>)> reducer = [](tuple<string&, int&, int&> in)->tuple<string, int>{
        return tuple<string, int>(get<0>(in), get<1>(in)+get<2>(in));
    };
    auto res = mapreduce<ifstream, int, string, int>(MyReadFile, mapper, reducer);
    print_map(res);
}
void count_requests(ifstream& MyReadFile){
    function<vector<tuple<string, int>>(ifstream&)> mapper = [](ifstream& is)->vector<tuple<string, int>>{
        vector<tuple<string, int>> res;
        string s;
        while (getline (is, s)) {
            s = s.substr(s.find('['), s.find(']') - s.find('['));
            s = s.substr(s.find(':')+1,2);
            res.emplace_back(tuple<string, int>(s, 1));
        }
        return res;
    };
    function<tuple<string, int>(tuple<string&, int&, int&>)> reducer = [](tuple<string&, int&, int&> in)->tuple<string, int>{
        return tuple<string, int>(get<0>(in), get<1>(in)+get<2>(in));
    };
    auto res = mapreduce<ifstream, int, string, int>(MyReadFile, mapper, reducer);
    print_map(res);
}
void count_most_url(ifstream& MyReadFile){
    function<vector<tuple<string, int>>(ifstream&)> mapper = [](ifstream& is)->vector<tuple<string, int>>{
        vector<tuple<string, int>> res;
        string s;
        string reg = R"lit(([0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}) - - \[(.*)\] "([^\ ]*) (\/.*) (.+)" (.*))lit";
        regex rgx(reg);
        smatch match;
        while (getline (is, s)) {
            regex_search(s, match, rgx);
            res.emplace_back(tuple<string, int>(match[4].str(), 1));
        }
        return res;
    };
    function<tuple<string, int>(tuple<string&, int&, int&>)> reducer = [](tuple<string&, int&, int&> in)->tuple<string, int>{
        return tuple<string, int>(get<0>(in), get<1>(in)+get<2>(in));
    };
    auto res = mapreduce<ifstream, int, string, int>(MyReadFile, mapper, reducer);
    print_map(res);
}
void find_attackers(ifstream& MyReadFile){
    function<vector<tuple<string, string>>(ifstream&)> mapper = [](ifstream& is)->vector<tuple<string, string>>{
        vector<tuple<string, string>> res;
        string s;
        string reg = R"lit(([0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}) - - \[(.*)\] "([^\ ]*) (\/.*) (.+)" ([0-8]+) ([0-9]+))lit";
        regex rgx(reg);
        set<string> codes = {"404", "404", "405"};
        smatch match;
        while (getline (is, s)) {
            regex_search(s, match, rgx);
            if(codes.find(match[6].str()) != codes.end())
                res.emplace_back(tuple<string, string>(match[1].str(), match[6].str()));
        }
        return res;
    };
    function<tuple<string, string>(tuple<string&, string&, string&>)> reducer = [](tuple<string&, string&, string&> in)->tuple<string, string>{
        return tuple<string, string>(get<0>(in), "");
    };
    auto res = mapreduce<ifstream, string, string, string>(MyReadFile, mapper, reducer);
    print_map(res);
}

void test_serialization(){
    ptree p;
    p.
}

int main() {

    string myText;
    ifstream MyReadFile("../log.txt");
    find_attackers(MyReadFile);
    test_multi();
    MyReadFile.close();
    return 0;
}
