#ifndef IC_SERVER_HELPER_DTO_H_
#define IC_SERVER_HELPER_DTO_H_
#include "helper.h"

#define DTO_IN     bool Deserialize(const Json::Value& json)
#define DTO_OUT    Json::Value Serialize() const
#define DTO_IN_OUT DTO_IN; DTO_OUT

#define MAKE_DTO(type_name, var_name) \
    type_name var_name;\
    if (!var_name.Deserialize(req.json_params())) {\
        RETURN_INVALID_PARAM_MSG("Invalid request param");\
    }

#define MAKE_DTO_ARRAY(type_name, var_name) \
    std::vector<type_name> var_name;\
    {\
        const Json::Value& json_params = req.json_params();\
        if (json_params.isArray()) {\
            unsigned int size = json_params.size();\
            var_name.resize(size);\
            for (unsigned int i = 0; i < size; ++i) {\
                if (!var_name[i].Deserialize(json_params[i])) {\
                    RETURN_INVALID_PARAM_MSG("Invalid request param");\
                }\
            }\
        }\
        else {\
            RETURN_INVALID_PARAM_MSG("Invalid request param");\
        }\
    }

#endif // IC_SERVER_HELPER_DTO_H_
