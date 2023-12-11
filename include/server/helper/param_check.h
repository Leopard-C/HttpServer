#ifndef IC_SERVER_HELPER_PARAM_CHECK_H_
#define IC_SERVER_HELPER_PARAM_CHECK_H_
#include "_narg.h"
#include "../request.h"
#include "../response.h"

/***************************************************************************************************
 *
 *    检查并获取 URL、body 中的参数，必须存在.
 * 
 *       + CHECK_URL_PARAM_STR(...)          // URL
 *       + CHECK_URL_PARAM_STR_UPPER(...)
 *       + CHECK_URL_PARAM_STR_LOWER(...)
 *       + CHECK_URL_PARAM_BOOL(...)
 *       + CHECK_URL_PARAM_INT(...)
 *       + CHECK_URL_PARAM_UINT(...)
 *       + CHECK_URL_PARAM_INT64(...)
 *       + CHECK_URL_PARAM_UINT64(...)
 *       + CHECK_URL_PARAM_DOUBLE(...)
 * 
 *       + CHECK_BODY_PARAM_STR(...)        // (1)application/x-www-form-urlencoded
 *       + ...
 * 
 *       + CHECK_JSON_PARAM_STR(...)        // (2)application/json
 *       + CHECK_JSON_PARAM_STR_ARRAY(...)
 *       + ...
 * 
 *       + CHECK_BODY_PARAM_STR_EX(...)     // 自动选择(1)或(2)，即接口同时支持这两种类型的请求
 *       + ...
 * 
 *    使用示例1:
 *       某次GET请求 /article/getContent?id=100&from=web&token=xxx
 *       可以在控制器中使用如下方式检查并获取参数 id, from, token 的值：
 *           void ArticleController::GetContent(Request& req, Response& res) {
 *               API_INIT();
 *               CHECK_URL_PARAM_INT(id);
 *               CHECK_URL_PARAM_STR(from, token);
 *               // ...
 *               RETURN_OK();
 *           }
 * 
 *    使用示例2:
 *       某次POST请求 /article/updateContent, 请求体为JSON格式 { "id": 100, "update_by": "admin", "content": "xxx" }
 *       可以在控制器中使用如下方式检查并获取参数 id, update_by, content 的值：
 *           void ArticleController::UpdateContent(Request& req, Response& res) {
 *               API_INIT();
 *               CHECK_JSON_PARAM_INT(id);
 *               CHECK_JSON_PARAM_STR(update_by, content);
 *               // ...
 *               RETURN_OK();
 *           }
 *
***************************************************************************************************/

#define CHECK_URL_PARAM_STR(...)         __IC_MACRO_VFUNC(__CHECK_PARAM_TYPE, __check_url_param_str, std::string, __VA_ARGS__)
#define CHECK_BODY_PARAM_STR(...)        __IC_MACRO_VFUNC(__CHECK_PARAM_TYPE, __check_body_param_str, std::string, __VA_ARGS__)
#define CHECK_JSON_PARAM_STR(...)        __IC_MACRO_VFUNC(__CHECK_PARAM_TYPE, __check_json_param_str, std::string, __VA_ARGS__)
#define CHECK_JSON_PARAM_STR_ARRAY(...)  __IC_MACRO_VFUNC(__CHECK_PARAM_TYPE, __check_json_param_str_array, std::vector<std::string>, __VA_ARGS__)
#define CHECK_BODY_PARAM_STR_EX(...)     __IC_MACRO_VFUNC(__CHECK_PARAM_TYPE, __check_body_param_str_ex, std::string, __VA_ARGS__)

