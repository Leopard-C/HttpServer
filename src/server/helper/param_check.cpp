#include "server/helper/param_check.h"
#include "server/helper/helper.h"
#include "server/util/convert/convert_case.h"
#include "server/util/convert/convert_number.h"
#include "server/util/string/trim.h"

#define DEF_FUNC_CHECK_PARAM_STR(func, func_get_param) \
    bool func(Request& req, Response& res, Json::Value& root, const std::string& name, std::string& value) {\
        bool exist;\
        value = req.func_get_param(name, &exist);\
        if (exist) {\
            return true;\
        }\
        RETURN_MISSING_PARAM(name) false;\
    }

#define DEF_FUNC_CHECK_PARAM_STR_CASE(func, func_check_param, func_convert_case) \
    bool func(Request& req, Response& res, Json::Value& root, const std::string& name, std::string& value) {\
        if (func_check_param(req, res, root, name, value)) {\
            util::conv::func_convert_case(value);\
            return true;\
        }\
        return false;\
    }

#define DEF_FUNC_CHECK_PARAM_STR_CASE_ARRAY(func, func_check_param, func_convert_case) \
    bool func(Request& req, Response& res, Json::Value& root, const std::string& name, std::vector<std::string>& value) {\
        if (func_check_param(req, res, root, name, value)) {\
            for (auto& v : value) {\
                util::conv::func_convert_case(v);\
            }\
            return true;\
        }\
        return false;\
    }

#define DEF_FUNC_CHECK_BODY_PARAM_EX(func, func_check_body_param, func_check_json_param, value_type) \
    bool func(Request& req, Response& res, Json::Value& root, const std::string& name, value_type& value) {\
        auto& content_type = req.content_type();\
        if (content_type.IsApplicationJson()) {\
            return func_check_json_param(req, res, root, name, value);\
        }\
        else if (content_type.IsApplicationXWwwFormUrlEncoded()) {\
            return func_check_body_param(req, res, root, name, value);\
        }\
        else {\
            RETURN_MISSING_PARAM(name) false;\
        }\
    }

#define DEF_FUNC_CHECK_PARAM_BOOL(func, func_get_param) \
    bool func(Request& req, Response& res, Json::Value& root, const std::string& name, bool& value) {\
        value = false;\
        bool exist;\
        std::string value_ = req.func_get_param(name, &exist);\
        if (exist) {\
            util::conv::to_lower(util::trim(value_));\
            if (value_ == "true" || value_ == "1") {\
                value = true;\
                return true;\
            }\
            else if (value_ == "false" || value_ == "0") {\
                value = false;\
                return true;\
            }\
            else {\
                RETURN_INVALID_PARAM(name) false;\
            }\
        }\
        return false;\
    }

#define DEF_FUNC_CHECK_JSON_PARAM(func, value_type) \
    bool func(Request& req, Response& res, Json::Value& root, const std::string& name, value_type& value) {\
        const Json::Value& json_value = req.GetJsonParam(name);\
        if (json_value.is<value_type>()) {\
            value = json_value.as<value_type>();\
            return true;\
        }\
        else if (json_value.isNull()) {\
            RETURN_MISSING_PARAM(name) false;\
        }\
        else {\
            RETURN_INVALID_PARAM(name) false;\
        }\
    }

#define DEF_FUNC_CHECK_JSON_PARAM_ARRAY(func, value_type, as_value_type) \
    bool func(Request& req, Response& res, Json::Value& root, const std::string& name, std::vector<as_value_type>& value) {\
        const Json::Value& array_node = req.GetJsonParam(name);\
        if (array_node.isArray()) {\
            unsigned int size = array_node.size();\
            value.reserve(size);\
            for (unsigned int i = 0; i < size; ++i) {\
                if (array_node[i].is<value_type>()) {\
                    value.push_back(array_node[i].as<as_value_type>());\
                }\
                else {\
                    RETURN_INVALID_PARAM(name) false;\
                }\
            }\
            return true;\
        }\
        else if (array_node.isNull()) {\
            RETURN_MISSING_PARAM(name) false;\
        }\
        else {\
            RETURN_INVALID_PARAM(name) false;\
        }\
    }

#define DEF_FUNC_CHECK_PARAM_NUMBER(func, func_get_param, number_type) \
    bool func(Request& req, Response& res, Json::Value& root, const std::string& name, number_type& value) {\
        bool exist;\
        const std::string& value_ = req.func_get_param(name, &exist);\
        if (exist) {\
            if (util::conv::convert_number(value_, &value)){\
                return true;\
            }\
            RETURN_INVALID_PARAM(name) false;\
        }\
        RETURN_MISSING_PARAM(name) false;\
    }

