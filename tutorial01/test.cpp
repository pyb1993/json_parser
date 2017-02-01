#include "iostream"
#include "leptjson.h"
static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

#define EXPECT_EQ_BASE(equality, expect, actual)             \
    do {                                                                                   \
        test_count++;                                                                \
        if (equality)                                                                     \
            test_pass++;                                                               \
        else {                                                                                \
            std::cout<<std::dec<<"expect: "<<expect<<"actual: "<<actual<<std::endl;\
            main_ret = 1;\
        }                                                                                           \
    } while(0)
inline void EXPECT_EQ_INT(int expect, int actual){EXPECT_EQ_BASE((expect) == (actual), expect, actual);}
inline void EXPECT_EQ_DOUBLE(double expect, double actual){EXPECT_EQ_BASE((expect) == (actual), expect, actual);}
inline void EXPECT_EQ_STRING(std::string expect, std::string actual) {EXPECT_EQ_BASE((expect==actual),expect,actual);}
inline void EXPECT_BOOL(bool expect,bool actual) {EXPECT_EQ_BASE((expect==actual),expect,actual);}

#define TEST_STRUCTURE_TYPE(t,json,expect_type) \
do {\
      t.set_null();\
      EXPECT_EQ_INT(LEPT_PARSE_OK,t.lept_parse(json));\
      EXPECT_EQ_INT(expect_type,t.lept_get_type());\
  }while(0)


#define TEST_BOOL(expect,expect_type,json)\
    do{\
        json_tree t;\
        t.value.type = LEPT_NULL;\
        EXPECT_EQ_INT(LEPT_PARSE_OK,t.lept_parse(json));\
        EXPECT_EQ_INT(expect_type,t.lept_get_type());\
        EXPECT_BOOL(expect,t.get<bool>());\
    }while(0)


#define TEST_NUMBER(expect,json)\
    do{\
        json_tree t;\
        t.value.type = LEPT_NULL;\
        EXPECT_EQ_INT(LEPT_PARSE_OK,t.lept_parse(json));\
        EXPECT_EQ_INT(LEPT_NUMBER,t.lept_get_type());\
        EXPECT_EQ_DOUBLE(expect,t.get<double>());\
    }while(0)

#define TEST_ERROR(error,json)\
    do{\
        json_tree t;\
        t.value.type = LEPT_FALSE;\
        EXPECT_EQ_INT(error, t.lept_parse(json));\
        EXPECT_EQ_INT(LEPT_NULL, t.lept_get_type());\
    }while(0)

#define TEST_STRING(expect, json)\
    do {\
        json_tree t;\
        EXPECT_EQ_INT(LEPT_PARSE_OK, t.lept_parse(json));\
        EXPECT_EQ_INT(LEPT_STRING, t.lept_get_type());\
        EXPECT_EQ_STRING(expect, t.get<std::string>());\
    } while(0)


static void test_parse_null() {
    std::cout<<"test null\n";
    json_tree t;
    t.set(false);
    EXPECT_EQ_INT(LEPT_PARSE_OK, t.lept_parse("null"));//test the null value
    EXPECT_EQ_INT(LEPT_NULL, t.lept_get_type());// test return type
}

static void test_parse_expect_value() {
    json_tree t;
    t.value.type = LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_PARSE_EXPECT_VALUE, t.lept_parse(""));//test empty json
    EXPECT_EQ_INT(LEPT_NULL, t.lept_get_type());

    t.value.type = LEPT_FALSE;
    EXPECT_EQ_INT(LEPT_PARSE_EXPECT_VALUE, t.lept_parse(" "));// test empty json
    EXPECT_EQ_INT(LEPT_NULL, t.lept_get_type());
}

static void test_parse_invalid_value() {
        std::cout<<"test invalid\n";

    TEST_ERROR(LEPT_PARSE_INVALID_VALUE,"NUL");
            #if 1
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE,"?");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE,".123");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE," inf");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE," 1.");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE," +1");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE," 012.3");
    #endif


    TEST_ERROR(LEPT_PARSE_INVALID_VALUE,"[[],[0 ], [0 , 1 ,2],12345x]");

}

static void test_parse_root_not_singular() {
    std::cout<<"test not singular\n";
    TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR,"null x");
    TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR," 123 x");

}