#define CHECK_URL_PARAM_STR_LOWER(...)         __IC_MACRO_VFUNC(__CHECK_PARAM_TYPE, __check_url_param_str_lower, std::string, __VA_ARGS__)
#define CHECK_URL_PARAM_STR_UPPER(...)         __IC_MACRO_VFUNC(__CHECK_PARAM_TYPE, __check_url_param_str_upper, std::string, __VA_ARGS__)
#define CHECK_BODY_PARAM_STR_LOWER(...)        __IC_MACRO_VFUNC(__CHECK_PARAM_TYPE, __check_body_param_str_lower, std::string, __VA_ARGS__)
#define CHECK_BODY_PARAM_STR_UPPER(...)        __IC_MACRO_VFUNC(__CHECK_PARAM_TYPE, __check_body_param_str_upper, std::string, __VA_ARGS__)
#define CHECK_JSON_PARAM_STR_LOWER(...)        __IC_MACRO_VFUNC(__CHECK_PARAM_TYPE, __check_json_param_str_lower, std::string, __VA_ARGS__)
#define CHECK_JSON_PARAM_STR_UPPER(...)        __IC_MACRO_VFUNC(__CHECK_PARAM_TYPE, __check_json_param_str_upper, std::string, __VA_ARGS__)
#define CHECK_JSON_PARAM_STR_LOWER_ARRAY(...)  __IC_MACRO_VFUNC(__CHECK_PARAM_TYPE, __check_json_param_str_lower_array, std::vector<std::string>, __VA_ARGS__)
#define CHECK_JSON_PARAM_STR_UPPER_ARRAY(...)  __IC_MACRO_VFUNC(__CHECK_PARAM_TYPE, __check_json_param_str_upper_array, std::vector<std::string>, __VA_ARGS__)
#define CHECK_BODY_PARAM_STR_LOWER_EX(...)     __IC_MACRO_VFUNC(__CHECK_PARAM_TYPE, __check_body_param_str_lower_ex, std::string, __VA_ARGS__)
#define CHECK_BODY_PARAM_STR_UPPER_EX(...)     __IC_MACRO_VFUNC(__CHECK_PARAM_TYPE, __check_body_param_str_upper_ex, std::string, __VA_ARGS__)

#define CHECK_URL_PARAM_BOOL(...)         __IC_MACRO_VFUNC(__CHECK_PARAM_TYPE, __check_url_param_bool, bool, __VA_ARGS__)
#define CHECK_BODY_PARAM_BOOL(...)        __IC_MACRO_VFUNC(__CHECK_PARAM_TYPE, __check_body_param_bool, bool, __VA_ARGS__)
#define CHECK_JSON_PARAM_BOOL(...)        __IC_MACRO_VFUNC(__CHECK_PARAM_TYPE, __check_json_param_bool, bool, __VA_ARGS__)
#define CHECK_JSON_PARAM_BOOL_ARRAY(...)  __IC_MACRO_VFUNC(__CHECK_PARAM_TYPE, __check_json_param_bool_array, std::vector<int32_t>, __VA_ARGS__)
#define CHECK_BODY_PARAM_BOOL_EX(...)     __IC_MACRO_VFUNC(__CHECK_PARAM_TYPE, __check_body_param_bool_ex, bool, __VA_ARGS__)

#define CHECK_URL_PARAM_INT(...)         __IC_MACRO_VFUNC(__CHECK_PARAM_TYPE, __check_url_param_int, int32_t, __VA_ARGS__)
#define CHECK_BODY_PARAM_INT(...)        __IC_MACRO_VFUNC(__CHECK_PARAM_TYPE, __check_body_param_int, int32_t, __VA_ARGS__)
#define CHECK_JSON_PARAM_INT(...)        __IC_MACRO_VFUNC(__CHECK_PARAM_TYPE, __check_json_param_int, int32_t, __VA_ARGS__)
#define CHECK_JSON_PARAM_INT_ARRAY(...)  __IC_MACRO_VFUNC(__CHECK_PARAM_TYPE, __check_json_param_int_array, std::vector<int32_t>, __VA_ARGS__)
#define CHECK_BODY_PARAM_INT_EX(...)     __IC_MACRO_VFUNC(__CHECK_PARAM_TYPE, __check_body_param_int_ex, int32_t, __VA_ARGS__)

