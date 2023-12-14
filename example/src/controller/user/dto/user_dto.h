#pragma once
#include <server/helper/dto.h>
#include "manager/user_manager.h"

namespace dto {
namespace user {

struct LoginDto {
    DTO_IN;

    /**
     * @brief 用户ID.
     * @required
     */
    std::string uid;

    /**
     * @brief 密码.
     * @required
     * @alias password
     */
    std::string password_md5;
};

struct UpdateInfoDto {
    DTO_IN;

    /**
     * @brief 性别.
     */
    int gender;

    /**
     * @brief 省份.
     */
    int province;

    /**
     * @brief 职位.
     */
    int job;

    /**
     * @brief 头像.
     */
    std::string avatar;

    /**
     * @brief 昵称.
     */
    std::string nickname;

    /**
     * @brief 个性签名.
     */
    std::string sign;

    void SwapToUser(User& user) {
        user.gender = gender;
        user.province = province;
        user.job = job;
        user.avatar.swap(avatar);
        user.nickname.swap(nickname);
        user.sign.swap(sign);
    }
};

} // namespace user
} // namespace dto
