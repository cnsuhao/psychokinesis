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
	},
	
	onBackKeyDown: function() {
		navigator.app.exitApp();
	}
};