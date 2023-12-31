// Generated by script server-assistant.py
// DO NOT EDIT
#include "user_dto.h"

bool dto::user::LoginDto::Deserialize(const Json::Value& json) {
    if (!json.isObject()) { return false; }
    const Json::Value& _uid_node_temp_ = json["uid"]; if (_uid_node_temp_.is<std::string>()) { uid = _uid_node_temp_.as<std::string>(); } else { return false; }
    const Json::Value& _password_md5_node_temp_ = json["password"]; if (_password_md5_node_temp_.is<std::string>()) { password_md5 = _password_md5_node_temp_.as<std::string>(); } else { return false; }
    return true;
}

bool dto::user::UpdateInfoDto::Deserialize(const Json::Value& json) {
    if (!json.isObject()) { return false; }
    const Json::Value& _gender_node_temp_ = json["gender"]; if (_gender_node_temp_.is<int>()) { gender = _gender_node_temp_.as<int>(); } else { return false; }
    const Json::Value& _province_node_temp_ = json["province"]; if (_province_node_temp_.is<int>()) { province = _province_node_temp_.as<int>(); } else { return false; }
    const Json::Value& _job_node_temp_ = json["job"]; if (_job_node_temp_.is<int>()) { job = _job_node_temp_.as<int>(); } else { return false; }
    const Json::Value& _avatar_node_temp_ = json["avatar"]; if (_avatar_node_temp_.is<std::string>()) { avatar = _avatar_node_temp_.as<std::string>(); } else { return false; }
    const Json::Value& _nickname_node_temp_ = json["nickname"]; if (_nickname_node_temp_.is<std::string>()) { nickname = _nickname_node_temp_.as<std::string>(); } else { return false; }
    const Json::Value& _sign_node_temp_ = json["sign"]; if (_sign_node_temp_.is<std::string>()) { sign = _sign_node_temp_.as<std::string>(); } else { return false; }
    return true;
}
