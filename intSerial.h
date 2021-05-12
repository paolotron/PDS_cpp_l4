//
// Created by paolo on 11/05/21.
//

#ifndef LAB4_INTSERIAL_H
#define LAB4_INTSERIAL_H

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
namespace pt = boost::property_tree;
using namespace std;

class intSerial {
private:
    int value;
public:
    explicit intSerial(int n):value(n){};

    intSerial(){
        value = 0;
    }

    int getInt() const{
        return value;
    }

    void setInt(int n){
        value = n;
    }

    string serialize(){
        pt::ptree root;
        stringstream str;
        root.put("value", value);
        pt::json_parser::write_json(str, root, false);
        return str.str();
    }

    static intSerial deserialize(string js){
        pt::ptree root;
        stringstream  str(js);
        pt::json_parser::read_json(str, root);
        return intSerial(root.get<int>("value"));
    }
};

intSerial operator+(intSerial s1, const intSerial s2){
    return intSerial(s1.getInt()+s2.getInt());
}

#endif //LAB4_INTSERIAL_H
