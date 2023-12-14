#include "test_controller.h"
#include <server/helper/helper.h>

/**
 * @brief 空请求，用于压力测试.
 *
 * @route  /api/Test/TestEmpty
 * @method GET
 * @config Authorization(0)
 */
void TestController::TestEmpty(Request& req, Response& res) {
    res.SetStringBody("Ok", "text/plain");
}

/************************************* CHECK_PARAM_xxx ********************************************/

/**
 * @brief 测试宏-CHECK_URL_PARAM_xxx.
 * 
 * @route  /api/Test/TestMacro_CHECK_URL_PARAM
 * @method GET
 */
void TestController::TestMacro_CHECK_URL_PARAM(Request& req, Response& res) {
    API_INIT();
    /* 获取URL中的参数 */
    CHECK_URL_PARAM_STR(str1, str2);   /* 括号内可以写多个参数 */
    CHECK_URL_PARAM_STR_LOWER(str_lower);
    CHECK_URL_PARAM_STR_UPPER(str_upper);
    CHECK_URL_PARAM_BOOL(bool_val);
    CHECK_URL_PARAM_INT(int_val);
    CHECK_URL_PARAM_UINT(uint_val);
    CHECK_URL_PARAM_INT64(int64_val);
    CHECK_URL_PARAM_UINT64(uint64_val);
    CHECK_URL_PARAM_DOUBLE(double_val);

    data["str1"] = str1;
    data["str2"] = str2;
    data["str_lower"] = str_lower;
    data["str_upper"] = str_upper;
    data["bool_val"] = bool_val;
    data["int_val"] = int_val;
    data["uint_val"] = uint_val;
    data["int64_val"] = int64_val;
    data["uint64_val"] = uint64_val;
    data["double_val"] = double_val;
    RETURN_OK();
}

/**
 * @brief 测试宏-CHECK_BODY_PARAM_xxx.
 * 
 * @route  /api/Test/TestMacro_CHECK_BODY_PARAM
 * @method POST
 */
void TestController::TestMacro_CHECK_BODY_PARAM(Request& req, Response& res) {
    API_INIT();
    /* 获取Content-Type: application/x-www-form-urlencoded请求体的参数 */
    CHECK_BODY_PARAM_STR(str1, str2);    /* 括号内可以写多个参数 */
    CHECK_BODY_PARAM_STR_LOWER(str_lower);
    CHECK_BODY_PARAM_STR_UPPER(str_upper);
    CHECK_BODY_PARAM_BOOL(bool_val);
    CHECK_BODY_PARAM_INT(int_val);
    CHECK_BODY_PARAM_UINT(uint_val);
    CHECK_BODY_PARAM_INT64(int64_val);
    CHECK_BODY_PARAM_UINT64(uint64_val);
    CHECK_BODY_PARAM_DOUBLE(double_val);

    data["str1"] = str1;
    data["str2"] = str2;
    data["str_lower"] = str_lower;
    data["str_upper"] = str_upper;
    data["bool_val"] = bool_val;
    data["int_val"] = int_val;
    data["uint_val"] = uint_val;
    data["int64_val"] = int64_val;
    data["uint64_val"] = uint64_val;
    data["double_val"] = double_val;
    RETURN_OK();
}

/**
 * @brief 测试宏-CHECK_JSON_PARAM_xxx.
 * 
 * @route  /api/Test/TestMacro_CHECK_JSON_PARAM
 * @method POST
 */
void TestController::TestMacro_CHECK_JSON_PARAM(Request& req, Response& res) {
    API_INIT();
    /* 获取Content-Type: application/json请求体的参数 */
    CHECK_JSON_PARAM_STR(str1, str2);    /* 括号内可以写多个参数 */
    CHECK_JSON_PARAM_STR_LOWER(str_lower);
    CHECK_JSON_PARAM_STR_UPPER(str_upper);
    CHECK_JSON_PARAM_BOOL(bool_val);
    CHECK_JSON_PARAM_INT(int_val);
    CHECK_JSON_PARAM_UINT(uint_val);
    CHECK_JSON_PARAM_INT64(int64_val);
    CHECK_JSON_PARAM_UINT64(uint64_val);
    CHECK_JSON_PARAM_DOUBLE(double_val);

    data["str1"] = str1;
    data["str2"] = str2;
    data["str_lower"] = str_lower;
    data["str_upper"] = str_upper;
    data["bool_val"] = bool_val;
    data["int_val"] = int_val;
    data["uint_val"] = uint_val;
    data["int64_val"] = int64_val;
    data["uint64_val"] = uint64_val;
    data["double_val"] = double_val;
    RETURN_OK();
}

