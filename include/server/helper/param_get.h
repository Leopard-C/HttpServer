#ifndef IC_SERVER_HELPER_PARAM_GET_H_
#define IC_SERVER_HELPER_PARAM_GET_H_
#include "_narg.h"
#include "../request.h"
#include "../response.h"

/***************************************************************************************************
 *
 *    获取 URL、body 中的参数，如果不存在，则使用默认值.
 * 
 *       + GET_URL_PARAM_STR(_default, ...)        // URL
 *       + GET_URL_PARAM_STR_LOWER(_default, ...)
 *       + GET_URL_PARAM_STR_UPPER(_default, ...)
 *       + GET_URL_PARAM_BOOL(_default, ...)
 *       + GET_URL_PARAM_INT(_default, ...)
 *       + GET_URL_PARAM_UINT(_default, ...)
 *       + GET_URL_PARAM_INT64(_default, ...)
 *       + GET_URL_PARAM_UINT64(_default, ...)
 *       + GET_URL_PARAM_DOUBLE(_default, ...)
 * 
 *       + GET_BODY_PARAM_STR(_default, ...)       // (1)application/x-www-form-urlencoded
 *       + ...
 * 
 *       + GET_JSON_PARAM_STR(_default, ...)       // (2)application/json
 *       + ...
 * 
 *       + GET_BODY_PARAM_STR_EX(_default, ...)    // 自动选择(1)或(2)，即接口同时支持这两种类型的请求
 *       + ...
 * 
 *    使用示例:
 *       某次GET请求 /article/list?pn=1&ps=20&type=source
 *       其中，参数pn(页码), ps(每页大小)和tyep(文章类型)均为可选
 *       可以在控制器中使用如下方式获取参数的值：
 *           void ArticleController::List(Request& req, Response& res) {
 *               API_INIT();
 *               GET_URL_PARAM_INT(1, pn);   // 获取pn的参数值，如果不存在将使用默认值1
 *               GET_URL_PARAM_INT(20, ps);  // 获取ps的参数值，不过不存在将使用默认值20
 *               GET_URL_PARAM_STR("source", type);
 *               // ...
 *               RETURN_OK();
 *           }
 *
***************************************************************************************************/

#define GET_URL_PARAM_STR(_default, ...)         __IC_MACRO_VFUNC(__GET_PARAM_TYPE, __get_url_param_str, std::string, _default, __VA_ARGS__)
#define GET_BODY_PARAM_STR(_default, ...)        __IC_MACRO_VFUNC(__GET_PARAM_TYPE, __get_body_param_str, std::string, _default, __VA_ARGS__)
#define GET_JSON_PARAM_STR(_default, ...)        __IC_MACRO_VFUNC(__GET_PARAM_TYPE, __get_json_param_str, std::string, _default, __VA_ARGS__)
#define GET_JSON_PARAM_STR_ARRAY(_default, ...)  __IC_MACRO_VFUNC(__GET_PARAM_TYPE, __get_json_param_str_array, std::vector<std::string>, _default, __VA_ARGS__)
#define GET_BODY_PARAM_STR_EX(_default, ...)     __IC_MACRO_VFUNC(__GET_PARAM_TYPE, __get_body_param_str_ex, std::string, _default, __VA_ARGS__)

