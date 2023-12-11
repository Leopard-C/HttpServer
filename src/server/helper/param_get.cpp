#include "server/helper/param_get.h"
#include "server/helper/helper.h"
#include "server/util/convert/convert_case.h"
#include "server/util/convert/convert_number.h"
#include "server/util/string/trim.h"

#define DEF_FUNC_GET_PARAM_STR(func, func_get_param) \
    bool func(Request& req, Response& res, Json::Value& root, const std::string& name, std::string& value, const std::string& default_value) {\
        bool exist;\
        value = req.func_get_param(name, &exist);\
        if (!exist) {\
            value = default_value;\
        }\
        return true;\
    }

#define DEF_FUNC_GET_PARAM_STR_CASE(func, func_get_param, func_convert_case) \
    bool func(Request& req, Response& res, Json::Value& root, const std::string& name, std::string& value, const std::string& default_value) {\
        if (func_get_param(req, res, root, name, value, default_value)) {\
            util::conv::func_convert_case(value);\
            return true;\
        }\
        return false;\
    }

#define DEF_FUNC_GET_PARAM_STR_CASE_ARRAY(func, func_get_param, func_convert_case) \
    bool func(Request& req, Response& res, Json::Value& root, const std::string& name, std::vector<std::string>& value, const std::vector<std::string>& default_value) {\
        if (func_get_param(req, res, root, name, value, default_value)) {\
            for (auto& v : value) {\
                util::conv::func_convert_case(v);\
            }\
            return true;\
        }\
        return false;\
    }

#define DEF_FUNC_GET_BODY_PARAM_EX(func, func_get_body_param, func_get_json_param, value_type, default_value_type) \
    bool func(Request& req, Response& res, Json::Value& root, const std::string& name, value_type& value, default_value_type default_value) {\
        auto& content_type = req.content_type();\
        if (content_type.IsApplicationJson()) {\
            return func_get_json_param(req, res, root, name, value, default_value);\
        }\
        else if (content_type.IsApplicationXWwwFormUrlEncoded() || content_type.type().empty()) {\
            return func_get_body_param(req, res, root, name, value, default_value);\
        }\
        else {\
            RETURN_MISSING_PARAM(name) false;\
        }\
    }

#define DEF_FUNC_GET_PARAM_BOOL(func, func_get_param) \
    bool func(Request& req, Response& res, Json::Value& root, const std::string& name, bool& value, bool default_value) {\
        bool exist;\
        std::string value_ = req.func_get_param(name, &exist);\
        if (exist) {\
            util::conv::to_lower(util::trim(value_));\
            if (value_ == "true" || value_ == "1"){\
                value = true;\
                return true;\
            }\
            else if (value_ == "false" || value_ == "0"){\
                value = false;\
                return true;\
            }\
            else {\
                RETURN_INVALID_PARAM(name) false;\
            }\
        }\
        else {\
            value = default_value;\
            return true;\
        }\
    }

#define DEF_FUNC_GET_JSON_PARAM(func, value_type) \
    bool func(Request& req, Response& res, Json::Value& root, const std::string& name, value_type& value, value_type default_value) {\
        const Json::Value& json_value = req.GetJsonParam(name);\
        if (json_value.is<value_type>()) {\
            value = json_value.as<value_type>();\
            return true;\
        }\
        else if (json_value.isNull()) {\
            value = default_value;\
            return true;\
        }\
        else {\
            RETURN_INVALID_PARAM(name) false;\
        }\
    }

#define DEF_FUNC_GET_JSON_PARAM_ARRAY(func, value_type, as_value_type) \
    bool func(Request& req, Response& res, Json::Value& root, const std::string& name, std::vector<as_value_type>& value, const std::vector<as_value_type>& default_value) {\
        const Json::Value& array_node = req.GetJsonParam(name);\
        if (array_node.isArray()) {\
            if (array_node.size()) {\
                if (array_node[0].is<value_type>()) {\
                    unsigned int size = array_node.size();\
                    value.reserve(size);\
                    for (unsigned int i = 0; i < size; ++i) {\
                        value.push_back(array_node[i].as<as_value_type>());\
                    }\
                    return true;\
                }\
                else {\
                    RETURN_INVALID_PARAM(name) false;\
                }\
            }\
            return true;\
        }\
        else if (array_node.isNull()) {\
            value = default_value;\
            return true;\
        }\
        else {\
            RETURN_INVALID_PARAM(name) false;\
        }\
    }