template<typename T>
static Json::Value to_json(const std::vector<T>& vec) {
    Json::Value node;
    for (auto& v : vec) {
        node.append(v);
    }
    return node;
}

/**
 * @brief 测试宏-CHECK_JSON_PARAM_xxx_ARRAY.
 * 
 * @route  /api/Test/TestMacro_CHECK_JSON_PARAM_ARRAY
 * @method POST
 */
void TestController::TestMacro_CHECK_JSON_PARAM_ARRAY(Request& req, Response& res) {
    API_INIT();
    /* 获取Content-Type: application/json请求体的参数(数组) */
    CHECK_JSON_PARAM_STR_ARRAY(str1, str2);    /* 括号内可以写多个参数 */
    CHECK_JSON_PARAM_STR_LOWER_ARRAY(str_lower);
    CHECK_JSON_PARAM_STR_UPPER_ARRAY(str_upper);
    CHECK_JSON_PARAM_BOOL_ARRAY(bool_val);
    CHECK_JSON_PARAM_INT_ARRAY(int_val);
    CHECK_JSON_PARAM_UINT_ARRAY(uint_val);
    CHECK_JSON_PARAM_INT64_ARRAY(int64_val);
    CHECK_JSON_PARAM_UINT64_ARRAY(uint64_val);
    CHECK_JSON_PARAM_DOUBLE_ARRAY(double_val);

    data["str1"] = to_json(str1);
    data["str2"] = to_json(str2);
    data["str_lower"] = to_json(str_lower);
    data["str_upper"] = to_json(str_upper);
    data["bool_val"] = to_json(bool_val);
    data["int_val"] = to_json(int_val);
    data["uint_val"] = to_json(uint_val);
    data["int64_val"] = to_json(int64_val);
    data["uint64_val"] = to_json(uint64_val);
    data["double_val"] = to_json(double_val);
    RETURN_OK();
}

/**
 * @brief 测试宏-CHECK_BODY_PARAM_xxx_EX.
 * 
 * @route  /api/Test/TestMacro_CHECK_BODY_PARAM_EX
 * @method POST
 */
void TestController::TestMacro_CHECK_BODY_PARAM_EX(Request& req, Response& res) {
    API_INIT();
    /**
     * 同时支持
     * (1) Content-Type: application/json
     * (2) Content-Type: application/x-www-form-urlencoded
     */
    CHECK_BODY_PARAM_STR_EX(str1, str2);    /* 括号内可以写多个参数 */
    CHECK_BODY_PARAM_STR_LOWER_EX(str_lower);
    CHECK_BODY_PARAM_STR_UPPER_EX(str_upper);
    CHECK_BODY_PARAM_BOOL_EX(bool_val);
    CHECK_BODY_PARAM_INT_EX(int_val);
    CHECK_BODY_PARAM_UINT_EX(uint_val);
    CHECK_BODY_PARAM_INT64_EX(int64_val);
    CHECK_BODY_PARAM_UINT64_EX(uint64_val);
    CHECK_BODY_PARAM_DOUBLE_EX(double_val);

    data["str1"] = str1;
    data["str2"] = str2;
    data["str_lower"] = str_lower;
    data["str_upper"] = str_upper;
    data["bool_val"] = bool_val;
    data["int_val"] = int_val;
    data["uint_val"] = uint_val;
    data["int64_val"] = int64_val;
    data["uint64_val"] = uint64_val;
    data["double_val"] = double_val;
    RETURN_OK();
}

/*************************************** GET_PARAM_xxx ********************************************/

