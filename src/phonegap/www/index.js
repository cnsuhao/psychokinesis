var communication = null;       
var ping_timer = null;
var progress_objs = {};
var child_browser = null;
var in_background = false;
var pc_online = false;
var mobile_online = false;
var reconnect_timer = 0;

$(document).on("pageinit", function() {
	// fix the new iOS 7 transparent status bar
	if (navigator.userAgent.match(/(iPad.*|iPhone.*|iPod.*);.*CPU.*OS 7_\d/i)) {
		$("body").addClass("ios7");
		$("body").append('');
	}
});

$(document).ready(function() {
	$("#signin_button").click(function(){
		start_login();
	});
	
	$("#jump_signup_button").click(function(){
		$.mobile.changePage("#signup_page", {transition: "slide"});
	});
	
	$("#signup_button").click(function(){
		start_signup();
	});
	
	$(".logout_buttons").click(function(){
		mobile_online = false;
		
		if (communication) {
			communication.disconnect();
		}
		
		$.mobile.changePage("#signin_page", {transition: "flip", reverse: "true"});
	});
	
	$("#add_resource_button").click(function(){
		var rs_url = $("#resource_location").val();
		if (rs_url == '' || !rs_url.match(/^(http[s]?|ftp|magnet):/i))
		{
			$('#error_dialog_content').html('无效的资源地址！请再次确认资源地址。');
			$.mobile.changePage("#error_dialog", {transition: "pop"});
			return;
		}
		
		add_resource();
	});
	
	fetch_download_websites();
});

function on_connect(status)
{
	if (status == Strophe.Status.CONNECTED)
	{
		mobile_online = true;
		
		loading_message_hide();
		
		if (pc_online)
			$.mobile.changePage("#resource_page", {transition: "flip"});
		else
			$.mobile.changePage("#waiting_pc_page", {transition: "flip"});
		
		communication.listen_presence(on_presence);
		communication.listen_message(on_message);
		communication.presence();
		
		var response_time = new Date();
		
		ping_timer = window.setInterval(function()
						   {
								var diff = Math.round((new Date() - response_time) / 1000);
								if (diff == 31)              // 只执行一次
								{
									$("#resource_list").find("li").each(function () {
										$(this).find(".resource_state").html('无法获取当前的下载状态');
									});
								}
								
								if (in_background)
								{
									response_time = new Date();
									return;
								}
								
								var send_json = {method: 'download',
						 						 opr: 'list'};
								
								communication.send_message(send_json, null, function(back_msg){
									if (back_msg.ret_code == 0)
									{
										for (var i = 0, len = back_msg.list.length; i < len; ++i)
										{
											var rs_number = back_msg.list[i].gid;
											
											if ($("#resource_" + rs_number).length)
											{
												var speed = bytes_transform(back_msg.list[i].download_speed) + '/s';
												var size = bytes_transform(back_msg.list[i].total_length);
												if (back_msg.list[i].total_length > 0)
													var progress = Math.round(back_msg.list[i].completed_length * 100 / back_msg.list[i].total_length);
												else
													var progress = 0;
												
												$('#resource_state_' + rs_number).html(size + ' ' + speed);
												progress_objs['resource_progressbar_' + rs_number].setValue(progress);
											}
											else
											{
												resource_item_create(rs_number);
											}
										}
									}
									
									response_time = new Date();
								});
						   },
							5*1000);
	}
	else if (status == Strophe.Status.AUTHFAIL)
	{
		loading_message_hide();
		$('#error_dialog_content').html('账号或密码错误！');
		$.mobile.changePage("#error_dialog", {transition: "pop"});
		
		window.localStorage.removeItem("account");
		window.localStorage.removeItem("password");
	}
	else if (status == Strophe.Status.ERROR || 
			 status == Strophe.Status.CONNFAIL)
	{
		if (mobile_online) {
			if (++reconnect_timer > 3) {
				mobile_online = false;
				
				window.localStorage.removeItem("account");
				window.localStorage.removeItem("password");

				alert('服务器无响应！请稍后再试。');
				window.location = "index.html";
			}
			else
				loading_message_show("网络中断，正在重新连接......");
		}
	}
	else if (status == Strophe.Status.DISCONNECTED)
	{
		if (ping_timer)
		{
			window.clearInterval(ping_timer);
			ping_timer = null;
		}
		
		if (mobile_online && communication && reconnect_timer <= 3)
		{
			// 5秒后自动重连
			setTimeout("communication.reconnect(on_connect)", 5000);
		}
	}
	else if (status == 999)                // 连接超时
	{
		if (communication) {
			communication.disconnect();
		}
		mobile_online = false;
		loading_message_hide();
		
		$.mobile.changePage("#signin_page", {transition: "flip", reverse: "true"});
		
		$('#error_dialog_content').html('服务器未响应！请稍后再试。');
		$.mobile.changePage("#error_dialog", {transition: "pop"});
	}
}