#define DEF_FUNC_GET_PARAM_NUMBER(func, func_get_param, number_type) \
    bool func(Request& req, Response& res, Json::Value& root, const std::string& name, number_type& value, number_type default_value) {\
        bool exist;\
        const std::string& value_ = req.func_get_param(name, &exist);\
        if (exist) {\
            if (util::conv::convert_number(value_, &value)) {\
                return true;\
            }\
            RETURN_INVALID_PARAM(name) false;\
        }\
        else {\
            value = default_value;\
            return true;\
        }\
    }

namespace ic {
namespace server {
namespace helper {

/*******************************************************
 * 
 * string
 * 
*******************************************************/
DEF_FUNC_GET_PARAM_STR(__get_url_param_str, GetUrlParam)
DEF_FUNC_GET_PARAM_STR(__get_body_param_str, GetBodyParam)

bool __get_json_param_str(Request& req, Response& res, Json::Value& root, const std::string& name, std::string& value, const std::string& default_value) {
    auto& json_value = req.GetJsonParam(name);
    if (json_value.isString() || json_value.isNumeric()) {
        value = json_value.asString();
        return true;
    }
    else if (json_value.isNull()) {
        value = default_value;
        return true;
    }
    else {
        RETURN_INVALID_PARAM(name) false;
    }
}

bool __get_json_param_str_array(Request& req, Response& res, Json::Value& root, const std::string& name, std::vector<std::string>& value, const std::vector<std::string>& default_value) {
    const Json::Value& array_node = req.GetJsonParam(name);
    if (array_node.isArray()) {
        if (array_node.size()) {
            if (array_node[0].isString() || array_node[0].isNumeric()) {
                unsigned int size = array_node.size();
                value.reserve(size);
                for (unsigned int i = 0; i < size; ++i) {
                    value.push_back(array_node[i].asString());
                }
                return true;
            }
            else {
                RETURN_INVALID_PARAM(name) false;
            }
        }
        return true;
    }
    else if (array_node.isNull()) {
        value = default_value;
        return true;
    }
    else {
        RETURN_INVALID_PARAM(name) false;
    }
}

DEF_FUNC_GET_BODY_PARAM_EX(__get_body_param_str_ex, __get_body_param_str, __get_json_param_str, std::string, const std::string&)


/*******************************************************
 * 
 * string lower/upper case
 * 
*******************************************************/
DEF_FUNC_GET_PARAM_STR_CASE(__get_url_param_str_lower, __get_url_param_str, to_lower)
DEF_FUNC_GET_PARAM_STR_CASE(__get_url_param_str_upper, __get_url_param_str, to_upper)
DEF_FUNC_GET_PARAM_STR_CASE(__get_body_param_str_lower, __get_body_param_str, to_lower)
DEF_FUNC_GET_PARAM_STR_CASE(__get_body_param_str_upper, __get_body_param_str, to_upper)
DEF_FUNC_GET_PARAM_STR_CASE(__get_json_param_str_lower, __get_json_param_str, to_lower)
DEF_FUNC_GET_PARAM_STR_CASE(__get_json_param_str_upper, __get_json_param_str, to_upper)
DEF_FUNC_GET_PARAM_STR_CASE_ARRAY(__get_json_param_str_lower_array, __get_json_param_str_array, to_lower)
DEF_FUNC_GET_PARAM_STR_CASE_ARRAY(__get_json_param_str_upper_array, __get_json_param_str_array, to_upper)
DEF_FUNC_GET_BODY_PARAM_EX(__get_body_param_str_lower_ex, __get_body_param_str_lower, __get_json_param_str_lower, std::string, const std::string&)
DEF_FUNC_GET_BODY_PARAM_EX(__get_body_param_str_upper_ex, __get_body_param_str_upper, __get_json_param_str_upper, std::string, const std::string&)


/*******************************************************
 * 
 * bool
 * 
*******************************************************/
DEF_FUNC_GET_PARAM_BOOL(__get_url_param_bool, GetUrlParam)
DEF_FUNC_GET_PARAM_BOOL(__get_body_param_bool, GetBodyParam)
DEF_FUNC_GET_JSON_PARAM(__get_json_param_bool, bool)
DEF_FUNC_GET_JSON_PARAM_ARRAY(__get_json_param_bool_array, bool, int)
DEF_FUNC_GET_BODY_PARAM_EX(__get_body_param_bool_ex, __get_body_param_bool, __get_json_param_bool, bool, bool)


/*******************************************************
 * 
 * int32_t
 * 
*******************************************************/
DEF_FUNC_GET_PARAM_NUMBER(__get_url_param_int, GetUrlParam, int32_t)
DEF_FUNC_GET_PARAM_NUMBER(__get_body_param_int, GetBodyParam, int32_t)
DEF_FUNC_GET_JSON_PARAM(__get_json_param_int, int32_t)
DEF_FUNC_GET_JSON_PARAM_ARRAY(__get_json_param_int_array, int32_t, int32_t)
DEF_FUNC_GET_BODY_PARAM_EX(__get_body_param_int_ex, __get_body_param_int, __get_json_param_int, int32_t, int32_t)


/*******************************************************
 * 
 * uint32_t
 * 
*******************************************************/
DEF_FUNC_GET_PARAM_NUMBER(__get_url_param_uint, GetUrlParam, uint32_t)
DEF_FUNC_GET_PARAM_NUMBER(__get_body_param_uint, GetBodyParam, uint32_t)
DEF_FUNC_GET_JSON_PARAM(__get_json_param_uint, uint32_t)
DEF_FUNC_GET_JSON_PARAM_ARRAY(__get_json_param_uint_array, uint32_t, uint32_t)
DEF_FUNC_GET_BODY_PARAM_EX(__get_body_param_uint_ex, __get_body_param_uint, __get_json_param_uint, uint32_t, uint32_t)


/*******************************************************
 * 
 * int64_t
 * 
*******************************************************/
DEF_FUNC_GET_PARAM_NUMBER(__get_url_param_int64, GetUrlParam, int64_t)
DEF_FUNC_GET_PARAM_NUMBER(__get_body_param_int64, GetBodyParam, int64_t)
DEF_FUNC_GET_JSON_PARAM(__get_json_param_int64, int64_t)
DEF_FUNC_GET_JSON_PARAM_ARRAY(__get_json_param_int64_array, int64_t, int64_t)
DEF_FUNC_GET_BODY_PARAM_EX(__get_body_param_int64_ex, __get_body_param_int64, __get_json_param_int64, int64_t, int64_t)


/*******************************************************
 * 
 * uint64_t
 * 
*******************************************************/
DEF_FUNC_GET_PARAM_NUMBER(__get_url_param_uint64, GetUrlParam, uint64_t)
DEF_FUNC_GET_PARAM_NUMBER(__get_body_param_uint64, GetBodyParam, uint64_t)
DEF_FUNC_GET_JSON_PARAM(__get_json_param_uint64, uint64_t)
DEF_FUNC_GET_JSON_PARAM_ARRAY(__get_json_param_uint64_array, uint64_t, uint64_t)
DEF_FUNC_GET_BODY_PARAM_EX(__get_body_param_uint64_ex, __get_body_param_uint64, __get_json_param_uint64, uint64_t, uint64_t)


/*******************************************************
 * 
 * double
 * 
*******************************************************/
DEF_FUNC_GET_PARAM_NUMBER(__get_url_param_double, GetUrlParam, double)
DEF_FUNC_GET_PARAM_NUMBER(__get_body_param_double, GetBodyParam, double)
DEF_FUNC_GET_JSON_PARAM(__get_json_param_double, double)
DEF_FUNC_GET_JSON_PARAM_ARRAY(__get_json_param_double_array, double, double)
DEF_FUNC_GET_BODY_PARAM_EX(__get_body_param_double_ex, __get_body_param_double, __get_json_param_double, double, double)

} // namespace helper
} // namespace server
} // namespace ic