#define GET_URL_PARAM_STR_LOWER(_default, ...)         __IC_MACRO_VFUNC(__GET_PARAM_TYPE, __get_url_param_str_lower, std::string, _default, __VA_ARGS__)
#define GET_URL_PARAM_STR_UPPER(_default, ...)         __IC_MACRO_VFUNC(__GET_PARAM_TYPE, __get_url_param_str_upper, std::string, _default, __VA_ARGS__)
#define GET_BODY_PARAM_STR_LOWER(_default, ...)        __IC_MACRO_VFUNC(__GET_PARAM_TYPE, __get_body_param_str_lower, std::string, _default, __VA_ARGS__)
#define GET_BODY_PARAM_STR_UPPER(_default, ...)        __IC_MACRO_VFUNC(__GET_PARAM_TYPE, __get_body_param_str_upper, std::string, _default, __VA_ARGS__)
#define GET_JSON_PARAM_STR_LOWER(_default, ...)        __IC_MACRO_VFUNC(__GET_PARAM_TYPE, __get_json_param_str_lower, std::string, _default, __VA_ARGS__)
#define GET_JSON_PARAM_STR_UPPER(_default, ...)        __IC_MACRO_VFUNC(__GET_PARAM_TYPE, __get_json_param_str_upper, std::string, _default, __VA_ARGS__)
#define GET_JSON_PARAM_STR_LOWER_ARRAY(_default, ...)  __IC_MACRO_VFUNC(__GET_PARAM_TYPE, __get_json_param_str_lower_array, std::vector<std::string>, _default, __VA_ARGS__)
#define GET_JSON_PARAM_STR_UPPER_ARRAY(_default, ...)  __IC_MACRO_VFUNC(__GET_PARAM_TYPE, __get_json_param_str_upper_array, std::vector<std::string>, _default, __VA_ARGS__)
#define GET_BODY_PARAM_STR_LOWER_EX(_default, ...)     __IC_MACRO_VFUNC(__GET_PARAM_TYPE, __get_body_param_str_lower_ex, std::string, _default, __VA_ARGS__)
#define GET_BODY_PARAM_STR_UPPER_EX(_default, ...)     __IC_MACRO_VFUNC(__GET_PARAM_TYPE, __get_body_param_str_upper_ex, std::string, _default, __VA_ARGS__)

#define GET_URL_PARAM_BOOL(_default, ...)         __IC_MACRO_VFUNC(__GET_PARAM_TYPE, __get_url_param_bool, bool, _default, __VA_ARGS__)
#define GET_BODY_PARAM_BOOL(_default, ...)        __IC_MACRO_VFUNC(__GET_PARAM_TYPE, __get_body_param_bool, bool, _default, __VA_ARGS__)
#define GET_JSON_PARAM_BOOL(_default, ...)        __IC_MACRO_VFUNC(__GET_PARAM_TYPE, __get_json_param_bool, bool, _default, __VA_ARGS__)
#define GET_JSON_PARAM_BOOL_ARRAY(_default, ...)  __IC_MACRO_VFUNC(__GET_PARAM_TYPE, __get_json_param_bool_array, std::vector<int32_t>, _default, __VA_ARGS__)
#define GET_BODY_PARAM_BOOL_EX(_default, ...)     __IC_MACRO_VFUNC(__GET_PARAM_TYPE, __get_body_param_bool_ex, bool, _default, __VA_ARGS__)

#define GET_URL_PARAM_INT(_default, ...)         __IC_MACRO_VFUNC(__GET_PARAM_TYPE, __get_url_param_int, int32_t, _default, __VA_ARGS__)
#define GET_BODY_PARAM_INT(_default, ...)        __IC_MACRO_VFUNC(__GET_PARAM_TYPE, __get_body_param_int, int32_t, _default, __VA_ARGS__)
#define GET_JSON_PARAM_INT(_default, ...)        __IC_MACRO_VFUNC(__GET_PARAM_TYPE, __get_json_param_int, int32_t, _default, __VA_ARGS__)
#define GET_JSON_PARAM_INT_ARRAY(_default, ...)  __IC_MACRO_VFUNC(__GET_PARAM_TYPE, __get_json_param_int_array, std::vector<int32_t>, _default, __VA_ARGS__)
#define GET_BODY_PARAM_INT_EX(_default, ...)     __IC_MACRO_VFUNC(__GET_PARAM_TYPE, __get_body_param_int_ex, int32_t, _default, __VA_ARGS__)

#define GET_URL_PARAM_UINT(_default, ...)         __IC_MACRO_VFUNC(__GET_PARAM_TYPE, __get_url_param_uint, uint32_t, _default, __VA_ARGS__)
#define GET_BODY_PARAM_UINT(_default, ...)        __IC_MACRO_VFUNC(__GET_PARAM_TYPE, __get_body_param_uint, uint32_t, _default, __VA_ARGS__)
#define GET_JSON_PARAM_UINT(_default, ...)        __IC_MACRO_VFUNC(__GET_PARAM_TYPE, __get_json_param_uint, uint32_t, _default, __VA_ARGS__)
#define GET_JSON_PARAM_UINT_ARRAY(_default, ...)  __IC_MACRO_VFUNC(__GET_PARAM_TYPE, __get_json_param_uint_array, std::vector<uint32_t>, _default, __VA_ARGS__)
#define GET_BODY_PARAM_UINT_EX(_default, ...)     __IC_MACRO_VFUNC(__GET_PARAM_TYPE, __get_body_param_uint_ex, uint32_t, _default, __VA_ARGS__)

