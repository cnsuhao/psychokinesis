var communication = null;       
var ping_timer = null;
var progress_objs = {};

$(document).ready(function() {
	$("#signin_button").click(function(){
		start_login();
	});
	
	$("#add_resource_button").click(function(){
		var rs_url = $("#resource_location").val();
		if (rs_url == '' || !rs_url.match(/^(http[s]?|ftp|magnet):/i))
			return;
		
		loading_message_show('请稍等......');
		
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
		
		communication.send_message(send_json, 10*1000, function(back_msg){
			if (back_msg.ret_code == 0 && back_msg.list[0].result == 'success')
			{
				var rs_number = back_msg.list[0].gid;
				
				$("#cancel_resource_button").click();
		
				resource_item_create(rs_number);
			}
			else
			{
				alert('添加资源失败，请确认电脑上的Psychokinesis已启用');
			}
			
			loading_message_hide();
			$("#resource_location").val('');
		});
	});
});

function on_connect(status)
{
	if (status == Strophe.Status.CONNECTED)
	{
		loading_message_hide();
		$.mobile.changePage("#resource_page", {transition: "flip"});
		
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
		loading_message_show("网络中断，正在重新连接......");
	}
	else if (status == Strophe.Status.DISCONNECTED)
	{
		if (ping_timer)
		{
			window.clearInterval(ping_timer);
			ping_timer = null;
		}
		
		// 自动重连
		communication.reconnect(on_connect);
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
	$("body").append('<div class="freezeWindow"/>');
	$.mobile.loadingMessage = msg;             // 显示的文字
	$.mobile.loadingMessageTextVisible = true; // 是否显示文字
	$.mobile.loadingMessageTheme = 'a';        // 加载器主题样式a-e
	$.mobile.showPageLoadingMsg();             // 显示加载器
}

function loading_message_hide()
{
	$(".freezeWindow").remove();
	$.mobile.hidePageLoadingMsg();
}

function resource_item_create(item_id)
{
	var list = $("<li id='resource_" + item_id + "'>" + 
				 "<img src='images/movie.png' />" +
				 "<h3 id='resource_name_" + item_id + "' class='resource_name'>" + item_id + "</h3>" +
				 "<p>" +
					"<div id='resource_state_" + item_id + "' class='resource_state'>正在连接</div>" +
					"<div id='resource_progressbar_" + item_id + "' class='resource_progressbar'/>" + 
				 "</p></li>");
					
	$("#resource_list").append(list);
	$('ul').listview('refresh');
				
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
	2*1000);
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
		
	communication = Communication.create();
	communication.connect(account, pwd, on_connect);
	
	window.localStorage.setItem("account", account);
	window.localStorage.setItem("password", pwd);
}