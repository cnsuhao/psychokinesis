function message_box(title, msg) {
	$.Dialog({
        overlay: true,
        shadow: true,
        flat: true,
        draggable: true,
        title: title,
        content: '',
		width: 300,
		height: 20,
        padding: 10,
        onShow: function(_dialog){
			var content = '<div id="message_box">' +
						  '<span>' + msg + '</span><br/><br/>' +
						  '<button class="button" type="button" onclick="$.Dialog.close()">确定</button>' +
						  '</div>';

			$.Dialog.content(content);
        }
    });
}

function convert_error_string(error_code) {
	switch (error_code) {
	case 1:
		return '有些数据不合法哦！请再重新检查一下';
	case 10:
		return '服务器出错了！请稍后再试。';
	case 20:
		return '操作太频繁了！慢慢来，别着急。';
	default:
		return '操作失败了！请稍后再试。';
	}
}