namespace ic {
namespace server {
namespace helper {

/*******************************************************
 * 
 * string
 * 
*******************************************************/
DEF_FUNC_CHECK_PARAM_STR(__check_url_param_str, GetUrlParam)
DEF_FUNC_CHECK_PARAM_STR(__check_body_param_str, GetBodyParam)

bool __check_json_param_str(Request& req, Response& res, Json::Value& root, const std::string& name, std::string& value) {
    auto& json_value = req.GetJsonParam(name);
    if (json_value.isString() || json_value.isNumeric()) {
        value = json_value.asString();
        return true;
    }
    else if (json_value.isNull()) {
        RETURN_MISSING_PARAM(name) false;
    }
    else {
        RETURN_INVALID_PARAM(name) false;
    }
}

DEF_FUNC_CHECK_JSON_PARAM_ARRAY(__check_json_param_str_array, std::string, std::string)
DEF_FUNC_CHECK_BODY_PARAM_EX(__check_body_param_str_ex, __check_body_param_str, __check_json_param_str, std::string)


/*******************************************************
 * 
 * string lower/upper case
 * 
*******************************************************/
DEF_FUNC_CHECK_PARAM_STR_CASE(__check_url_param_str_lower, __check_url_param_str, to_lower)
DEF_FUNC_CHECK_PARAM_STR_CASE(__check_url_param_str_upper, __check_url_param_str, to_upper)
DEF_FUNC_CHECK_PARAM_STR_CASE(__check_body_param_str_lower, __check_body_param_str, to_lower)
DEF_FUNC_CHECK_PARAM_STR_CASE(__check_body_param_str_upper, __check_body_param_str, to_upper)
DEF_FUNC_CHECK_PARAM_STR_CASE(__check_json_param_str_lower, __check_json_param_str, to_lower)
DEF_FUNC_CHECK_PARAM_STR_CASE(__check_json_param_str_upper, __check_json_param_str, to_upper)
DEF_FUNC_CHECK_PARAM_STR_CASE_ARRAY(__check_json_param_str_lower_array, __check_json_param_str_array, to_lower)
DEF_FUNC_CHECK_PARAM_STR_CASE_ARRAY(__check_json_param_str_upper_array, __check_json_param_str_array, to_upper)
DEF_FUNC_CHECK_BODY_PARAM_EX(__check_body_param_str_lower_ex, __check_body_param_str_lower, __check_json_param_str_lower, std::string)
DEF_FUNC_CHECK_BODY_PARAM_EX(__check_body_param_str_upper_ex, __check_body_param_str_upper, __check_json_param_str_upper, std::string)


/*******************************************************
 * 
 * bool
 * 
*******************************************************/
DEF_FUNC_CHECK_PARAM_BOOL(__check_url_param_bool, GetUrlParam)
DEF_FUNC_CHECK_PARAM_BOOL(__check_body_param_bool, GetBodyParam)
DEF_FUNC_CHECK_JSON_PARAM(__check_json_param_bool, bool)
DEF_FUNC_CHECK_JSON_PARAM_ARRAY(__check_json_param_bool_array, bool, int32_t)
DEF_FUNC_CHECK_BODY_PARAM_EX(__check_body_param_bool_ex, __check_body_param_bool, __check_json_param_bool, bool)


/*******************************************************
 * 
 * int32_t
 * 
*******************************************************/
DEF_FUNC_CHECK_PARAM_NUMBER(__check_url_param_int, GetUrlParam, int32_t)
DEF_FUNC_CHECK_PARAM_NUMBER(__check_body_param_int, GetBodyParam, int32_t)
DEF_FUNC_CHECK_JSON_PARAM(__check_json_param_int, int32_t)
DEF_FUNC_CHECK_JSON_PARAM_ARRAY(__check_json_param_int_array, int32_t, int32_t)
DEF_FUNC_CHECK_BODY_PARAM_EX(__check_body_param_int_ex, __check_body_param_int, __check_json_param_int, int32_t)


/*******************************************************
 * 
 * uint32_t
 * 
*******************************************************/
DEF_FUNC_CHECK_PARAM_NUMBER(__check_url_param_uint, GetUrlParam, uint32_t)
DEF_FUNC_CHECK_PARAM_NUMBER(__check_body_param_uint, GetBodyParam, uint32_t)
DEF_FUNC_CHECK_JSON_PARAM(__check_json_param_uint, uint32_t)
DEF_FUNC_CHECK_JSON_PARAM_ARRAY(__check_json_param_uint_array, uint32_t, uint32_t)
DEF_FUNC_CHECK_BODY_PARAM_EX(__check_body_param_uint_ex, __check_body_param_uint, __check_json_param_uint, uint32_t)


/*******************************************************
 * 
 * int64_t
 * 
*******************************************************/
DEF_FUNC_CHECK_PARAM_NUMBER(__check_url_param_int64, GetUrlParam, int64_t)
DEF_FUNC_CHECK_PARAM_NUMBER(__check_body_param_int64, GetBodyParam, int64_t)
DEF_FUNC_CHECK_JSON_PARAM(__check_json_param_int64, int64_t)
DEF_FUNC_CHECK_JSON_PARAM_ARRAY(__check_json_param_int64_array, int64_t, int64_t)
DEF_FUNC_CHECK_BODY_PARAM_EX(__check_body_param_int64_ex, __check_body_param_int64, __check_json_param_int64, int64_t)


/*******************************************************
 * 
 * uint64_t
 * 
*******************************************************/
DEF_FUNC_CHECK_PARAM_NUMBER(__check_url_param_uint64, GetUrlParam, uint64_t)
DEF_FUNC_CHECK_PARAM_NUMBER(__check_body_param_uint64, GetBodyParam, uint64_t)
DEF_FUNC_CHECK_JSON_PARAM(__check_json_param_uint64, uint64_t)
DEF_FUNC_CHECK_JSON_PARAM_ARRAY(__check_json_param_uint64_array, uint64_t, uint64_t)
DEF_FUNC_CHECK_BODY_PARAM_EX(__check_body_param_uint64_ex, __check_body_param_uint64, __check_json_param_uint64, uint64_t)


/*******************************************************
 * 
 * double
 * 
*******************************************************/
DEF_FUNC_CHECK_PARAM_NUMBER(__check_url_param_double, GetUrlParam, double)
DEF_FUNC_CHECK_PARAM_NUMBER(__check_body_param_double, GetBodyParam, double)
DEF_FUNC_CHECK_JSON_PARAM(__check_json_param_double, double)
DEF_FUNC_CHECK_JSON_PARAM_ARRAY(__check_json_param_double_array, double, double)
DEF_FUNC_CHECK_BODY_PARAM_EX(__check_body_param_double_ex, __check_body_param_double, __check_json_param_double, double)

} // namespace helper
} // namespace server
} // namespace ic
