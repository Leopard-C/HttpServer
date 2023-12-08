#pragma once
#include <map>
#include <mutex>
#include <string>
#include <jsoncpp/json/value.h>

class User {
public:
    int gender = 0;  /* 0保密，1男，2女 */
    int job = 3;
    int province = 19;
    std::string uid;
    std::string password;  /* sha256 hash */
    std::string nickname;
    std::string avatar;
    std::string sign;
public:
    void FromJson(const Json::Value& node);
    Json::Value ToJson() const;
};

class UserManager {
public:
    UserManager();
    ~UserManager();

    /**
     * @brief 初始化.
     * 
     * @details 读取公私钥.
     */
    bool Init();

    /**
     * @brief 用户登录.
     * 
     * @param[in]  uid 用户ID
     * @param[in]  password_sha256 密码，经过SHA256哈希计算
     * @param[in]  max_age 生成的token有效期，单位:秒
     * @param[out] token 生成的Json Web Token
     */
    bool Login(const std::string& uid, const std::string& password_sha256, unsigned int max_age, std::string* token);

    /**
     * @brief 认证用户的token.
     */
    bool Auth(const std::string& token, User* user = nullptr);

    /**
     * @brief 获取用户信息
     */
    bool GetUser(const std::string& uid, User* user);

    /**
     * @brief 更新用户信息
     */
    bool UpdateUserInfo(const User& user);

private:
    bool LoadUserInfoFromFile();
    bool WriteUserInfoToFile();

    /**
     * @brief 生成token.
     */
    std::string CreateToken();

    /**
     * @brief 检查token是否过期，过期则删除.
     */
    void ValidateTokens();

private:
    std::string user_data_file_;

    std::mutex mutex_;
    time_t check_token_last_time_ = 0;

    // <uid, User*>
    std::map<std::string, User*> users_;
    // <token, <uid, expires_at>>
    std::map<std::string, std::pair<std::string, time_t>> tokens_;
};