function on_presence(status)
{
	if (status == 'available')
	{
		$.mobile.changePage("#resource_page", {transition: "slideup", reverse: "true"});
		pc_online = true;
	}
	else if (status == 'unavailable')
	{
		$.mobile.changePage("#waiting_pc_page", {transition: "slideup"});
		pc_online = false;
		
		$("#resource_list").empty();
		$("#task_promote").show();
	}
}

function on_message(from, message)
{
	if (message.info == 'event')
	{
		var rs_number = message.gid;
		
		if ($("#resource_" + rs_number).length)
		{
			if (message.event_type == 'START')
				var state = '0B 0KB/s';
			else if (message.event_type == 'PAUSE')
				var state = '暂停';
			else if (message.event_type == 'STOP')
				var state = '停止';
			else if (message.event_type == 'COMPLETE')
			{
				var state = '已完成';
				
				progress_objs['resource_progressbar_' + rs_number].setValue(100);
			}
			else if (message.event_type == 'ERROR')
				var state = '无法获取资源';
				
			if (state)
				$('#resource_state_' + rs_number).html(state);
		}
	}
}

function loading_message_show(msg)
{
	if ($(".freezeWindow").length == 0)
		$("body").append('<div class="freezeWindow"/>');

		$.mobile.loading( 'show', {
			text: msg,
			textVisible: true,
		});
}

function loading_message_hide()
{
	$(".freezeWindow").remove();
	$.mobile.loading('hide');
}

function resource_item_create(item_id)
{
	$("#task_promote").hide();
	
	var list = $("<li id='resource_" + item_id + "' class='resource_item' >" + 
				 "<img src='images/file.png' />" +
				 "<h2 id='resource_name_" + item_id + "' class='resource_name'>" + item_id + "</h2>" +
				 "<p>" +
					"<div id='resource_state_" + item_id + "' class='resource_state'>正在连接</div>" +
					"<div id='resource_progressbar_" + item_id + "' class='resource_progressbar'/>" + 
				 "</p></li>");
					
	$("#resource_list").append(list);
	$("#resource_list").listview('refresh');
				
	progress_objs['resource_progressbar_' + item_id] = jQMProgressBar('resource_progressbar_' + item_id)
				.setOuterTheme('b')
				.setInnerTheme('e')
				.isMini(true)
				.setMax(100)
				.setStartFrom(0)
				.showCounter(true)
				.build();
	
	// 由于BT资源并不能立即获得资源名称，所以10秒获取一次资源名称，直到名称不为空为止		
	var fetch_item_detail_timer = window.setInterval(function()
	{
		var send_json = {method: 'download',
						 opr: 'list_item',
						 gid: item_id};
						 
		communication.send_message(send_json, null, function(back_msg)
		{
			if (back_msg.ret_code == 0)
			{
				if (back_msg.name != '')
				{
					$("#resource_name_" + item_id).html(back_msg.name);
					window.clearInterval(fetch_item_detail_timer);
				}
			}
			else                                                  // 资源被删的情况
				window.clearInterval(fetch_item_detail_timer);
		});
	},
	10*1000);
}

function bytes_transform(bytes)
{
	var count = 0;
	while (bytes > 1024)
	{
		bytes /= 1024;
		++count;
	}
	
	var transform = ['B', 'KB', 'MB', 'GB', 'TB'];
	if (count > 0)
		bytes = bytes.toFixed(1);
	
	return bytes + transform[count];
}

function start_login()
{
	account = $("#signin_account").val();
	pwd = $("#signin_password").val();
	if (account == '' || pwd == '')
	{
		$('#error_dialog_content').html('账号或密码为空！');
		$.mobile.changePage("#error_dialog", {transition: "pop"});
		return;
	}
		
	loading_message_show('登录中......');
	
	reconnect_timer = 0;
	pc_online = false;
	
	if (!communication)
		communication = Communication.create();
		
	communication.connect(account, pwd, on_connect);
	
	window.localStorage.setItem("account", account);
	window.localStorage.setItem("password", pwd);
}

function add_resource()
{		
	loading_message_show('请稍等......');
		
	var rs_url = $("#resource_location").val();
	var send_json = {method: 'download',
					 opr: 'add',
					 list: [
					 {
						uris: [
							rs_url
						],
						options: [
						],
						position: -1
					 }
					 ]
	};
		
	communication.send_message(send_json, 20*1000, function(back_msg){
		if (back_msg.ret_code == 0 && back_msg.list[0].result == 'success')
		{
			var rs_number = back_msg.list[0].gid;
				
			$("#add_resource_dialog").dialog('close');
		
			resource_item_create(rs_number);
		}
		else
		{
			$('#error_dialog_content').html('添加资源失败，请稍后重试！');
			$.mobile.changePage("#error_dialog", {transition: "pop"});
		}
			
		loading_message_hide();
		$("#resource_location").val('');
	});
}