static void test_parse_false(){
     std::cout<<"test false\n";
     TEST_BOOL(false,LEPT_FALSE,"  false");
     TEST_BOOL(false,LEPT_FALSE," false ");
}
static void test_parse_true(){
     std::cout<<"test true\n";
     TEST_BOOL(true,LEPT_TRUE,"  true");
     TEST_BOOL(true,LEPT_TRUE,"true ");
}

static void test_parse_number(){
    std::cout<<"test number\n";
    TEST_NUMBER(0.0,"0");
    TEST_NUMBER(0.0,"-0");
    TEST_NUMBER(0.0,"-0.0");
    TEST_NUMBER(1.0,"1");
    TEST_NUMBER(-1.0,"-1");
    TEST_NUMBER(1.5,"1.5");
    TEST_NUMBER(-1.5,"-1.5");
    TEST_NUMBER(1E10, "1E10");
    TEST_NUMBER(1e10, "1e10");
    TEST_NUMBER(1E+10, "1E+10");
    TEST_NUMBER(1E-10, "1E-10");
    TEST_NUMBER(-1E10, "-1E10");
    TEST_NUMBER(-1e10, "-1e10");
    TEST_NUMBER(-1E+10, "-1E+10");
    TEST_NUMBER(-1E-10, "-1E-10");
    TEST_NUMBER(1.234E+10, "1.234E+10");
    TEST_NUMBER(1.234E-10, "1.234E-10");
    TEST_NUMBER(0.0, "1e-10000"); /* must underflow */
    TEST_NUMBER(1.0000000000000002, "1.0000000000000002"); /* the smallest number > 1 */
    TEST_NUMBER( 1.7976931348623157e+308, "1.7976931348623157e+308");//max
     TEST_NUMBER( 4.9406564584124654e-324, "4.9406564584124654e-324");//min

}


static void test_parse_string() {
    std::cout<<"test string\n";

    TEST_STRING("", "\"\"");
        #if 1
    TEST_STRING("Hello", "\"Hello\"");
    TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
    TEST_STRING("\" \\ / ", "\"\\\" \\\\ \\/ \"");
    TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
    #endif
    #if 1
   TEST_STRING("Hello\0World", "\"Hello\\u0000World\"");
   TEST_STRING("\x24", "\"\\u0024\"");         /* Dollar sign U+0024 */
    TEST_STRING("\xC2\xA2", "\"\\u00A2\"");     /* Cents sign U+00A2 */
       #endif
        #if 1
    TEST_STRING("\xE2\x82\xAC", "\"\\u20AC\""); /* Euro sign U+20AC */
    TEST_STRING("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\"");  /* G clef sign U+1D11E */
    TEST_STRING("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\"");  /* G clef sign U+1D11E */
     #endif
}

static void test_parse_array(){
    std::cout<<"test array\n";
    json_tree t;
#if 1
    TEST_STRUCTURE_TYPE(t,"[null ,false  ,true,123,\"abcd\"]",LEPT_ARRAY);
    EXPECT_BOOL(false,              t.get<bool>(1));

    EXPECT_BOOL(true,               t.get<bool>(2));
    EXPECT_EQ_DOUBLE(123,     t.get<double>(3));
    EXPECT_EQ_STRING("abcd",  t.get<std::string>(4));
    #endif
    t.set_null();
#if 1
    TEST_STRUCTURE_TYPE(t,"[[],[0 ], [0 , 1 ,2],12345]",LEPT_ARRAY);
    EXPECT_EQ_DOUBLE(0,             t.get<double>({1,0}) );
    EXPECT_EQ_DOUBLE(2,             t.get<double>({2,2}));
    EXPECT_EQ_DOUBLE(12345,     t.get<double>(3));
#endif
}

static void test_parse_object(){
    std::cout<<"test object\n";
    json_tree t;
    #if 1
    TEST_STRUCTURE_TYPE(t,"{\"1\":123 ,\"fuck\" :  true,\"fuck2\" : \"2017.1.28\" }",LEPT_OBJECT);

    EXPECT_EQ_DOUBLE(123,(t.get<double>("1")));
    EXPECT_BOOL(true,(t.get<bool>("fuck")));
    EXPECT_EQ_STRING("2017.1.28",(t.get<std::string>("fuck2")));
    #endif

    #if 1
    auto s = "{\"n\":null,   \"f\" : false,  \"t\" :true ,\"i\"  : 123 ,\"s\" :  \"abc\",   \"a\" :[1,2,3],  \"o\" :{\"1\" : 1, \"2\" : 2, \"3\" : {\"fuck\":5} }}";

    TEST_STRUCTURE_TYPE(t,s,LEPT_OBJECT);
    EXPECT_BOOL(false,(t.get<bool>("f")));
    EXPECT_EQ_DOUBLE(123,(t.get<double>("i")));
    EXPECT_EQ_DOUBLE(2,(*t.get<json_value::vec_val_ptr>("a")[1]));
    EXPECT_EQ_DOUBLE(2,t.get<double>("o.2"));
    EXPECT_EQ_DOUBLE(5,t.get<double>("o.3.fuck"));
    #endif
}

