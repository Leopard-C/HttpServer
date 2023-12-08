
function LayerAlertError(msg, title) {
  title = title || '错误';
  layer.alert(msg, { icon: 2, title: title });
}

function LayerAlertSuccess(msg, title) {
  title = title || '成功';
  layer.alert(msg, { icon: 1, title: title });
}

function LayerAlertInfo(msg, title) {
  title = title || '信息';
  layer.alert(msg, { title: title });
}

function LayerMsgInfo(msg, time) {
  time = time || 2000;
  layer.msg(msg, {
    time: time,
    offset: '20px'
  });
}

function LayerMsgError(msg, time) {
  time = time || 2000;
  layer.msg(msg, {
    time: time,
    icon: 2,
    offset: '20px'
  });
}

function LayerConfirm(content, success) {
  layer.confirm(content, {
    title: '提示',
    icon: 3,
    shade: false,
    btn: ['确定', '取消']
  }, function (index) {
    layer.close(index);
    success && success();
  })
}
