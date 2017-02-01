#ifndef LEPTJSON_H__
#define LEPTJSON_H__
#include "string"
#include "cassert"
#include "typeinfo"
#include "memory"
#include "vector"
#include "unordered_map"
#include "iostream"
typedef std::string string;
typedef enum { LEPT_NULL,LEPT_FALSE, LEPT_TRUE, LEPT_NUMBER, LEPT_STRING, LEPT_ARRAY, LEPT_OBJECT } lept_type;

class json_tree;
class json_value{
public:
    typedef std::shared_ptr<json_value> smart_value_ptr;
    typedef std::vector<smart_value_ptr> vec_val_ptr;
    typedef std::unordered_multimap<std::string,smart_value_ptr> map_object;

    lept_type type = LEPT_NULL;
    union{
    double d;
    std::string s;
    vec_val_ptr a;
    map_object o;
};

    json_value():type(LEPT_NULL){};
    json_value(const json_value& rhs){*this = rhs;}
    json_value (const double& _d) : type(LEPT_NUMBER),d(_d){}
    json_value (const int& _d) : type(LEPT_NUMBER),d(_d){}
    json_value (const bool _b) :type(_b?LEPT_TRUE:LEPT_FALSE){}
    json_value (const std::string& _s) :type(LEPT_STRING),s(_s){}
    json_value (const vec_val_ptr& _a) :type(LEPT_ARRAY),a(_a){}
    json_value (const map_object& _o) :type(LEPT_OBJECT),o(_o){}
    ~json_value() {set_null();}

    json_value& operator =(const json_value& rhs);
    operator double(){return get_number();}
    operator std::string(){return get_str();}
    operator bool() {return get_bool();}
    operator vec_val_ptr() {return get_array();}
    operator map_object() {return get_object();}
    void        fix_str(){assert(type==LEPT_STRING);s = s.substr(0,s.find('\0'));}

    smart_value_ptr index_of                         (size_t index)const;
    smart_value_ptr get_object_element        (const std::string& key)const;
    smart_value_ptr create_object_element   (const std::string& key);

    lept_type                  get_type() const{return type;}
    const double&          get_number() const {assert(type==LEPT_NUMBER);return d;}
    const std::string&     get_str()const{assert(type==LEPT_STRING);return s;}
    const vec_val_ptr&   get_array()const{assert(type==LEPT_ARRAY);return a;}
    const map_object&   get_object()const{assert(type==LEPT_OBJECT);return o;}
    const bool                 get_bool() const {
        assert(get_type()==LEPT_TRUE||get_type()==LEPT_FALSE);
        return (type==LEPT_TRUE);
    }

    void set_null();
    void set(const double _d);
    void set(const int _d);
    void set(const std::string& _s);
    void set(const char s[]);
    void set(const bool _b);
    void set(const json_tree& t);
    void set(const map_object&);
    void set(const vec_val_ptr&);
    /**add value**/
    void add(const std::string& key,const json_value& val);

};




class json_tree
{
public:

    json_value value;
    std::string s;
    std:: string::const_iterator p;
    std::  string::const_iterator end;

    int     lept_parse(const std::string& json);
    int     lept_parse_literal(json_value& val,const std::string literal,lept_type specified_type);
    int     lept_parse_value(json_value& val);
    void  lept_parse_whitespace();
    int     lept_parse_number(json_value& val);
    int     lept_parse_string(json_value& val);
    int     lept_parse_array(json_value& val);
    int     lept_parse_object(json_value& val);
    int    lept_stringify(std::string& str);
    lept_type lept_get_type() const{ return value.get_type();};


    template<typename T> T get(){
    assert(value.type!=LEPT_NULL);
    return value;
    }


    template<typename T> T get(const std::string& path) {
    assert(value.type==LEPT_OBJECT);
    return *value.get_object_element(path);
    }


    template<typename T> T get(const size_t& index){
        return *value.index_of(index);
    }

    template<typename T> T get(const std::initializer_list<size_t>& indexs){

        assert(indexs.size()!=0);
        auto ptr = &value;
        size_t pos = 0;

        for(const auto& pos:indexs){
            auto temp = ptr->index_of(pos);
            ptr = temp.get();
        }
        return *ptr;
    }

    void set_null();
    void set_val(const json_value& val);
    template<typename T>
    void set(const std::string& path,const T& val){
    assert(path.size()!=0);

     auto& child = *value.create_object_element(path);
     child.set(val);
}

    template<typename T>
    void set(const T& val){
        value.set(val);
    }

    void add_child(const std::string& path,const json_tree& t){
        assert(path.size()!=0);
        assert(lept_get_type()==LEPT_OBJECT);
        if(path.find('.')!=std::string::npos)
        {
            auto pos = path.rfind('.');
            auto sub_path = path.substr(0,pos);
            auto last = path.substr(pos+1);
            auto& child = *value.create_object_element(sub_path);
             if(child.get_type()==LEPT_NULL)
                child.set(json_value::map_object());
            child.add(last,t.get_value());
        }
        else{
                value.add(path,t.get_value());
        }
    }

    void append_child(const std::string& path,const json_tree& t){
        assert(lept_get_type()==LEPT_OBJECT);
        if(path.size()==0){
            value.add("",t.get_value());
        }

        auto& child = *value.create_object_element(path);
        if(child.get_type()==LEPT_NULL)
             child.set(json_value::vec_val_ptr());
        child.add("",t.get_value());
    }

    const json_value& get_value()const{return value;}
    //assist function
    int     lept_parse_hex4(unsigned& u);
};

enum {
    LEPT_PARSE_OK = 0,//code 0
    LEPT_PARSE_EXPECT_VALUE,//code 1 only contains whitespace
    LEPT_PARSE_ROOT_NOT_SINGULAR,// code 2after a value and whitespace, there are extra values
     LEPT_PARSE_INVALID_VALUE,    //code 3
    LEPT_PARSE_NUMBER_TOO_BIG,//too big  error code 4
    LEPT_PARSE_MISS_QUOTATION_MARK,//missing "  error code5
    LEPT_PARSE_INVALID_STRING_ESCAPE,//invalid \x in string code6
    LEPT_PARSE_INVALID_STRING_CHAR,// code 7
    LEPT_PARSE_INVALID_UNICODE_HEX,//code 8
    LEPT_PARSE_INVALID_UNICODE_SURROGATE,//code 9
    LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET,//code 10
    LEPT_PARSE_MISS_KEY,//code 11
    LEPT_PARSE_MISS_COLON,//code 12
    LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET,//code 13
    LEPT_STRINGIFY_OK//code 14
};

/*******hasher************/
class json_hasher{
    public:
        size_t operator() (const json_value& val) const{
            std::size_t ret = 0;
            switch(val.get_type()){
                case LEPT_ARRAY:
                    {
                        for(const auto& v_ptr:val.get_array())
                            ret^= json_hasher()(*v_ptr);
                        return ret;
                    }
                case LEPT_OBJECT:
                    {
                        for(const auto& pair:val.get_object())
                            ret^=json_hasher()(*pair.second);
                        return ret;
                    }
                case LEPT_STRING:return std::hash<std::string>()(val.get_str());
                case LEPT_NUMBER:return std::hash<size_t>()(val.get_number());
                case LEPT_NULL:return 0;
                default:return val.get_type();
            }

        }
};
bool operator== (const json_value& t1,const json_value& t2);
#endif  // LEPTJSON_H__