/**
 * @brief 测试宏-GET_URL_PARAM_xxx.
 * 
 * @route  /api/Test/TestMacro_GET_URL_PARAM
 * @method GET
 */
void TestController::TestMacro_GET_URL_PARAM(Request& req, Response& res) {
    API_INIT();
    /* 获取URL中的参数 */
    GET_URL_PARAM_STR("default_str", str1, str2);   /* 括号内可以写多个参数 */
    GET_URL_PARAM_STR_LOWER("default_str_lower", str_lower);
    GET_URL_PARAM_STR_UPPER("DEFAULT_STR_UPPER", str_upper);
    GET_URL_PARAM_BOOL(true, bool_val);
    GET_URL_PARAM_INT(-111, int_val);
    GET_URL_PARAM_UINT(222, uint_val);
    GET_URL_PARAM_INT64(-333, int64_val);
    GET_URL_PARAM_UINT64(444, uint64_val);
    GET_URL_PARAM_DOUBLE(555.233, double_val);

    data["str1"] = str1;
    data["str2"] = str2;
    data["str_lower"] = str_lower;
    data["str_upper"] = str_upper;
    data["bool_val"] = bool_val;
    data["int_val"] = int_val;
    data["uint_val"] = uint_val;
    data["int64_val"] = int64_val;
    data["uint64_val"] = uint64_val;
    data["double_val"] = double_val;
    RETURN_OK();
}

/**
 * @brief 测试宏-GET_BODY_PARAM_xxx.
 * 
 * @route  /api/Test/TestMacro_GET_BODY_PARAM
 * @method POST
 */
void TestController::TestMacro_GET_BODY_PARAM(Request& req, Response& res) {
    API_INIT();
    /* 获取Content-Type: application/x-www-form-urlencoded请求体的参数 */
    GET_BODY_PARAM_STR("default_str", str1, str2);   /* 括号内可以写多个参数 */
    GET_BODY_PARAM_STR_LOWER("default_str_lower", str_lower);
    GET_BODY_PARAM_STR_UPPER("DEFAULT_STR_UPPER", str_upper);
    GET_BODY_PARAM_BOOL(true, bool_val);
    GET_BODY_PARAM_INT(-111, int_val);
    GET_BODY_PARAM_UINT(222, uint_val);
    GET_BODY_PARAM_INT64(-333, int64_val);
    GET_BODY_PARAM_UINT64(444, uint64_val);
    GET_BODY_PARAM_DOUBLE(555.233, double_val);

    data["str1"] = str1;
    data["str2"] = str2;
    data["str_lower"] = str_lower;
    data["str_upper"] = str_upper;
    data["bool_val"] = bool_val;
    data["int_val"] = int_val;
    data["uint_val"] = uint_val;
    data["int64_val"] = int64_val;
    data["uint64_val"] = uint64_val;
    data["double_val"] = double_val;
    RETURN_OK();
}

/**
 * @brief 测试宏-GET_JSON_PARAM_xxx.
 * 
 * @route  /api/Test/TestMacro_GET_JSON_PARAM
 * @method POST
 */
void TestController::TestMacro_GET_JSON_PARAM(Request& req, Response& res) {
    API_INIT();
    /* 获取Content-Type: application/json请求体的参数 */
    GET_JSON_PARAM_STR("default_str", str1, str2);   /* 括号内可以写多个参数 */
    GET_JSON_PARAM_STR_LOWER("default_str_lower", str_lower);
    GET_JSON_PARAM_STR_UPPER("DEFAULT_STR_UPPER", str_upper);
    GET_JSON_PARAM_BOOL(true, bool_val);
    GET_JSON_PARAM_INT(-111, int_val);
    GET_JSON_PARAM_UINT(222, uint_val);
    GET_JSON_PARAM_INT64(-333, int64_val);
    GET_JSON_PARAM_UINT64(444, uint64_val);
    GET_JSON_PARAM_DOUBLE(555.233, double_val);

    data["str1"] = str1;
    data["str2"] = str2;
    data["str_lower"] = str_lower;
    data["str_upper"] = str_upper;
    data["bool_val"] = bool_val;
    data["int_val"] = int_val;
    data["uint_val"] = uint_val;
    data["int64_val"] = int64_val;
    data["uint64_val"] = uint64_val;
    data["double_val"] = double_val;
    RETURN_OK();
}

