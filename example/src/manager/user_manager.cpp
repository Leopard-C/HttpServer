#include "user_manager.h"
#include <fstream>
#include <jsoncpp/json/json.h>
#include <log/logger.h>
#include <server/http_server.h>
#include <server/util/convert/convert_case.h>
#include <server/util/hash/md5.h>
#include <server/util/io.h>
#include <server/util/path.h>

void User::FromJson(const Json::Value& node) {
    gender = node["gender"].asInt();
    uid = node["uid"].asString();
    password = ic::server::util::conv::to_lower_copy(node["password"].asString());
    nickname = node["nickname"].asString();
    avatar = node["avatar"].asString();
    job = node["job"].asInt();
    province = node["province"].asInt();
    sign = node["sign"].asString();
}

Json::Value User::ToJson() const {
    Json::Value node;
    node["gender"] = gender;
    node["uid"] = uid;
    node["password"] = password;
    node["nickname"] = nickname;
    node["avatar"] = avatar;
    node["province"] = province;
    node["job"] = job;
    node["sign"] = sign;
    return node;
}

UserManager::UserManager() {
    std::string user_data_dir = ic::server::HttpServer::GetBinDir() + "../data/system/";
    ic::server::util::path::create_dir(user_data_dir, true);
    user_data_file_ = user_data_dir + "user.dat";
}

UserManager::~UserManager() {
    for (auto iter = users_.begin(); iter != users_.end(); ++iter) {
        delete iter->second;
    }
}

/**
 * @brief 初始化
 */
bool UserManager::Init() {
    if (!LoadUserInfoFromFile()) {
        return false;
    }
    return true;
}

bool UserManager::LoadUserInfoFromFile() {
    std::lock_guard<std::mutex> lck(mutex_);
    bool ok = false;
    do {
        bool is_admin_exist = false;
        std::ifstream ifs(user_data_file_);
        if (ifs) {
            Json::Reader reader;
            Json::Value root;
            if (!reader.parse(ifs, root, false) || !root.isArray()) {
                break;
            }
            for (unsigned int i = 0; i < root.size(); ++i) {
                User* user = new User();
                user->FromJson(root[i]);
                if (user->uid == "admin") {
                    is_admin_exist = true;
                }
                users_[user->uid] = user;
            }
            ifs.close();
        }

        if (!is_admin_exist) {
            User* admin = new User();
            admin->uid = "admin";
            admin->password = ic::server::util::hash::md5_lower("123456");
            admin->gender = 1;
            admin->nickname = "管理员";
            admin->province = 19;
            admin->job = 3;
            admin->sign = "凡是过往，皆为序章";
            admin->avatar = "99eaabaa1fa6af540f8c9e929fccdcfc.jpg";
            users_[admin->uid] = admin;
            if (!WriteUserInfoToFile()) {
                break;
            }
        }

        ok = true;
    } while (false);

    if (!ok) {
        LError("read file '{}' failed", user_data_file_);
        return false;
    }
    return true;
}

bool UserManager::WriteUserInfoToFile() {
    Json::Value root;
    for (auto iter = users_.begin(); iter != users_.end(); ++iter) {
        User* user = iter->second;
        root.append(user->ToJson());
    }
    Json::FastWriter fw;
    fw.emitUTF8();
    fw.omitEndingLineFeed();
    std::ofstream ofs(user_data_file_);
    if (!ofs) {
        LError("write file '{}' failed", user_data_file_);
        return false;
    }
    ofs << fw.write(root);
    return true;
}

/**
 * @brief 用户登录.
 * 
 * @param[in]  uid 用户ID
 * @param[in]  password_md5 密码，经过MD5哈希计算
 * @param[in]  max_age 生成的token有效期，单位:秒
 * @param[out] token 生成的token
 */
bool UserManager::Login(
    const std::string& uid,
    const std::string& password_md5,
    unsigned int max_age,
    std::string* token)
{
    std::lock_guard<std::mutex> lck(mutex_);
    ValidateTokens();
    auto iter = users_.find(uid);
    if (iter == users_.end() || iter->second->password != password_md5) {
        return false;
    }
    *token = CreateToken();
    time_t expires_at = time(NULL) + max_age;
    tokens_.emplace(*token, std::make_pair(uid, expires_at));
    return true;
}

/**
 * @brief 生成token.
 */
std::string UserManager::CreateToken() {
    std::string token(32, '\0');
    for (int i = 0; i < 32; ++i) {
        int tmp = rand() % 36;
        if (tmp < 10) {
            token[i] = tmp + '0';
        }
        else {
            token[i] = (tmp - 10) + 'a';
        }
    }
    return token;
}

/**
 * @brief 认证用户的token.
 */
bool UserManager::Auth(const std::string& token, User* user/* = nullptr*/) {
    std::lock_guard<std::mutex> lck(mutex_);
    ValidateTokens();
    auto iter1 = tokens_.find(token);
    if (iter1 == tokens_.end()) {
        return false;
    }
    auto iter2 = users_.find(iter1->second.first);
    if (iter2 == users_.end()) {
        return false;
    }
    if (user) {
        *user = *(iter2->second);
    }
    return true;
}

/**
 * @brief 获取用户信息
 */
bool UserManager::GetUser(const std::string& uid, User* user) {
    std::lock_guard<std::mutex> lck(mutex_);
    auto iter = users_.find(uid);
    if (iter == users_.end()) {
        return false;
    }
    *user = *(iter->second);
    return true;
}

/**
 * @brief 更新用户信息
 */
bool UserManager::UpdateUserInfo(const User& user) {
    std::lock_guard<std::mutex> lck(mutex_);
    auto iter = users_.find(user.uid);
    if (iter == users_.end()) {
        return false;
    }
    iter->second->avatar = user.avatar;
    iter->second->nickname = user.nickname;
    iter->second->gender = user.gender;
    iter->second->job = user.job;
    iter->second->sign = user.sign;
    iter->second->province = user.province;
    if (!WriteUserInfoToFile()) {
        return false;
    }
    return true;
}

/**
 * @brief 检查token是否过期，过期则删除.
 */
void UserManager::ValidateTokens() {
    /* 10s检测一次 */
    time_t now = time(NULL);
    if (now - check_token_last_time_ < 10) {
        return;
    }
    check_token_last_time_ = now;

    for (auto iter = tokens_.begin(); iter != tokens_.end();/* ++iter*/) {
        if (iter->second.second < now) {
            iter = tokens_.erase(iter);
        }
        else {
            ++iter;
        }
    }
}
