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

var isMobile = {
	Android: function() {
		return navigator.userAgent.match(/Android/i);
	},
	iOS: function() {
		return navigator.userAgent.match(/iPhone|iPad|iPod/i);
	},
	Windows: function() {
		return navigator.userAgent.match(/IEMobile/i);
	}
};

function download_app(type, cb) {
	if (type == 'android') {
		if (cb)
			cb(true);
		window.location.href = '/Psychokinesis.apk';
	} else if (type == 'wp') {
		if (cb)
			cb(true);
		window.location.href = 'http://www.windowsphone.com/s?appid=00acc51c-ff96-419a-b0da-1736fd378f1b';
	} else if (type == 'ios') {
		if (cb)
			cb(true);
		window.location.href = 'https://itunes.apple.com/cn/app/psychokinesis/id898639199?mt=8';
	} else if (type == 'windows') {
		if (cb)
			cb(true);
		window.location.href = '/psychokinesis.exe';
	} else {
		if (cb)
			cb(false);
	}
}
