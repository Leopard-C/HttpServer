#include "server/helper/param_check.h"
#include "server/helper/helper.h"
#include "server/util/convert/convert_case.h"
#include "server/util/convert/convert_number.h"
#include "server/util/string/trim.h"

#define DEF_FUNC_CHECK_BODY_PARAM_EX(func, func_check_body_param, func_check_form_param, func_check_json_param, value_type) \
    bool func(Request& req, Response& res, Json::Value& root, const std::string& name, value_type& value) {\
        auto& content_type = req.content_type();\
        if (content_type.IsApplicationJson()) {\
            return func_check_json_param(req, res, root, name, value);\
        }\
        else if (content_type.IsApplicationXWwwFormUrlEncoded()) {\
            return func_check_body_param(req, res, root, name, value);\
        }\
        else if (content_type.IsMultipartFormData()) {\
            return func_check_form_param(req, res, root, name, value);\
        }\
        else {\
            RETURN_MISSING_PARAM(name) false;\
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
#pragma region string
bool __check_url_param_str(Request& req, Response& res, Json::Value& root, const std::string& name, std::string& value) {
    bool exist;
    value = req.GetUrlParam(name, &exist);
    if (exist) {
        return true;
    }
    RETURN_MISSING_PARAM(name) false;
}

bool __check_body_param_str(Request& req, Response& res, Json::Value& root, const std::string& name, std::string& value) {
    bool exist;
    value = req.GetBodyParam(name, &exist);
    if (exist) {
        return true;
    }
    RETURN_MISSING_PARAM(name) false;
}

bool __check_form_param_str(Request& req, Response& res, Json::Value& root, const std::string& name, std::string& value) {
    auto param = req.GetFormParam(name);
    if (param) {
        value = param->content().ToString();
        return true;
    }
    RETURN_MISSING_PARAM(name) false;
}

bool __check_json_param_str(Request& req, Response& res, Json::Value& root, const std::string& name, std::string& value) {
    auto& json_value = req.GetJsonParam(name);
    if (json_value.isConvertibleTo(Json::ValueType::stringValue)) {
        value = json_value.asString();
        return true;
    }
    RETURN_INVALID_PARAM(name) false;
}

bool __check_json_param_str_array(Request& req, Response& res, Json::Value& root, const std::string& name, std::vector<std::string>& value) {
    const Json::Value& array_node = req.GetJsonParam(name);
    if (array_node.isArray()) {
        unsigned int size = array_node.size();
        value.reserve(size);
        for (unsigned int i = 0; i < size; ++i) {
            if (array_node[i].isConvertibleTo(Json::ValueType::stringValue)) {
                value.push_back(array_node[i].asString());
            }
            else {
                RETURN_INVALID_PARAM(name) false;
            }
        }
        return true;
    }
    else if (array_node.isNull()) {
        RETURN_MISSING_PARAM(name) false;
    }
    else {
        RETURN_INVALID_PARAM(name) false;
    }
}

DEF_FUNC_CHECK_BODY_PARAM_EX(__check_body_param_str_ex, __check_body_param_str, __check_form_param_str, __check_json_param_str, std::string)
#pragma endregion


/*******************************************************
 * 
 * string lower/upper case
 * 
*******************************************************/
#pragma region string-lower-upper
bool __check_url_param_str_lower(Request& req, Response& res, Json::Value& root, const std::string& name, std::string& value) {
    if (__check_url_param_str(req, res, root, name, value)) {
        util::conv::to_lower(value);
        return true;
    }
    return false;
}

bool __check_url_param_str_upper(Request& req, Response& res, Json::Value& root, const std::string& name, std::string& value) {
    if (__check_url_param_str(req, res, root, name, value)) {
        util::conv::to_upper(value);
        return true;
    }
    return false;
}

bool __check_body_param_str_lower(Request& req, Response& res, Json::Value& root, const std::string& name, std::string& value) {
    if (__check_body_param_str(req, res, root, name, value)) {
        util::conv::to_lower(value);
        return true;
    }
    return false;
}

bool __check_body_param_str_upper(Request& req, Response& res, Json::Value& root, const std::string& name, std::string& value) {
    if (__check_body_param_str(req, res, root, name, value)) {
        util::conv::to_upper(value);
        return true;
    }
    return false;
}

bool __check_form_param_str_lower(Request& req, Response& res, Json::Value& root, const std::string& name, std::string& value) {
    if (__check_form_param_str(req, res, root, name, value)) {
        util::conv::to_lower(value);
        return true;
    }
    return false;
}

bool __check_form_param_str_upper(Request& req, Response& res, Json::Value& root, const std::string& name, std::string& value) {
    if (__check_form_param_str(req, res, root, name, value)) {
        util::conv::to_upper(value);
        return true;
    }
    return false;
}

bool __check_json_param_str_lower(Request& req, Response& res, Json::Value& root, const std::string& name, std::string& value) {
    if (__check_json_param_str(req, res, root, name, value)) {
        util::conv::to_lower(value);
        return true;
    }
    return false;
}

bool __check_json_param_str_upper(Request& req, Response& res, Json::Value& root, const std::string& name, std::string& value) {
    if (__check_json_param_str(req, res, root, name, value)) {
        util::conv::to_upper(value);
        return true;
    }
    return false;
}

bool __check_json_param_str_lower_array(Request& req, Response& res, Json::Value& root, const std::string& name, std::vector<std::string>& value) {
    if (__check_json_param_str_array(req, res, root, name, value)) {
        for (auto& v : value) {
            util::conv::to_lower(v);
        }
        return true;
    }
    return false;
}

bool __check_json_param_str_upper_array(Request& req, Response& res, Json::Value& root, const std::string& name, std::vector<std::string>& value) {
    if (__check_json_param_str_array(req, res, root, name, value)) {
        for (auto& v : value) {
            util::conv::to_upper(v);
        }
        return true;
    }
    return false;
}

DEF_FUNC_CHECK_BODY_PARAM_EX(__check_body_param_str_lower_ex, __check_body_param_str_lower, __check_form_param_str_lower, __check_json_param_str_lower, std::string)
DEF_FUNC_CHECK_BODY_PARAM_EX(__check_body_param_str_upper_ex, __check_body_param_str_upper, __check_form_param_str_upper, __check_json_param_str_upper, std::string)
#pragma endregion


/*******************************************************
 * 
 * bool
 * 
*******************************************************/
#pragma region bool
static bool __check_bool_value(Request& req, Response& res, Json::Value& root, const std::string& name, std::string& str_value, bool& value) {
    util::conv::to_lower(util::trim(str_value));
    if (str_value == "true" || str_value == "1") {
        value = true;
        return true;
    }
    else if (str_value == "false" || str_value == "0") {
        value = false;
        return true;
    }
    else {
        RETURN_INVALID_PARAM(name) false;
    }
}

bool __check_url_param_bool(Request& req, Response& res, Json::Value& root, const std::string& name, bool& value) {
    value = false;
    bool exist;
    std::string str_value = req.GetUrlParam(name, &exist);
    if (exist) {
        return __check_bool_value(req, res, root, name, str_value, value);
    }
    RETURN_MISSING_PARAM(name) false;
}

bool __check_body_param_bool(Request& req, Response& res, Json::Value& root, const std::string& name, bool& value) {
    value = false;
    bool exist;
    std::string str_value = req.GetBodyParam(name, &exist);
    if (exist) {
        return __check_bool_value(req, res, root, name, str_value, value);
    }
    RETURN_MISSING_PARAM(name) false;
}

bool __check_form_param_bool(Request& req, Response& res, Json::Value& root, const std::string& name, bool& value) {
    value = false;
    auto ptr_value = req.GetFormParam(name);
    if (ptr_value) {
        std::string str_value = ptr_value->content().ToString();
        return __check_bool_value(req, res, root, name, str_value, value);
    }
    RETURN_MISSING_PARAM(name) false;
}

bool __check_json_param_bool(Request& req, Response& res, Json::Value& root, const std::string& name, bool& value) {
    const Json::Value& json_value = req.GetJsonParam(name);
    if (json_value.isBool()) {
        value = json_value.asBool();
        return true;
    }
    else if (json_value.isNull()) {
        RETURN_MISSING_PARAM(name) false;
    }
    else {
        RETURN_INVALID_PARAM(name) false;
    }
}

bool __check_json_param_bool_array(Request& req, Response& res, Json::Value& root, const std::string& name, std::vector<int32_t>& value) {
    const Json::Value& array_node = req.GetJsonParam(name);
    if (array_node.isArray()) {
        unsigned int size = array_node.size();
        value.reserve(size);
        for (unsigned int i = 0; i < size; ++i) {
            if (array_node[i].isBool()) {
                value.push_back((int)array_node[i].asBool());
            }
            else {
                RETURN_INVALID_PARAM(name) false;
            }
        }
        return true;
    }
    else if (array_node.isNull()) {
        RETURN_MISSING_PARAM(name) false;
    }
    else {
        RETURN_INVALID_PARAM(name) false;
    }
}

DEF_FUNC_CHECK_BODY_PARAM_EX(__check_body_param_bool_ex, __check_body_param_bool, __check_form_param_bool, __check_json_param_bool, bool)
#pragma endregion


/*******************************************************
 * 
 * number
 * 
*******************************************************/
#pragma region number
template<typename T>
bool __check_url_param_number(Request& req, Response& res, Json::Value& root, const std::string& name, T& value) {
    bool exist;
    const std::string& str_value = req.GetUrlParam(name, &exist);
    if (exist) {
        if (util::conv::convert_number(str_value, &value)){
            return true;
        }
        RETURN_INVALID_PARAM(name) false;
    }
    RETURN_MISSING_PARAM(name) false;
}

template<typename T>
bool __check_body_param_number(Request& req, Response& res, Json::Value& root, const std::string& name, T& value) {
    bool exist;
    const std::string& str_value = req.GetBodyParam(name, &exist);
    if (exist) {
        if (util::conv::convert_number(str_value, &value)){
            return true;
        }
        RETURN_INVALID_PARAM(name) false;
    }
    RETURN_MISSING_PARAM(name) false;
}

template<typename T>
bool __check_form_param_number(Request& req, Response& res, Json::Value& root, const std::string& name, T& value) {
    auto ptr_value = req.GetFormParam(name);
    if (ptr_value) {
        std::string str_value = ptr_value->content().ToString();
        if (util::conv::convert_number(str_value, &value)){
            return true;
        }
        RETURN_INVALID_PARAM(name) false;
    }
    RETURN_MISSING_PARAM(name) false;
}

template<typename T>
bool __check_json_param_number(Request& req, Response& res, Json::Value& root, const std::string& name, T& value) {
    const Json::Value& json_value = req.GetJsonParam(name);
    if (json_value.is<T>()) {
        value = json_value.as<T>();
        return true;
    }
    else if (json_value.isString()) {
        std::string str_value = json_value.asString();
        if (util::conv::convert_number(str_value, &value)){
            return true;
        }
        RETURN_INVALID_PARAM(name) false;
    }
    else if (json_value.isNull()) {
        RETURN_MISSING_PARAM(name) false;
    }
    else {
        RETURN_INVALID_PARAM(name) false;
    }
}

template<typename T>
bool __check_json_param_number_array(Request& req, Response& res, Json::Value& root, const std::string& name, std::vector<T>& value) {
    const Json::Value& array_node = req.GetJsonParam(name);
    if (array_node.isArray()) {
        unsigned int size = array_node.size();
        value.reserve(size);
        for (unsigned int i = 0; i < size; ++i) {
            if (array_node[i].is<T>()) {
                value.push_back(array_node[i].as<T>());
            }
            else if (array_node[i].isString()) {
                std::string str_value = array_node[i].asString();
                T num;
                if (util::conv::convert_number(str_value, &num)){
                    value.push_back(num);
                }
                else {
                    RETURN_INVALID_PARAM(name) false;
                }
            }
            else {
                RETURN_INVALID_PARAM(name) false;
            }
        }
        return true;
    }
    else if (array_node.isNull()) {
        RETURN_MISSING_PARAM(name) false;
    }
    else {
        RETURN_INVALID_PARAM(name) false;
    }
}
#pragma endregion


/*******************************************************
 * 
 * int32_t
 * 
*******************************************************/
#pragma region int32_t
bool __check_url_param_int(Request& req, Response& res, Json::Value& root, const std::string& name, int32_t& value) {
    return __check_url_param_number(req, res, root, name, value);
}

bool __check_body_param_int(Request& req, Response& res, Json::Value& root, const std::string& name, int32_t& value) {
    return __check_body_param_number(req, res, root, name, value);
}

bool __check_form_param_int(Request& req, Response& res, Json::Value& root, const std::string& name, int32_t& value) {
    return __check_form_param_number(req, res, root, name, value);
}

bool __check_json_param_int(Request& req, Response& res, Json::Value& root, const std::string& name, int32_t& value) {
    return __check_json_param_number(req, res, root, name, value);
}

bool __check_json_param_int_array(Request& req, Response& res, Json::Value& root, const std::string& name, std::vector<int32_t>& value) {
    return __check_json_param_number_array(req, res, root, name, value);
}

DEF_FUNC_CHECK_BODY_PARAM_EX(__check_body_param_int_ex, __check_body_param_int, __check_form_param_int, __check_json_param_int, int32_t)
#pragma endregion int32_t


/*******************************************************
 * 
 * uint32_t
 * 
*******************************************************/
#pragma region uint32_t
bool __check_url_param_uint(Request& req, Response& res, Json::Value& root, const std::string& name, uint32_t& value) {
    return __check_url_param_number(req, res, root, name, value);
}

bool __check_body_param_uint(Request& req, Response& res, Json::Value& root, const std::string& name, uint32_t& value) {
    return __check_body_param_number(req, res, root, name, value);
}

bool __check_form_param_uint(Request& req, Response& res, Json::Value& root, const std::string& name, uint32_t& value) {
    return __check_form_param_number(req, res, root, name, value);
}

bool __check_json_param_uint(Request& req, Response& res, Json::Value& root, const std::string& name, uint32_t& value) {
    return __check_json_param_number(req, res, root, name, value);
}

bool __check_json_param_uint_array(Request& req, Response& res, Json::Value& root, const std::string& name, std::vector<uint32_t>& value) {
    return __check_json_param_number_array(req, res, root, name, value);
}

DEF_FUNC_CHECK_BODY_PARAM_EX(__check_body_param_uint_ex, __check_body_param_uint, __check_form_param_uint, __check_json_param_uint, uint32_t)
#pragma endregion uint32_t


/*******************************************************
 * 
 * int64_t
 * 
*******************************************************/
#pragma region int64_t
bool __check_url_param_int64(Request& req, Response& res, Json::Value& root, const std::string& name, int64_t& value) {
    return __check_url_param_number(req, res, root, name, value);
}

bool __check_body_param_int64(Request& req, Response& res, Json::Value& root, const std::string& name, int64_t& value) {
    return __check_body_param_number(req, res, root, name, value);
}

bool __check_form_param_int64(Request& req, Response& res, Json::Value& root, const std::string& name, int64_t& value) {
    return __check_form_param_number(req, res, root, name, value);
}

bool __check_json_param_int64(Request& req, Response& res, Json::Value& root, const std::string& name, int64_t& value) {
    return __check_json_param_number(req, res, root, name, value);
}

bool __check_json_param_int64_array(Request& req, Response& res, Json::Value& root, const std::string& name, std::vector<int64_t>& value) {
    return __check_json_param_number_array(req, res, root, name, value);
}

DEF_FUNC_CHECK_BODY_PARAM_EX(__check_body_param_int64_ex, __check_body_param_int64, __check_form_param_int64, __check_json_param_int64, int64_t)
#pragma endregion int64_t


/*******************************************************
 * 
 * uint64_t
 * 
*******************************************************/
#pragma region uint64_t
bool __check_url_param_uint64(Request& req, Response& res, Json::Value& root, const std::string& name, uint64_t& value) {
    return __check_url_param_number(req, res, root, name, value);
}

bool __check_body_param_uint64(Request& req, Response& res, Json::Value& root, const std::string& name, uint64_t& value) {
    return __check_body_param_number(req, res, root, name, value);
}

bool __check_form_param_uint64(Request& req, Response& res, Json::Value& root, const std::string& name, uint64_t& value) {
    return __check_form_param_number(req, res, root, name, value);
}

bool __check_json_param_uint64(Request& req, Response& res, Json::Value& root, const std::string& name, uint64_t& value) {
    return __check_json_param_number(req, res, root, name, value);
}

bool __check_json_param_uint64_array(Request& req, Response& res, Json::Value& root, const std::string& name, std::vector<uint64_t>& value) {
    return __check_json_param_number_array(req, res, root, name, value);
}

DEF_FUNC_CHECK_BODY_PARAM_EX(__check_body_param_uint64_ex, __check_body_param_uint64, __check_form_param_uint64, __check_json_param_uint64, uint64_t)
#pragma endregion uint64_t


/*******************************************************
 * 
 * double
 * 
*******************************************************/
#pragma region double
bool __check_url_param_double(Request& req, Response& res, Json::Value& root, const std::string& name, double& value) {
    return __check_url_param_number(req, res, root, name, value);
}

bool __check_body_param_double(Request& req, Response& res, Json::Value& root, const std::string& name, double& value) {
    return __check_body_param_number(req, res, root, name, value);
}

bool __check_form_param_double(Request& req, Response& res, Json::Value& root, const std::string& name, double& value) {
    return __check_form_param_number(req, res, root, name, value);
}

bool __check_json_param_double(Request& req, Response& res, Json::Value& root, const std::string& name, double& value) {
    return __check_json_param_number(req, res, root, name, value);
}

bool __check_json_param_double_array(Request& req, Response& res, Json::Value& root, const std::string& name, std::vector<double>& value) {
    return __check_json_param_number_array(req, res, root, name, value);
}

DEF_FUNC_CHECK_BODY_PARAM_EX(__check_body_param_double_ex, __check_body_param_double, __check_form_param_double, __check_json_param_double, double)
#pragma endregion double

} // namespace helper
} // namespace server
} // namespace ic