#define GET_URL_PARAM_INT64(_default, ...)         __IC_MACRO_VFUNC(__GET_PARAM_TYPE, __get_url_param_int64, int64_t, _default, __VA_ARGS__)
#define GET_BODY_PARAM_INT64(_default, ...)        __IC_MACRO_VFUNC(__GET_PARAM_TYPE, __get_body_param_int64, int64_t, _default, __VA_ARGS__)
#define GET_JSON_PARAM_INT64(_default, ...)        __IC_MACRO_VFUNC(__GET_PARAM_TYPE, __get_json_param_int64, int64_t, _default, __VA_ARGS__)
#define GET_JSON_PARAM_INT64_ARRAY(_default, ...)  __IC_MACRO_VFUNC(__GET_PARAM_TYPE, __get_json_param_int64_array, std::vector<int64_t>, _default, __VA_ARGS__)
#define GET_BODY_PARAM_INT64_EX(_default, ...)     __IC_MACRO_VFUNC(__GET_PARAM_TYPE, __get_body_param_int64_ex, int64_t, _default, __VA_ARGS__)

#define GET_URL_PARAM_UINT64(_default, ...)         __IC_MACRO_VFUNC(__GET_PARAM_TYPE, __get_url_param_uint64, uint64_t, _default, __VA_ARGS__)
#define GET_BODY_PARAM_UINT64(_default, ...)        __IC_MACRO_VFUNC(__GET_PARAM_TYPE, __get_body_param_uint64, uint64_t, _default, __VA_ARGS__)
#define GET_JSON_PARAM_UINT64(_default, ...)        __IC_MACRO_VFUNC(__GET_PARAM_TYPE, __get_json_param_uint64, uint64_t, _default, __VA_ARGS__)
#define GET_JSON_PARAM_UINT64_ARRAY(_default, ...)  __IC_MACRO_VFUNC(__GET_PARAM_TYPE, __get_json_param_uint64_array, std::vector<uint64_t>, _default, __VA_ARGS__)
#define GET_BODY_PARAM_UINT64_EX(_default, ...)     __IC_MACRO_VFUNC(__GET_PARAM_TYPE, __get_body_param_uint64_ex, uint64_t, _default, __VA_ARGS__)

#define GET_URL_PARAM_DOUBLE(_default, ...)         __IC_MACRO_VFUNC(__GET_PARAM_TYPE, __get_url_param_double, double, _default, __VA_ARGS__)
#define GET_BODY_PARAM_DOUBLE(_default, ...)        __IC_MACRO_VFUNC(__GET_PARAM_TYPE, __get_body_param_double, double, _default, __VA_ARGS__)
#define GET_JSON_PARAM_DOUBLE(_default, ...)        __IC_MACRO_VFUNC(__GET_PARAM_TYPE, __get_json_param_double, double, _default, __VA_ARGS__)
#define GET_JSON_PARAM_DOUBLE_ARRAY(_default, ...)  __IC_MACRO_VFUNC(__GET_PARAM_TYPE, __get_json_param_double_array, std::vector<double>, _default, __VA_ARGS__)
#define GET_BODY_PARAM_DOUBLE_EX(_default, ...)     __IC_MACRO_VFUNC(__GET_PARAM_TYPE, __get_body_param_double_ex, double, _default, __VA_ARGS__)


