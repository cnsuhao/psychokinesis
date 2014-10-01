var XMPP_DOMAIN = 'chat.psychokinesis.me';
var XMPP_PORT = 5288;
var BOSH_SERVICE = '/http-bind';

var Communication = {
	create : function() {
		var communication = {};
		
		communication.xmpp_jid = null;
		communication.xmpp_bare_jid = null;
		communication.xmpp_account = null;
		communication.xmpp_password = null;
		communication.xmpp_resource = null;
		communication.xmpp_connection = null;
		communication.connect_timeout = null;
		
		var xmpp_connect = function (account, password, resource, on_connect)
		{
			if (communication.xmpp_connection == null)
			{
				if (window.WebSocket)
					communication.xmpp_connection = new Strophe.Connection('ws://' + XMPP_DOMAIN + ':' + XMPP_PORT,
																		   {protocol: 'ws'});
				else
					communication.xmpp_connection = new Strophe.Connection('http://' + XMPP_DOMAIN + ':' + XMPP_PORT + BOSH_SERVICE);
			}
	
			var connect_listener = function (status)
			{
				if (communication.connect_timeout && status == Strophe.Status.CONNECTED)
				{
					clearTimeout(communication.connect_timeout);
					communication.connect_timeout = null;
				}
				
				on_connect(status);
			}
			
			communication.xmpp_account = account;
			communication.xmpp_jid = account + '@' + XMPP_DOMAIN + '/' + resource;
			communication.xmpp_bare_jid = account + '@' + XMPP_DOMAIN;
			communication.xmpp_password = password;
			communication.xmpp_resource = resource;
	
			communication.xmpp_connection.reset();
			communication.xmpp_connection.connect(communication.xmpp_jid, communication.xmpp_password, connect_listener);
			
			// 60秒连接超时
			communication.connect_timeout = setTimeout("on_connect(999)", 60000);
		}
		
		communication.connect = function (serial_number, on_connect)
		{
			// 根据序列号从服务器获取账号
			var try_time = 0;
			
			function try_fetch_account() {
				if (++try_time > 3) {
					on_connect(998);
					return;
				}
				
				$.ajax({
					type:"GET",
					url: 'http://psychokinesis.me/nodejs/access-communication',
					data: {serialnumber: serial_number},
					success: function(data){
						if (data.account && data.password)
						{
							// 使用获取到的账号登录
							var res = CryptoJS.MD5(Math.random().toString()).toString().substr(8, 16);
							xmpp_connect(data.account, data.password, 'psychokinesis-webapp-' + res, on_connect);
						}
						else
						{
							on_connect(998);
							return;
						}
					},
					timeout: "10000",
					error: function(){
						try_fetch_account();
					}
				});
			}
			
			try_fetch_account();
		}
		
		communication.reconnect = function (on_connect)
		{
			var connect_listener = function (status)
			{
				if (communication.connect_timeout && status == Strophe.Status.CONNECTED)
				{
					clearTimeout(communication.connect_timeout);
					communication.connect_timeout = null;
				}
				
				on_connect(status);
			}
			
			communication.xmpp_connection.connect(communication.xmpp_jid, communication.xmpp_password, connect_listener);
			
			// 60秒连接超时
			communication.connect_timeout = setTimeout("on_connect(999)", 60000);
		}
		
		communication.disconnect = function ()
		{
			communication.xmpp_connection.disconnect();
		}
		
		communication.listen_presence = function (on_func)
		{
			// 接收其他客户端上下线消息
			communication.xmpp_connection.addHandler(function (msg)
													 {
														if (msg.getAttribute('from') != communication.xmpp_jid)
														{
															var bare_jid = Strophe.getBareJidFromJid(msg.getAttribute('from'));
															var res = Strophe.getResourceFromJid(msg.getAttribute('from'));
															
															var t = bare_jid.split('@');
															var acc = t[0];
															
															if (msg.getAttribute('type') == 'unavailable')
																on_func('unavailable', acc, res);
															else
																on_func('available', acc, res);
														}
														 
														return true;
													 }, 
										   null, 'presence', null, null, null); 
		}
		
		communication.listen_message = function (on_func)
		{
			communication.xmpp_connection.addHandler(function (msg)
													 {
														try
														{
															if (msg.getAttribute('from') != communication.xmpp_jid)    // 防止自己发的消息被处理
															{
																if (msg.getAttribute('type') != 'chat')
																	return true;
																
																var from = Strophe.getBareJidFromJid(msg.getAttribute('from'));
        														var message = $(msg).find('body')[0].childNodes[0].data;
																
																on_func(from, message);
															}
														}
														catch(err)
														{
															// alert(err.message);
														}
														
														return true;
													 }, 
										   null, 'message', null, null, null); 
		}
		
		communication.presence = function ()
		{
			communication.xmpp_connection.send($pres().tree());
		}
		
		communication.ping = function ()
		{
			communication.xmpp_connection.ping.ping(XMPP_DOMAIN, null, null, 30*1000);
		}
		
		communication.send_message = function (message)
		{
			var m_msg = $msg({to: communication.xmpp_account + '@' + XMPP_DOMAIN,
							  from: communication.xmpp_jid, 
							  type: 'chat'})
       					.c('body', null, message);
			
			communication.xmpp_connection.send(m_msg.tree());
		}
		
		return communication;
	}
};