#define CHECK_URL_PARAM_UINT(...)         __IC_MACRO_VFUNC(__CHECK_PARAM_TYPE, __check_url_param_uint, uint32_t, __VA_ARGS__)
#define CHECK_BODY_PARAM_UINT(...)        __IC_MACRO_VFUNC(__CHECK_PARAM_TYPE, __check_body_param_uint, uint32_t, __VA_ARGS__)
#define CHECK_JSON_PARAM_UINT(...)        __IC_MACRO_VFUNC(__CHECK_PARAM_TYPE, __check_json_param_uint, uint32_t, __VA_ARGS__)
#define CHECK_JSON_PARAM_UINT_ARRAY(...)  __IC_MACRO_VFUNC(__CHECK_PARAM_TYPE, __check_json_param_uint_array, std::vector<uint32_t>, __VA_ARGS__)
#define CHECK_BODY_PARAM_UINT_EX(...)     __IC_MACRO_VFUNC(__CHECK_PARAM_TYPE, __check_body_param_uint_ex, uint32_t, __VA_ARGS__)

#define CHECK_URL_PARAM_INT64(...)         __IC_MACRO_VFUNC(__CHECK_PARAM_TYPE, __check_url_param_int64, int64_t, __VA_ARGS__)
#define CHECK_BODY_PARAM_INT64(...)        __IC_MACRO_VFUNC(__CHECK_PARAM_TYPE, __check_body_param_int64, int64_t, __VA_ARGS__)
#define CHECK_JSON_PARAM_INT64(...)        __IC_MACRO_VFUNC(__CHECK_PARAM_TYPE, __check_json_param_int64, int64_t, __VA_ARGS__)
#define CHECK_JSON_PARAM_INT64_ARRAY(...)  __IC_MACRO_VFUNC(__CHECK_PARAM_TYPE, __check_json_param_int64_array, std::vector<int64_t>, __VA_ARGS__)
#define CHECK_BODY_PARAM_INT64_EX(...)     __IC_MACRO_VFUNC(__CHECK_PARAM_TYPE, __check_body_param_int64_ex, int64_t, __VA_ARGS__)

#define CHECK_URL_PARAM_UINT64(...)         __IC_MACRO_VFUNC(__CHECK_PARAM_TYPE, __check_url_param_uint64, uint64_t, __VA_ARGS__)
#define CHECK_BODY_PARAM_UINT64(...)        __IC_MACRO_VFUNC(__CHECK_PARAM_TYPE, __check_body_param_uint64, uint64_t, __VA_ARGS__)
#define CHECK_JSON_PARAM_UINT64(...)        __IC_MACRO_VFUNC(__CHECK_PARAM_TYPE, __check_json_param_uint64, uint64_t, __VA_ARGS__)
#define CHECK_JSON_PARAM_UINT64_ARRAY(...)  __IC_MACRO_VFUNC(__CHECK_PARAM_TYPE, __check_json_param_uint64_array, std::vector<uint64_t>, __VA_ARGS__)
#define CHECK_BODY_PARAM_UINT64_EX(...)     __IC_MACRO_VFUNC(__CHECK_PARAM_TYPE, __check_body_param_uint64_ex, uint64_t, __VA_ARGS__)

#define CHECK_URL_PARAM_DOUBLE(...)         __IC_MACRO_VFUNC(__CHECK_PARAM_TYPE, __check_url_param_double, double, __VA_ARGS__)
#define CHECK_BODY_PARAM_DOUBLE(...)        __IC_MACRO_VFUNC(__CHECK_PARAM_TYPE, __check_body_param_double, double, __VA_ARGS__)
#define CHECK_JSON_PARAM_DOUBLE(...)        __IC_MACRO_VFUNC(__CHECK_PARAM_TYPE, __check_json_param_double, double, __VA_ARGS__)
#define CHECK_JSON_PARAM_DOUBLE_ARRAY(...)  __IC_MACRO_VFUNC(__CHECK_PARAM_TYPE, __check_json_param_double_array, std::vector<double>, __VA_ARGS__)
#define CHECK_BODY_PARAM_DOUBLE_EX(...)     __IC_MACRO_VFUNC(__CHECK_PARAM_TYPE, __check_body_param_double_ex, double, __VA_ARGS__)


