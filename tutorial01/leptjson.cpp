#include "leptjson.h"
#include "cassert"  /* assert() */
#include "iostream"
#include "stdexcept"
#include "stdlib.h"
#include "string"
#include "math.h"
#include "unordered_set"
#include "functional"

#define EXPECT(p, ch)      do { assert(*p == (ch)); ++p; } while(0)
#define PUT(ch) do{buffer.push_back(ch); ++p;}while(0);break
#define BETWEEN(ch,a,b) ((ch)>=a &&(ch)<=b)
#define INSERT_JSON_VALUE(_beg,_end,group)  do{for(auto iter = _beg;iter!=_end;++iter) group.insert(*(iter->second));}while(0)

#define CHECK_CONDITION(cond) do{ if(!(cond)) return false;}while(0)

bool check_hex4(std::string::const_iterator p,unsigned& u) {
    for(size_t i = 0;i<4;++i){
        char ch = *p++;
        if(BETWEEN(ch,'0','9')){u = (u<<4)+int(ch-'0');}
        else if (BETWEEN(ch,'A','F')){u = (u<<4)+int(ch-'A')+10;}
         else if (BETWEEN(ch,'a','f')){u = (u<<4)+int(ch-'a')+10;}
        else return false;
    }
     return true;
}
bool is_ws(std::string::const_iterator p){
        return (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r');
    }
void json_tree:: lept_parse_whitespace() {
    while (is_ws(p))
        ++p;
}


int json_tree:: lept_parse_literal(json_value& val,const std::string literal,lept_type specified_type){
    EXPECT(p,literal[0]);
    size_t len = literal.size()-1;
    if ((p+len)>end || std::string(p,p+len) != literal.substr(1)) {//note: use string(p,p+3)=='null' may be out of bound
        return LEPT_PARSE_INVALID_VALUE;
    }
        p += len;
        val.type = specified_type;
        return   LEPT_PARSE_OK;
}


int json_tree:: lept_parse_number(json_value& val)
{
    int sign = 1;
    char *endptr;

    if(*p=='-'){ ++p;sign=-1;}
    std::string temp_json = std::string(p,end);
    const char* c = temp_json.c_str();
    if (*p<'0'|*p>'9'){return LEPT_PARSE_INVALID_VALUE;}//+ or other non-number is not allowed
    if (*p == '0'&&(*(p+1)>='0'&&*(p+1)<='9')){return LEPT_PARSE_INVALID_VALUE;}// 0xxx is invalid}

    auto result = sign*strtod(c,&endptr);
    p+=(endptr-c);

    if(p!=end&&!is_ws(p)&&*p!='}'&&*p!=']'&&*p!=','){return LEPT_PARSE_INVALID_VALUE;}//123.5X
    if (result == HUGE_VAL|result == -HUGE_VAL){return LEPT_PARSE_NUMBER_TOO_BIG;}//too big
    if (*(p-1)=='.') {return LEPT_PARSE_INVALID_VALUE;}//deal with at least one digit after point
    val.type = LEPT_NUMBER;
    val.set(result);

    return LEPT_PARSE_OK;
}

void OutputByte(json_value& val,unsigned u, std::string& s){
    s.push_back(u);
}

int json_tree::lept_parse_hex4(unsigned& u)//convert /uxxxx -> unsigned
{
    u = 0;
    EXPECT(p,'u');
    unsigned u2 = 0;

    if ((p+4>end)||!check_hex4(p,u)){return LEPT_PARSE_INVALID_UNICODE_HEX;}
    p+=4;
    if (u>=0xD800&&u<=0xDBFF){
        if (*p++!='\\'||*p++!='u') {return LEPT_PARSE_INVALID_UNICODE_SURROGATE;}

        if (p+4>end||!check_hex4(p,u2)||!BETWEEN(u2,0xDC00,0xDFFF)){
            return LEPT_PARSE_INVALID_UNICODE_SURROGATE;
        }
            p+=4;
            u = 0x10000+(u-0xD800)*0x400+(u2-0xDC00);
    }
    return LEPT_PARSE_OK;
}
void lept_parse_code_point(json_value& val,unsigned& u,std::string& s){
    assert(u<=0x10FFFF);
    if(u<0x007F){
            OutputByte(val,u,s);
    }
    else if(u<0x07FF){
        OutputByte(val,0xC0|((u>>6) & 0x1F),s);//110xxxx
        OutputByte(val,0x80|(u          &0x3F),s);
    }
    else if(u<0xFFFF){
        OutputByte(val,0xE0|((u>>12) & 0xF),s);
        OutputByte(val,0x80|((u>>6)   &0x3F),s);
        OutputByte(val,0x80|(u           &0x3F),s);
    }
    else {

        OutputByte(val,0xF0|((u>>18)   & 0x7),s);
        OutputByte(val,0x80|((u>>12)   &0x3F),s);
        OutputByte(val,0x80|((u>>6)     &0x3F),s);
        OutputByte(val,0x80|(u           &0x3F),s);
    }
}

int json_tree::lept_parse_string(json_value& val)
{
    EXPECT(p,'\"');
    std::string buffer;
    unsigned u;
    while(1){
        char ch = *(p++);
        switch(ch){
            case '\"':
                val.set(buffer);
                val.fix_str();// to remove \0 from the string

                return LEPT_PARSE_OK;
            case '\0':
                return LEPT_PARSE_MISS_QUOTATION_MARK;
            case '\\':
                switch(*p){
                    case 'n':PUT('\n');
                    case 'b':PUT('\b');
                    case 'f':PUT('\f');
                    case 'r':PUT('\r');
                    case 't':PUT('\t');
                    case '\\':PUT('\\');
                    case '/':PUT('/');
                    case '"':PUT('"');
                    case 'u':
                        {
                            int ret = lept_parse_hex4(u);
                            if(ret!=LEPT_PARSE_OK) return ret;
                            lept_parse_code_point(val,u,buffer);
                            break;
                        }
                    default:return LEPT_PARSE_INVALID_STRING_ESCAPE;
                }
                break;
            default:
                if (ch>=1 && ch<=31){return LEPT_PARSE_INVALID_STRING_CHAR;}
                buffer.push_back(ch);
        }//switch end
    }
}

int json_tree::lept_parse_array(json_value& val)
    {
       val.set(json_value::vec_val_ptr());
        int ret;
        EXPECT(p,'[');
        if(*p==']')
          {
            ++p;
            val.type = LEPT_ARRAY;
            return LEPT_PARSE_OK;
          }
        while(1)
        {

            json_value child_val;
            if((ret=lept_parse_value(child_val))!=LEPT_PARSE_OK) {
                return ret;
            }

            val.a.push_back(std::make_shared<json_value>(child_val));

            lept_parse_whitespace();
            if(*p==','){ ++p;lept_parse_whitespace();}
            else if (*p==']'){
                ++p;
                val.type = LEPT_ARRAY;
                return LEPT_PARSE_OK;
            }
            else
                return LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
        }
    }

int json_tree::lept_parse_object(json_value& val){
    val.set(json_value::map_object());
    int ret;
    EXPECT(p,'{');
    lept_parse_whitespace();

    /*return null object*/
    if(*p=='}'){
        ++p;
        val.type = LEPT_OBJECT;
        return LEPT_PARSE_OK;
    }
    while(1){
        if((*p)!='\"'){return LEPT_PARSE_MISS_KEY;}
        /*parse key*/
        json_value key_v;
        auto v_ptr = std::make_shared<json_value> ();

        if((ret=lept_parse_string(key_v)) !=LEPT_PARSE_OK ){break;}
        std::string key = key_v;

        lept_parse_whitespace();
        if(*p != ':'){return LEPT_PARSE_MISS_COLON;}
        ++p;

        lept_parse_whitespace();
        if((ret=lept_parse_value(*v_ptr)) !=LEPT_PARSE_OK){ break;};
        val.o.insert({key,v_ptr});
        /*comma or right_curly_breacket*/
        lept_parse_whitespace();
        if(*p==','){++p;lept_parse_whitespace();}
        else if(*p == '}'){++p;val.type = LEPT_OBJECT;ret = LEPT_PARSE_OK;break;}
        else{ret = LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET;break;}
    }

return ret;
}


int json_tree::  lept_parse_value(json_value& val)
{
    if (p==end){
        return LEPT_PARSE_EXPECT_VALUE;
    }
    switch (*p) {
        case 'n':  return lept_parse_literal(val,"null",LEPT_NULL);
        case 'f': return lept_parse_literal(val,"false",LEPT_FALSE);
        case 't':return  lept_parse_literal(val,"true",LEPT_TRUE);
        case '\"': return lept_parse_string(val);
        case '[':return lept_parse_array(val);
        case '{':return lept_parse_object(val);
        default: return lept_parse_number(val);
    }
}

int json_tree:: lept_parse( const std::string& json) {
    p = json.begin();
    end = json.end();
    json_value val;
    int ret;
    lept_parse_whitespace();// strip all the whitespace before parse value
    if( (ret=lept_parse_value(val))==LEPT_PARSE_OK)
    {
    lept_parse_whitespace();// to deal with the last whitespace
         if (p!=end){
         val.set_null();
         ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
         }
    }
    else{
        val.set_null();
    }
    value = val;

    return ret;
}
/**************generator********************/
int lept_stringify_value(json_value& val,std::string& str);
void lept_stringify_object(json_value& val,std::string& str);
void lept_stringify_array(json_value& val,std::string& str);



void lept_stringify_string(const std::string& jstr,std::string& str){
     static const char hex_digits[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    str.push_back('\"');
    for(auto const &ch:jstr){
        switch(ch){
            case '\n':str.append("\\n");break;
            case '\t':str.append("\\t");break;
            case '\r':str.append("\\r");break;
            case '\b':str.append("\\b");break;
            case '\f':str.append("\\f");break;
            case '\\': str.append("\\\\");break;
            case '/':str.append("/");break;
            case '\"':str.append("\\\"");break;
            default:
                if (ch < 0x20) {
                    str.append("\\u00");
                    str.push_back( hex_digits[ch >> 4]);
                    str.push_back(hex_digits[ch&15]);
                }
                else
                    str.push_back(ch);
                break;
        }
    }
    str.push_back('\"');
}
void lept_stringify_array(json_value& val,std::string& str){
    str.push_back('[');
    for(auto const& v_ptr:val.get_array()){
        lept_stringify_value(*v_ptr,str);
        str.push_back(',');
    }
    str.replace(str.end()-1,str.end(),"]");
}

void lept_stringify_object(json_value& val,std::string& str){

    auto obj = val.get_object();
    if (obj.size()==0) {str.append("{}");return;}

    str.push_back('{');
    for(auto const& pair : obj){
        lept_stringify_string(pair.first,str);
        str.push_back(':');
        lept_stringify_value(*pair.second,str);
        str.push_back(',');
    }

    str.replace(str.end()-1,str.end(),"}");
}

int lept_stringify_value(json_value& val,std::string& str)
{
    int ret;
    switch(val.get_type()){
        case LEPT_NULL:str.append("null");break;
        case LEPT_FALSE:str.append("false");break;
        case LEPT_TRUE:str.append("true");break;
        case LEPT_NUMBER:
                {
                    str.append(std::to_string(val.get_number()));
                    break;
                }
        case LEPT_STRING: lept_stringify_string(val.get_str(),str);break;
        case LEPT_ARRAY:lept_stringify_array(val,str);break;
        case LEPT_OBJECT:lept_stringify_object(val,str);break;
    }
    return LEPT_STRINGIFY_OK;
}

int json_tree::lept_stringify(std::string& str)
{
    int ret;
    if((ret = lept_stringify_value(value,str))!=LEPT_STRINGIFY_OK){
        return ret;
    }
    assert(str.size()!=0);
    return ret;
}

void json_tree::set_null(){value.set_null();}
void json_tree::set_val(const json_value& val){value = val;}






/****************************************************************/
/*********************JSON VALUE::*****************************/
/***************************************************************/
void json_value:: set_null(){
    if(get_type()==LEPT_NULL) return;
    type = LEPT_NULL;
    switch(get_type()){
            case LEPT_STRING: s.~string();break;
            case LEPT_ARRAY: a.~vec_val_ptr();break;
            case LEPT_OBJECT:o.~map_object();break;
        }
}
void json_value::set(const double _d){set_null();type = LEPT_NUMBER;d = _d;}
void json_value::set(const int _d){set_null();set(double(_d));}
void json_value::set(const std::string& str){set_null();type = LEPT_STRING;new (&s) std::string(str);}
void json_value::set(const bool _b){set_null();type = _b?LEPT_TRUE:LEPT_FALSE;}
void json_value::set(const char s[] ){set_null();set(std::string(s));}
void json_value::set(const map_object& _o){set_null();type = LEPT_OBJECT;new (&o) map_object(_o);}
void json_value::set(const vec_val_ptr& _a){set_null();type = LEPT_ARRAY;new(&a) vec_val_ptr(_a);}


json_value& json_value::operator =(const json_value& rhs) {
        switch(rhs.get_type()){
            case LEPT_NULL:type=LEPT_NULL;break;
            case LEPT_FALSE:set(false);break;
            case LEPT_TRUE:set(true);break;
            case LEPT_NUMBER:set(rhs.get_number());break;
            case LEPT_STRING: set(rhs.get_str());break;
            case LEPT_ARRAY:set(rhs.get_array());break;
            case LEPT_OBJECT:set(rhs.get_object());break;
        }
        return*this;
    }

json_value::smart_value_ptr json_value::index_of(size_t index) const{
        assert(type==LEPT_ARRAY);
        assert(index < a.size());
        return a[index];
    }

  json_value::smart_value_ptr json_value::get_object_element(const std::string& key) const{
        auto index = key.find('.');
        if(index!=std::string::npos){
            auto root = key.substr(0,index);
            auto sub_path = key.substr(index+1);
            auto ret = o.find(root);
            assert(ret!=o.end());
            return ret->second->get_object_element(sub_path);
        }
        auto ret = o.find(key);
        assert(ret!=o.end());
        return ret->second;
    }

 json_value::smart_value_ptr json_value::create_object_element   (const std::string& key){
        auto index = key.find('.');
        if(index!=std::string::npos){
            auto root = key.substr(0,index);
            auto sub_path = key.substr(index+1);
            auto ret = o.find(root);
            if(ret==o.end()){
                auto new_node = std::make_shared<json_value>(map_object());

                o.insert({root,new_node});
                return new_node->create_object_element(sub_path);;
            }
            return ret->second->create_object_element(sub_path);
        }

        auto ret = o.find(key);
        if(ret==o.end()){
            auto new_node = std::make_shared<json_value>();
            o.insert({key,new_node});
            return new_node;
        }
        return ret->second;
    }

void json_value::add(const std::string& key,const json_value& val){
     switch(type){
        case LEPT_ARRAY:
        {
            a.push_back(std::make_shared<json_value>(val));
            break;
        }
        case LEPT_OBJECT:
             o.insert({key,std::make_shared<json_value>(val)});
             break;
        default:
          assert(std::string("only strucure can add element")=="");
      }
     }


bool test_json_array(const json_value& t1,const json_value& t2)
{
    auto N = t1.get_array().size();
        if(N==t2.get_array().size())
            for(size_t i = 0;i<N;++i)
               CHECK_CONDITION(*t1.index_of(i)==*t2.index_of(i));
        return true;
}

bool operator== (const json_value& t1,const json_value& t2){

    CHECK_CONDITION(t1.get_type() == t2.get_type());// IF size1!=size2,return false

    switch(t1.get_type())
    {
        case LEPT_ARRAY:return test_json_array(t1,t2);
        case LEPT_OBJECT:
        {
            auto o1 = t1.get_object();
            auto o2 = t2.get_object();
            CHECK_CONDITION(o1.size()==o2.size());// IF size1!=size2,return false

            std::unordered_multiset< std::string> unique_keys;
            for( auto& pair:o1)
            {
                std::string key = pair.first;
                if(unique_keys.find(key) ==unique_keys.end())
                  {
                    unique_keys.insert(key);
                    auto range1 = o1.equal_range(pair.first);
                    auto range2 = o2.equal_range(pair.first);
                    CHECK_CONDITION \
                        ( std::distance(range1.first,range1.second)==\
                           std::distance(range2.first,range2.second));//IF range1!=range2,return false

                    /****compare two group****/
                    std::unordered_set<json_value,json_hasher>  group1,group2;
                    INSERT_JSON_VALUE(range1.first,range1.second,group1);
                    INSERT_JSON_VALUE( range2.first,range2.second,group2);
                    CHECK_CONDITION(group1==group2);// if group1!group2, return false
                   }
            }
            return true;
        }
        case LEPT_NUMBER:
            return (t1.get_number()==t2.get_number());
        case LEPT_STRING:
            return (t1.get_str()==t2.get_str());
        default:
            return true;
    }
}