function child_browser_opening(event) {
	var url = event.url;
	
	if (url.match(/^(http[s]?|ftp).+?\.(exe|zip|rar|torrent)$/i) || url.match(/^magnet:/i)) {	
		$("#resource_location").val(url);
		add_resource();
		
		child_browser.close();
		child_browser = null;
	}
}

function child_browser_closed(event) {
	child_browser.removeEventListener('loadstart', child_browser_opening);
	child_browser.removeEventListener('exit', child_browser_closed);
	
	// 经测试此函数不一定会被调用，不要再后面添加处理逻辑
}

function open_child_browser(url) {
	child_browser = window.open(url, '_blank', 'location=yes');
	
	child_browser.addEventListener('loadstart', child_browser_opening);
	child_browser.addEventListener('exit', child_browser_closed);
}


function convert_error_string(error_code) {
	switch (error_code) {
	case 1:
		return '有些数据不合法哦！请再重新检查一下';
	case 10:
		return '服务器出错了！请稍后再试。';
	case 20:
		return '操作太频繁了！请稍后再试。';
	default:
		return '操作失败了！请稍后再试。';
	}
}

function start_signup() {
	var person = {account: $("#signup_account").val(),
				  password: $("#signup_password").val(),
				  email: $("#signup_email").val()};
	
	if (person.account == '' || person.password == '' ||
		person.account.length > 50 || person.password.length > 50 
		|| /['";&|\s]/.test(person.account) || /['";&|\s]/.test(person.password))
	{
		$('#error_dialog_content').html('账号或密码超过50字符或含有非法字符！');
		$.mobile.changePage("#error_dialog", {transition: "pop"});
		return;
	}
	
	var email_reg = /^([a-zA-Z0-9]+[_|\_|\.]?)*[a-zA-Z0-9]+@([a-zA-Z0-9]+[_|\_|\.]?)*[a-zA-Z0-9]+\.[a-zA-Z]{2,3}$/;
	if (person.email.length > 200 || !email_reg.test(person.email)) {
		$('#error_dialog_content').html('电子邮箱地址不合法！');
		$.mobile.changePage("#error_dialog", {transition: "pop"});
		return;
	}
	
	loading_message_show('注册中......');
	
	var try_time = 0;
	
	function try_again() {
		if (++try_time > 3) {
			loading_message_hide();
			
			$.mobile.changePage("#signin_page", {transition: "slide", reverse: "true"});
			
			$('#error_dialog_content').html('服务器无响应，请稍后重试！');
			$.mobile.changePage("#error_dialog", {transition: "pop"});
			
			return;
		}
			
		$.post("http://psychokinesis.me/nodejs/register-by-app", person)
		.done(function(data) {
		  	loading_message_hide();
			
			if (data.error_code) {
				$('#error_dialog_content').html(convert_error_string(data.error_code));
				$.mobile.changePage("#error_dialog", {transition: "pop"});
				return;
			}
	  
			$.mobile.changePage("#signin_page", {transition: "slide", reverse: "true"});
			
			$('#message_dialog_content').html("<h3>注册成功！</h3>" +
					"<p>您的账号：" + data.account + "</p>" +
					"<p>您的密码：" + data.password + "</p>");
			$.mobile.changePage("#message_dialog", {transition: "pop"});
			
			$("#signin_account").val(data.account);
			$("#signin_password").val(data.password);
		})
		.fail(function() {
			try_again();
		});
	}
	
	try_again();
}

function fetch_download_websites() {
	var try_time = 0;
	
	function try_again() {
		if (++try_time > 3)
			return;
			
		$.getJSON("http://psychokinesis.me/json/download-websites.json")
		.done(function(data) {
			if (data.ret_code != 0) {
				try_again();
				return;
			}
		  
		  	$("#website_list").empty();
			
			for (var i = 0, len = data.list.length; i < len; ++i)
			{	
				var list_head = $("<li data-role='list-divider'>" + data.list[i].type + "</li>");
				$("#website_list").append(list_head);
					
				var websites = data.list[i].websites;
					
				for (var j = 0, website_len = websites.length; j < website_len; ++j)
				{
					var list_item = $("<a href=\"javascript:void(0);\">" + websites[j].name + "</a>");
					
					list_item.click({url: websites[j].url}, function(event) {
						open_child_browser(event.data.url);
					});
					var li_item = $("<li></li>");
					li_item.append(list_item);
					
					$("#website_list").append(li_item);
				}
			}
				
			$("#website_list").listview('refresh');
		})
		.fail(function() {
			try_again();
		});
	}
	
	try_again();
}