namespace ic {
namespace server {
namespace helper {

bool __check_url_param_str(Request& req, Response& res, Json::Value& root, const std::string& name, std::string& value);
bool __check_body_param_str(Request& req, Response& res, Json::Value& root, const std::string& name, std::string& value);
bool __check_json_param_str(Request& req, Response& res, Json::Value& root, const std::string& name, std::string& value);
bool __check_json_param_str_array(Request& req, Response& res, Json::Value& root, const std::string& name, std::vector<std::string>& value);
bool __check_body_param_str_ex(Request& req, Response& res, Json::Value& root, const std::string& name, std::string& value);

bool __check_url_param_str_lower(Request& req, Response& res, Json::Value& root, const std::string& name, std::string& value);
bool __check_url_param_str_upper(Request& req, Response& res, Json::Value& root, const std::string& name, std::string& value);
bool __check_body_param_str_lower(Request& req, Response& res, Json::Value& root, const std::string& name, std::string& value);
bool __check_body_param_str_upper(Request& req, Response& res, Json::Value& root, const std::string& name, std::string& value);
bool __check_json_param_str_lower(Request& req, Response& res, Json::Value& root, const std::string& name, std::string& value);
bool __check_json_param_str_upper(Request& req, Response& res, Json::Value& root, const std::string& name, std::string& value);
bool __check_json_param_str_lower_array(Request& req, Response& res, Json::Value& root, const std::string& name, std::vector<std::string>& value);
bool __check_json_param_str_upper_array(Request& req, Response& res, Json::Value& root, const std::string& name, std::vector<std::string>& value);
bool __check_body_param_str_lower_ex(Request& req, Response& res, Json::Value& root, const std::string& name, std::string& value);
bool __check_body_param_str_upper_ex(Request& req, Response& res, Json::Value& root, const std::string& name, std::string& value);

bool __check_url_param_bool(Request& req, Response& res, Json::Value& root, const std::string& name, bool& value);
bool __check_body_param_bool(Request& req, Response& res, Json::Value& root, const std::string& name, bool& value);
bool __check_json_param_bool(Request& req, Response& res, Json::Value& root, const std::string& name, bool& value);
// use std::vector<int32_t> instead of std::vector<bool>
bool __check_json_param_bool_array(Request& req, Response& res, Json::Value& root, const std::string& name, std::vector<int32_t>& value);
bool __check_body_param_bool_ex(Request& req, Response& res, Json::Value& root, const std::string& name, bool& value);

bool __check_url_param_int(Request& req, Response& res, Json::Value& root, const std::string& name, int32_t& value);
bool __check_body_param_int(Request& req, Response& res, Json::Value& root, const std::string& name, int32_t& value);
bool __check_json_param_int(Request& req, Response& res, Json::Value& root, const std::string& name, int32_t& value);
bool __check_json_param_int_array(Request& req, Response& res, Json::Value& root, const std::string& name, std::vector<int32_t>& value);
bool __check_body_param_int_ex(Request& req, Response& res, Json::Value& root, const std::string& name, int32_t& value);

bool __check_url_param_uint(Request& req, Response& res, Json::Value& root, const std::string& name, uint32_t& value);
bool __check_body_param_uint(Request& req, Response& res, Json::Value& root, const std::string& name, uint32_t& value);
bool __check_json_param_uint(Request& req, Response& res, Json::Value& root, const std::string& name, uint32_t& value);
bool __check_json_param_uint_array(Request& req, Response& res, Json::Value& root, const std::string& name, std::vector<uint32_t>& value);
bool __check_body_param_uint_ex(Request& req, Response& res, Json::Value& root, const std::string& name, uint32_t& value);

bool __check_url_param_int64(Request& req, Response& res, Json::Value& root, const std::string& name, int64_t& value);
bool __check_body_param_int64(Request& req, Response& res, Json::Value& root, const std::string& name, int64_t& value);
bool __check_json_param_int64(Request& req, Response& res, Json::Value& root, const std::string& name, int64_t& value);
bool __check_json_param_int64_array(Request& req, Response& res, Json::Value& root, const std::string& name, std::vector<int64_t>& value);
bool __check_body_param_int64_ex(Request& req, Response& res, Json::Value& root, const std::string& name, int64_t& value);

bool __check_url_param_uint64(Request& req, Response& res, Json::Value& root, const std::string& name, uint64_t& value);
bool __check_body_param_uint64(Request& req, Response& res, Json::Value& root, const std::string& name, uint64_t& value);
bool __check_json_param_uint64(Request& req, Response& res, Json::Value& root, const std::string& name, uint64_t& value);
bool __check_json_param_uint64_array(Request& req, Response& res, Json::Value& root, const std::string& name, std::vector<uint64_t>& value);
bool __check_body_param_uint64_ex(Request& req, Response& res, Json::Value& root, const std::string& name, uint64_t& value);

bool __check_url_param_double(Request& req, Response& res, Json::Value& root, const std::string& name, double& value);
bool __check_body_param_double(Request& req, Response& res, Json::Value& root, const std::string& name, double& value);
bool __check_json_param_double(Request& req, Response& res, Json::Value& root, const std::string& name, double& value);
bool __check_json_param_double_array(Request& req, Response& res, Json::Value& root, const std::string& name, std::vector<double>& value);
bool __check_body_param_double_ex(Request& req, Response& res, Json::Value& root, const std::string& name, double& value);

} // namespace helper
} // namespace server
} // namespace ic


