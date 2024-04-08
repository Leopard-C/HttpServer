
window.APP_BASE_URI = GetAppBaseUri();

function GetAppBaseUri() {
  let path = window.location.pathname;
  path = path.substring(0, path.lastIndexOf('/'));
  path = path.substring(0, path.lastIndexOf('/'));
  return path + '/';
}

function U(path) {
  if (path.length > 0 && path[0] == '/') {
    path = path.substring(1);
  }
  return window.APP_BASE_URI + path;
}

function _check_response(res, success_0, success_not_0) {
  if (!('code' in res)) {
    return;
  }
  if (res.code == 0) {
    success_0 && success_0(res.data);
  }
  else {
    if (res.code == 1001) {  /* AUTH FAILED */
      window.location.href = U('web/login.html');
      return;
    }
    if (success_not_0) {
      success_not_0(res.code, res.msg, res.data);
    }
    else {
      debugger
      let content = `code: ${res.code}<br>msg: ${res.msg}`;
      LayerAlertError(content);
    }
  }
}

function _check_error(textStatus, error) {
  console.log(textStatus);
  if (error) {
    error(textStatus);
  }
  else {
    LayerAlertError(textStatus);
  }
}

/**
 * @brief 发送API Get请求.
 */
function __api_get(url, success, error) {
  $.ajax({
    url: U(url),
    type: 'GET',
    dataType: 'json',
    timeout: 5000,
    success: function(res) {
      success && success(res);
    },
    error(jqXHR, textStatus, errorThron) {
      _check_error(textStatus, error);
    }
  })
}

/**
 * @brief 发送API Get请求.
 */
function _api_get(url, success_0, success_not_0, error) {
  return __api_get(url, function(res){
    _check_response(res, success_0, success_not_0);
  }, error);
}

/**
 * @brief 发送API Post请求(application/json).
 */
function __api_post_json(url, json_body, success, error) {
  $.ajax({
    url: U(url),
    type: 'POST',
    contentType: 'application/json',
    data: JSON.stringify(json_body),
    dataType: 'json',
    timeout: 5000,
    success: function(res) {
      success && success(res);
    },
    error(jqXHR, textStatus, errorThron) {
      _check_error(textStatus, error);
    }
  })
}

function _api_post_json(url, json_body, success_0, success_not_0, error) {
  return __api_post_json(url, json_body, function(res){
    _check_response(res, success_0, success_not_0);
  }, error);
}

/**
 * @brief 发送API Post请求(multipart/form-data).
 */
function __api_post_form(url, form_data, success, error) {
  $.ajax({
    url: U(url),
    type: 'POST',
    data: form_data,
    processData: false,
    contentType: false,
    timeout: 5000,
    success: function(res) {
      success && success(res);
    },
    error(jqXHR, textStatus, errorThron) {
      _check_error(textStatus, error);
    }
  });
}

function _api_post_form(url, form_data, success_0, success_not_0, error) {
  return __api_post_form(url, form_data, function(res){
    _check_response(res, success_0, success_not_0);
  }, error);
}
