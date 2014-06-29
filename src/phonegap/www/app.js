var pause_time = 0;

var app = {
    // Application Constructor
    initialize: function() {
		this.bindEvents();
    },
    // Bind Event Listeners
    //
    // Bind any events that are required on startup. Common events are:
    // 'load', 'deviceready', 'offline', and 'online'.
    bindEvents: function() {
        document.addEventListener('deviceready', this.onDeviceReady, false);   // 准备就绪事件
		document.addEventListener("pause", this.onPause, false);               // 程序放入后台事件
		document.addEventListener("resume", this.onResume, false);             // 程序恢复到前台事件
		document.addEventListener("backbutton", this.onBackKeyDown, false);    // 后退事件
		
    },
	
    
    onDeviceReady: function() {
		var account = window.localStorage.getItem("account");
		var password = window.localStorage.getItem("password");
		
		if (account && password) {
			$("#signin_account").val(account);
			$("#signin_password").val(password);
			start_login();
		}
    },
    
	onPause: function() {
		in_background = true;
		pause_time = new Date();
	},
	
	onResume: function() {
		in_background = false;
		
		if (mobile_online && (new Date() - pause_time) / 1000 > 120) {        // 后台运行超过2分钟自动注销
			pause_time = 0;
			mobile_online = false;
		
			if (communication) {
				communication.disconnect();
			}
		
			$.mobile.changePage("#signin_page", {transition: "flip", reverse: "true"});
		}	
	},
	
	onBackKeyDown: function() {
		if (child_browser)
			child_browser.close();
		else
			navigator.app.exitApp();
	}
};