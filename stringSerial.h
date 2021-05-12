//
// Created by paolo on 11/05/21.
//

#ifndef LAB4_STRINGSERIAL_H
#define LAB4_STRINGSERIAL_H

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
namespace pt = boost::property_tree;
using namespace std;

class stringSerial{
private:
    string s{};
public:
    explicit stringSerial(std::string st){
        s = st;
    }

    string getString(){
        return s;
    }

    void setString(string st){
        s = st;
    }

    string serialize() const{
        pt::ptree root;
        stringstream str;
        root.put("value", s);
        pt::json_parser::write_json(str, root, false);
        return str.str();
    }

    static stringSerial deserialize(const string& js){
        pt::ptree root;
        stringstream  str(js);
        pt::json_parser::read_json(str, root);
        return stringSerial(root.get<string>("value"));
    }

};

int operator<(stringSerial s2, stringSerial s1){
    return s2.getString() < s1.getString();
}


#endif //LAB4_STRINGSERIAL_H