namespace ic {
namespace server {
namespace helper {

bool __get_url_param_str(Request& req, Response& res, Json::Value& root, const std::string& name, std::string& value, const std::string& default_value);
bool __get_body_param_str(Request& req, Response& res, Json::Value& root, const std::string& name, std::string& value, const std::string& default_value);
bool __get_json_param_str(Request& req, Response& res, Json::Value& root, const std::string& name, std::string& value, const std::string& default_value);
bool __get_json_param_str_array(Request& req, Response& res, Json::Value& root, const std::string& name, std::vector<std::string>& value, const std::vector<std::string>& default_value);
bool __get_body_param_str_ex(Request& req, Response& res, Json::Value& root, const std::string& name, std::string& value, const std::string& default_value);

bool __get_url_param_str_lower(Request& req, Response& res, Json::Value& root, const std::string& name, std::string& value, const std::string& default_value);
bool __get_url_param_str_upper(Request& req, Response& res, Json::Value& root, const std::string& name, std::string& value, const std::string& default_value);
bool __get_body_param_str_lower(Request& req, Response& res, Json::Value& root, const std::string& name, std::string& value, const std::string& default_value);
bool __get_body_param_str_upper(Request& req, Response& res, Json::Value& root, const std::string& name, std::string& value, const std::string& default_value);
bool __get_json_param_str_lower(Request& req, Response& res, Json::Value& root, const std::string& name, std::string& value, const std::string& default_value);
bool __get_json_param_str_upper(Request& req, Response& res, Json::Value& root, const std::string& name, std::string& value, const std::string& default_value);
bool __get_json_param_str_lower_array(Request& req, Response& res, Json::Value& root, const std::string& name, std::vector<std::string>& value, const std::vector<std::string>& default_value);
bool __get_json_param_str_upper_array(Request& req, Response& res, Json::Value& root, const std::string& name, std::vector<std::string>& value, const std::vector<std::string>& default_value);
bool __get_body_param_str_lower_ex(Request& req, Response& res, Json::Value& root, const std::string& name, std::string& value, const std::string& default_value);
bool __get_body_param_str_upper_ex(Request& req, Response& res, Json::Value& root, const std::string& name, std::string& value, const std::string& default_value);

bool __get_url_param_bool(Request& req, Response& res, Json::Value& root, const std::string& name, bool& value, bool default_value);
bool __get_body_param_bool(Request& req, Response& res, Json::Value& root, const std::string& name, bool& value, bool default_value);
bool __get_json_param_bool(Request& req, Response& res, Json::Value& root, const std::string& name, bool& value, bool default_value);
bool __get_json_param_bool_array(Request& req, Response& res, Json::Value& root, const std::string& name, std::vector<int32_t>& value, const std::vector<int32_t>& default_value);
bool __get_body_param_bool_ex(Request& req, Response& res, Json::Value& root, const std::string& name, bool& value, bool default_value);

bool __get_url_param_int(Request& req, Response& res, Json::Value& root, const std::string& name, int32_t& value, int32_t default_value);
bool __get_body_param_int(Request& req, Response& res, Json::Value& root, const std::string& name, int32_t& value, int32_t default_value);
bool __get_json_param_int(Request& req, Response& res, Json::Value& root, const std::string& name, int32_t& value, int32_t default_value);
bool __get_json_param_int_array(Request& req, Response& res, Json::Value& root, const std::string& name, std::vector<int32_t>& value, const std::vector<int32_t>& default_value);
bool __get_body_param_int_ex(Request& req, Response& res, Json::Value& root, const std::string& name, int32_t& value, int32_t default_value);

bool __get_url_param_uint(Request& req, Response& res, Json::Value& root, const std::string& name, uint32_t& value, uint32_t default_value);
bool __get_body_param_uint(Request& req, Response& res, Json::Value& root, const std::string& name, uint32_t& value, uint32_t default_value);
bool __get_json_param_uint(Request& req, Response& res, Json::Value& root, const std::string& name, uint32_t& value, uint32_t default_value);
bool __get_json_param_uint_array(Request& req, Response& res, Json::Value& root, const std::string& name, std::vector<uint32_t>& value, const std::vector<uint32_t>& default_value);
bool __get_body_param_uint_ex(Request& req, Response& res, Json::Value& root, const std::string& name, uint32_t& value, uint32_t default_value);

bool __get_url_param_int64(Request& req, Response& res, Json::Value& root, const std::string& name, int64_t& value, int64_t default_value);
bool __get_body_param_int64(Request& req, Response& res, Json::Value& root, const std::string& name, int64_t& value, int64_t default_value);
bool __get_json_param_int64(Request& req, Response& res, Json::Value& root, const std::string& name, int64_t& value, int64_t default_value);
bool __get_json_param_int64_array(Request& req, Response& res, Json::Value& root, const std::string& name, std::vector<int64_t>& value, const std::vector<int64_t>& default_value);
bool __get_body_param_int64_ex(Request& req, Response& res, Json::Value& root, const std::string& name, int64_t& value, int64_t default_value);

bool __get_url_param_uint64(Request& req, Response& res, Json::Value& root, const std::string& name, uint64_t& value, uint64_t default_value);
bool __get_body_param_uint64(Request& req, Response& res, Json::Value& root, const std::string& name, uint64_t& value, uint64_t default_value);
bool __get_json_param_uint64(Request& req, Response& res, Json::Value& root, const std::string& name, uint64_t& value, uint64_t default_value);
bool __get_json_param_uint64_array(Request& req, Response& res, Json::Value& root, const std::string& name, std::vector<uint64_t>& value, const std::vector<uint64_t>& default_value);
bool __get_body_param_uint64_ex(Request& req, Response& res, Json::Value& root, const std::string& name, uint64_t& value, uint64_t default_value);

bool __get_url_param_double(Request& req, Response& res, Json::Value& root, const std::string& name, double& value, double default_value);
bool __get_body_param_double(Request& req, Response& res, Json::Value& root, const std::string& name, double& value, double default_value);
bool __get_json_param_double(Request& req, Response& res, Json::Value& root, const std::string& name, double& value, double default_value);
bool __get_json_param_double_array(Request& req, Response& res, Json::Value& root, const std::string& name, std::vector<double>& value, const std::vector<double>& default_value);
bool __get_body_param_double_ex(Request& req, Response& res, Json::Value& root, const std::string& name, double& value, double default_value);

} // namespace helper
} // namespace server
} // namespace ic


