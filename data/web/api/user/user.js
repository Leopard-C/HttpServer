
/**
 * @brief 用户登录
 */
function API_UserLogin(uid, password_md5, success_0, success_not_0, error) {
  let body = {
    uid: uid,
    password: password_md5
  };
  _api_post_json('api/User/Login', body, success_0, success_not_0, error);
}

/**
 * @brief 获取当前登录用户信息
 */
function API_GetUserInfo(success_0, success_not_0, error) {
  _api_get('api/User/GetInfo', success_0, success_not_0, error);
}

/**
 * @brief 更新当前登录用户信息
 */
function API_UpdateUserInfo(user_info, success_0, success_not_0, error) {
  _api_post_json('api/User/UpdateInfo', user_info, success_0, success_not_0, error);
}

function API_UploadUserAvatar(fileObj, success_0, success_not_0, error) {
  let formData = new FormData();
  formData.append('image', fileObj);
  _api_post_form('api/User/UploadAvatar', formData, success_0, success_not_0, error);
}