#define __CHECK_PARAM_TYPE_3(func, type, _param) \
    type _param;\
    if (!ic::server::helper::func(req, res, root, #_param, _param)) {\
        return;\
    }
#define __CHECK_PARAM_TYPE_4(func, type, _1, _2) \
    __CHECK_PARAM_TYPE_3(func, type, _1); __CHECK_PARAM_TYPE_3(func, type, _2)
#define __CHECK_PARAM_TYPE_5(func, type, _1, _2, _3) \
    __CHECK_PARAM_TYPE_3(func, type, _1); __CHECK_PARAM_TYPE_4(func, type, _2, _3)
#define __CHECK_PARAM_TYPE_6(func, type, _1, _2, _3, _4) \
    __CHECK_PARAM_TYPE_3(func, type, _1); __CHECK_PARAM_TYPE_5(func, type, _2, _3, _4)
#define __CHECK_PARAM_TYPE_7(func, type, _1, _2, _3, _4, _5) \
    __CHECK_PARAM_TYPE_3(func, type, _1); __CHECK_PARAM_TYPE_6(func, type, _2, _3, _4, _5)
#define __CHECK_PARAM_TYPE_8(func, type, _1, _2, _3, _4, _5, _6) \
    __CHECK_PARAM_TYPE_3(func, type, _1); __CHECK_PARAM_TYPE_7(func, type, _2, _3, _4, _5, _6)
#define __CHECK_PARAM_TYPE_9(func, type, _1, _2, _3, _4, _5, _6, _7) \
    __CHECK_PARAM_TYPE_3(func, type, _1); __CHECK_PARAM_TYPE_8(func, type, _2, _3, _4, _5, _6, _7)
#define __CHECK_PARAM_TYPE_10(func, type, _1, _2, _3, _4, _5, _6, _7, _8) \
    __CHECK_PARAM_TYPE_3(func, type, _1); __CHECK_PARAM_TYPE_9(func, type, _2, _3, _4, _5, _6, _7, _8)
#define __CHECK_PARAM_TYPE_11(func, type, _1, _2, _3, _4, _5, _6, _7, _8, _9) \
    __CHECK_PARAM_TYPE_3(func, type, _1); __CHECK_PARAM_TYPE_10(func, type, _2, _3, _4, _5, _6, _7, _8, _9)
#define __CHECK_PARAM_TYPE_12(func, type, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10) \
    __CHECK_PARAM_TYPE_3(func, type, _1); __CHECK_PARAM_TYPE_11(func, type, _2, _3, _4, _5, _6, _7, _8, _9, _10)

#endif // IC_SERVER_HELPER_PARAM_CHECK_H_