/**
 * @brief 测试宏-GET_JSON_PARAM_xxx_ARRAY.
 * 
 * @route  /api/Test/TestMacro_GET_JSON_PARAM_ARRAY
 * @method POST
 */
void TestController::TestMacro_GET_JSON_PARAM_ARRAY(Request& req, Response& res) {
    API_INIT();
    /* 获取Content-Type: application/json请求体的参数(数组) */
    GET_JSON_PARAM_STR_ARRAY(std::vector<std::string>({"default_strA", "default_strB"}), str1, str2);    /* 括号内可以写多个参数 */
    GET_JSON_PARAM_STR_LOWER_ARRAY(std::vector<std::string>({ "default_stra", "default_strb" }), str_lower);
    GET_JSON_PARAM_STR_UPPER_ARRAY(std::vector<std::string>({ "DEFAULT_STRA", "DEFAULT_STRB" }), str_upper);
    GET_JSON_PARAM_BOOL_ARRAY(std::vector<int>({ 1, 0, 1 }), bool_val);
    GET_JSON_PARAM_INT_ARRAY(std::vector<int>({ -111, -112, -113 }), int_val);
    GET_JSON_PARAM_UINT_ARRAY(std::vector<uint32_t>({ 222, 223, 224 }), uint_val);
    GET_JSON_PARAM_INT64_ARRAY(std::vector<int64_t>({ -333, -334, -335 }), int64_val);
    GET_JSON_PARAM_UINT64_ARRAY(std::vector<uint64_t>({ 444, 445, 446 }), uint64_val);
    GET_JSON_PARAM_DOUBLE_ARRAY(std::vector<double>({ 555.233, 556.233, 557.233 }), double_val);

    data["str1"] = to_json(str1);
    data["str2"] = to_json(str2);
    data["str_lower"] = to_json(str_lower);
    data["str_upper"] = to_json(str_upper);
    data["bool_val"] = to_json(bool_val);
    data["int_val"] = to_json(int_val);
    data["uint_val"] = to_json(uint_val);
    data["int64_val"] = to_json(int64_val);
    data["uint64_val"] = to_json(uint64_val);
    data["double_val"] = to_json(double_val);
    RETURN_OK();
}

/**
 * @brief 测试宏-GET_BODY_PARAM_xxx_EX.
 * 
 * @route  /api/Test/TestMacro_GET_BODY_PARAM_EX
 * @method POST
 */
void TestController::TestMacro_GET_BODY_PARAM_EX(Request& req, Response& res) {
    API_INIT();
    /**
     * 同时支持
     * (1) Content-Type: application/json
     * (2) Content-Type: application/x-www-form-urlencoded
     */
    GET_BODY_PARAM_STR_EX("default_str", str1, str2);   /* 括号内可以写多个参数 */
    GET_BODY_PARAM_STR_LOWER_EX("default_str_lower", str_lower);
    GET_BODY_PARAM_STR_UPPER_EX("DEFAULT_STR_UPPER", str_upper);
    GET_BODY_PARAM_BOOL_EX(true, bool_val);
    GET_BODY_PARAM_INT_EX(-111, int_val);
    GET_BODY_PARAM_UINT_EX(222, uint_val);
    GET_BODY_PARAM_INT64_EX(-333, int64_val);
    GET_BODY_PARAM_UINT64_EX(444, uint64_val);
    GET_BODY_PARAM_DOUBLE_EX(555.233, double_val);

    data["str1"] = str1;
    data["str2"] = str2;
    data["str_lower"] = str_lower;
    data["str_upper"] = str_upper;
    data["bool_val"] = bool_val;
    data["int_val"] = int_val;
    data["uint_val"] = uint_val;
    data["int64_val"] = int64_val;
    data["uint64_val"] = uint64_val;
    data["double_val"] = double_val;
    RETURN_OK();
}