static void test_parse_too_bignumber(){
    std::cout<<"test too big number\n";
    TEST_ERROR( LEPT_PARSE_NUMBER_TOO_BIG, "1.7976931348623157e+1308");//max
     TEST_ERROR( LEPT_PARSE_NUMBER_TOO_BIG, "-4.9406564584124654e+1324");//min
}


static void test_parse_missing_quotation_mark() {
    TEST_ERROR(LEPT_PARSE_MISS_QUOTATION_MARK, "\"");
    TEST_ERROR(LEPT_PARSE_MISS_QUOTATION_MARK, "\"abc");
}

static void test_parse_invalid_string_escape() {
#if 1
    TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
    TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
    TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
    TEST_ERROR(LEPT_PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
#endif
}

static void test_parse_invalid_string_char() {
#if 1
    TEST_ERROR(LEPT_PARSE_INVALID_STRING_CHAR, "\"\x01\"");
    TEST_ERROR(LEPT_PARSE_INVALID_STRING_CHAR, "\"\x1F\"");
#endif
}

static void test_parse_invalid_unicode_hex() {
    std::cout<<"test invalid unicode hex\n";
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u0\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u01\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u012\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u/000\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\uG000\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u0G00\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u00G0\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u000/\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u000G\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX, "\"\\u 123\"");
}

static void test_parse_invalid_unicode_surrogate() {
        std::cout<<"test invalid unicode surrgoate\n";

    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uDBFF\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\\\\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uDBFF\"");
    TEST_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"");
}

