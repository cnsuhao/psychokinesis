var communication = null;       
var ping_timer = null;
var progress_objs = {};

$(document).ready(function() {
	$("#signin_button").click(function(){
		account = $("#signin_account").val();
		pwd = $("#signin_password").val();
		if (account == '' || pwd == '')
		{
			on_connect(Strophe.Status.AUTHFAIL);
			return;
		}
		
		loading_message_show('登录中......');
		
		communication = Communication.create();
		communication.connect(account, pwd, on_connect);
	});
	
	$("#add_resource_button").click(function(){
		var rs_url = $("#resource_location").val();
		if (rs_url == '')
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
		
		communication.send_message(send_json, function(back_msg){
			if (back_msg.ret_code == 0 && back_msg.list[0].result == 'success')
			{
				var rs_number = back_msg.list[0].gid;
				
				$("#cancel_resource_button").click();
		
				resource_item_create(rs_url, rs_number);
			}
			else
			{
				alert('添加资源失败');
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
									alert("PC客户端无响应");
									
									$("#resource_list").find("li").each(function () {
										$(this).find(".resource_speed").html('0MB 0KB/s');
									});
								}
								
								var send_json = {method: 'download',
						 						 opr: 'list'};
								
								communication.send_message(send_json, function(back_msg){
									if (back_msg.ret_code == 0)
									{
										for (var i = 0, len = back_msg.list.length; i < len; ++i)
										{
											var rs_number = back_msg.list[i].gid;
											
											if($("#resource_" + rs_number).length)
											{
												var speed = bytes_transform(back_msg.list[i].download_speed) + '/s';
												var size = bytes_transform(back_msg.list[i].total_length);
												var progress = Math.round(back_msg.list[i].completed_length * 100 / back_msg.list[i].total_length);
												
												$('#resource_speed_' + rs_number).html(size + ' ' + speed);
												progress_objs['resource_progressbar_' + rs_number].setValue(progress);
											}
											else
											{
												resource_item_create("", rs_number);
											}
										}
									}
									
									response_time = new Date();
								});
						   },
							1*1000);
	}
	else if (status == Strophe.Status.AUTHFAIL)
	{
		loading_message_hide();
		$.mobile.changePage("#signining_error_dialog", {transition: "pop"});
	}
	else if (status == Strophe.Status.ERROR || 
			 status == Strophe.Status.CONNFAIL)
	{
		if (ping_timer)
		{
			window.clearInterval(ping_timer);
			ping_timer = null;
		}
		
		loading_message_hide();
		$.mobile.changePage("#connecting_error_dialog", {transition: "pop"});
	}
}

function on_message(from, message)
{
	// alert('Get a message from ' + from + ': ' + message);
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

function resource_item_create(item_name, item_id)
{
	var list = $("<li id='resource_" + item_id + "'><a>" + 
				 "<img src='images/movie.png' />" +
				 "<h3>" + item_name + "</h3>" +
				 "<p>" +
					"<div id='resource_speed_" + item_id + "' class='resource_speed'>0MB 0KB/s</div>" +
					"<div id='resource_progressbar_" + item_id + "' class='resource_progressbar'/>" + 
				 "</p>" +
				 "</a><a href='#resource_config'>修改下载状态</a></li>");
					
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