#define __GET_PARAM_TYPE_4(func, type, _default, _param) \
    type _param;\
    if (!ic::server::helper::func(req, res, root, #_param, _param, _default)) {\
        return;\
    }
#define __GET_PARAM_TYPE_5(func, type, _default, _1, _2) \
    __GET_PARAM_TYPE_4(func, type, _default, _1); __GET_PARAM_TYPE_4(func, type, _default, _2)
#define __GET_PARAM_TYPE_6(func, type, _default, _1, _2, _3) \
    __GET_PARAM_TYPE_4(func, type, _default, _1); __GET_PARAM_TYPE_5(func, type, _default, _2, _3)
#define __GET_PARAM_TYPE_7(func, type, _default, _1, _2, _3, _4) \
    __GET_PARAM_TYPE_4(func, type, _default, _1); __GET_PARAM_TYPE_6(func, type, _default, _2, _3, _4)
#define __GET_PARAM_TYPE_8(func, type, _default, _1, _2, _3, _4, _5) \
    __GET_PARAM_TYPE_4(func, type, _default, _1); __GET_PARAM_TYPE_7(func, type, _default, _2, _3, _4, _5)
#define __GET_PARAM_TYPE_9(func, type, _default, _1, _2, _3, _4, _5, _6) \
    __GET_PARAM_TYPE_4(func, type, _default, _1); __GET_PARAM_TYPE_8(func, type, _default, _2, _3, _4, _5, _6)
#define __GET_PARAM_TYPE_10(func, type, _default, _1, _2, _3, _4, _5, _6, _7) \
    __GET_PARAM_TYPE_4(func, type, _default, _1); __GET_PARAM_TYPE_9(func, type, _default, _2, _3, _4, _5, _6, _7)
#define __GET_PARAM_TYPE_11(func, type, _default, _1, _2, _3, _4, _5, _6, _7, _8) \
    __GET_PARAM_TYPE_4(func, type, _default, _1); __GET_PARAM_TYPE_10(func, type, _default, _2, _3, _4, _5, _6, _7, _8)
#define __GET_PARAM_TYPE_12(func, type, _default, _1, _2, _3, _4, _5, _6, _7, _8, _9) \
    __GET_PARAM_TYPE_4(func, type, _default, _1); __GET_PARAM_TYPE_11(func, type, _default, _2, _3, _4, _5, _6, _7, _8, _9)
#define __GET_PARAM_TYPE_13(func, type, _default, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10) \
    __GET_PARAM_TYPE_4(func, type, _default, _1); __GET_PARAM_TYPE_12(func, type, _default, _2, _3, _4, _5, _6, _7, _8, _9, _10)

#endif // IC_SERVER_HELPER_PARAM_GET_H_