static void test_parse_miss_comma_or_square_bracket() {
    TEST_ERROR(LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1");
    TEST_ERROR(LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1}");
    TEST_ERROR(LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1 2");
    TEST_ERROR(LEPT_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[[]");
}

static void test_parse_miss_colon() {
    TEST_ERROR(LEPT_PARSE_MISS_COLON, "{\"a\"}");
    TEST_ERROR(LEPT_PARSE_MISS_COLON, "{\"a\",\"b\"}");
}

static void test_parse_miss_comma_or_curly_bracket() {
    TEST_ERROR(LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1");
    TEST_ERROR(LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1]");
    TEST_ERROR(LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1 \"b\"");
    TEST_ERROR(LEPT_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":{}");
}


static void test_access_null(){
    json_tree t;
    t.set_null();
    EXPECT_EQ_INT(LEPT_NULL, t.lept_get_type());
}

static void test_access_boolean() {
    std::cout<<"access boolean\n";
    json_tree t;
    t.set(false);
    EXPECT_BOOL(false,t.get<bool>());
    t.set(true);
    EXPECT_BOOL(true,t.get<bool>());
}

static void test_access_number() {
    std::cout<<"access number\n";

    json_tree t;
    t.set(323.3);
    EXPECT_EQ_DOUBLE(323.3, t.get<double>());

}

static void test_access_string() {
    std::cout<<"access string\n";
    json_tree t;
    t.set("abcd");
    EXPECT_EQ_STRING("abcd",t.get<std::string>());

}

static void test_set_any(){

    std::cout<<"test set any\n";
    json_tree t;
     TEST_STRUCTURE_TYPE(t,"{\"1\":2,\"2\":[3,4,5],\"3\":{\"a\": \"abcd\"}}",LEPT_OBJECT);
    t.set("1",3);
    EXPECT_EQ_DOUBLE(3, t.get<double>("1"));
    t.set("3.a","fuck");
    EXPECT_EQ_STRING("fuck", t.get<std::string>("3.a"));
    t.set("4.num",400);
    EXPECT_EQ_DOUBLE(400, t.get<double>("4.num"));
}

static void test_add_child(){
    std::cout<<"test add child\n";
    json_tree t,child1,child2;
    t.lept_parse("{\"a:\":1}");

    child1.lept_parse("5");
    t.add_child("b",child1);
    EXPECT_EQ_DOUBLE(5, t.get<double>("b"));

    child2.lept_parse("\"child\"");
    t.add_child("c.d",child2);
    EXPECT_EQ_STRING("child", t.get<std::string>("c.d"));
}

static void test_append_child(){
    std::cout<<"test append child\n";
    json_tree t,child1,child2,child3;

    t.lept_parse("{\"a:\":1,\"1\":[1,2,3]}");

    child1.lept_parse("5");
    t.append_child("b",child1);
    EXPECT_EQ_DOUBLE(5, *t.get<json_value::vec_val_ptr>("b")[0]);


    child2.lept_parse("\"child\"");
    t.append_child("c.d",child2);
    EXPECT_EQ_STRING("child", *t.get<json_value::vec_val_ptr>("c.d")[0]);


    child3.lept_parse("\"child\"");
     t.append_child("1",child3);

    EXPECT_EQ_STRING("child", *t.get<json_value::vec_val_ptr>("1")[3]);

}


#define TEST_ROUNDTRIP(json)\
do{\
    json_tree t;\
    std::string result;\
    EXPECT_EQ_INT(LEPT_PARSE_OK,t.lept_parse(json));\
    EXPECT_EQ_INT(LEPT_STRINGIFY_OK,t.lept_stringify(result));\
    EXPECT_EQ_STRING(json,result);\
}while(0)

#define TEST_TREE(json)\
do{\
     json_tree t;\
     json_tree other;\
     std::string result;\
     EXPECT_EQ_INT(LEPT_PARSE_OK,t.lept_parse(json));\
     EXPECT_EQ_INT(LEPT_STRINGIFY_OK,t.lept_stringify(result));\
     EXPECT_EQ_INT(LEPT_PARSE_OK,other.lept_parse(json));\
     EXPECT_BOOL(t.get_value()==other.get_value(),true);\
}while(0)


static void test_stringify(){
    std::cout<<"test stringify\n";

    TEST_TREE("null");
        #if 1
    TEST_TREE("true");
    TEST_TREE("false");
    TEST_TREE("1234.567000");// notice the accuracy
    TEST_TREE("\"abcd\"");
    TEST_TREE("\"abcd\\n10086\\tfuck\"");
    TEST_TREE("\"abcd\\\"n10086\\tfuck\"");


    TEST_TREE("[1.000000,  \"abcd\",[2.000000,false]]");


    TEST_TREE("{    }");
    TEST_TREE("{\"1\":   1}");
    TEST_TREE("{\"H\":1.000000,\"F\":\"abcd\",\"W\":[2.000000,false]}");
    TEST_TREE("{\"H\":1.000000,\"F\":\"abcd\",\"W\":[2.000000,false,{}]}");
    TEST_TREE("{\"n\":null,\"f\":false,\"t\":true,\"i\":123,\"s\":\"abc\",\"a\":[1,2,3],\"o\":{\"1\":1,\"2\":2,\"3\":3}}");
        #endif
    //TEST_TREE("\"Hello\\u0005World\"");
}


static void test_parse(){

#if 1
    test_parse_null();
    test_parse_expect_value();
    test_parse_false();
    test_parse_true();
    test_parse_number();
    test_parse_string();
    #endif


#if 1
    test_parse_array();
    test_parse_object();


    #endif

#if 1
     test_parse_too_bignumber();
    test_parse_root_not_singular();
     test_parse_invalid_value();
    test_parse_missing_quotation_mark();
    test_parse_invalid_string_char();

    test_parse_invalid_string_escape();
    test_parse_invalid_unicode_hex();
    test_parse_miss_comma_or_square_bracket();
    test_parse_miss_colon();
    test_parse_miss_comma_or_curly_bracket();
    #endif

        #if 1
    test_parse_invalid_unicode_surrogate();
    test_access_string();
    test_access_null();
    test_access_number();
    test_access_boolean();
    test_set_any();
    test_add_child();
    test_append_child();
    #endif

    #if 1
    test_stringify();
    #endif
}

int main() {

    test_parse();
    std::cout<<std::dec<<test_pass<<"/"<<test_count<<"("<<(test_pass*100)/test_count<<")  passed"<<"\n";
    return main_ret;
}
