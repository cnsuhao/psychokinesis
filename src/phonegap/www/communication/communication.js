var BOSH_DOMAIN = 'psychokinesis.wuyingfengsui.me';
var BOSH_PORT = 5280;
var BOSH_RESOURCE = 'psychokinesis-mobile';
var REMOTE_RESOURCE = 'psychokinesis-pc';
var BOSH_SERVICE = '/http-bind';

var Communication = {
	create : function() {
		var communication = {};
		
		communication.xmpp_jid = null;
		communication.xmpp_account = null;
		communication.xmpp_password = null;
		communication.xmpp_connection = null;
		communication.message_listeners = new Array();
		
		communication.connect = function (account, password, on_connect)
		{
			if (communication.xmpp_connection == null)
					communication.xmpp_connection = new Strophe.Connection('http://' + BOSH_DOMAIN + ':' + BOSH_PORT + BOSH_SERVICE);
	
			communication.xmpp_account = account;
			communication.xmpp_jid = account + '@' + BOSH_DOMAIN + '/' + BOSH_RESOURCE;
			communication.xmpp_password = password;
	
			communication.xmpp_connection.connect(communication.xmpp_jid, communication.xmpp_password, on_connect);
		}
		
		communication.reconnect = function (on_connect)
		{
			communication.xmpp_connection.connect(communication.xmpp_jid, communication.xmpp_password, on_connect);
		}
		
		communication.disconnect = function ()
		{
			communication.xmpp_connection.disconnect();
		}
		
		communication.listen_presence = function (on_func)
		{
			communication.xmpp_connection.addHandler(function (msg)
													 {
														 // TODO 接收PC客户端上下线消息
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
																var message_json = jQuery.parseJSON(message);
																
																if (message_json)
																{
																	on_func(from, message_json);
															
																	if (message_json.session_id != undefined && 
																		message_json.session_id < communication.message_listeners.length)
																	{
																		communication.message_listeners[message_json.session_id].func(message_json);
																		if (communication.message_listeners[message_json.session_id].timer)
																		{
																			window.clearInterval(communication.message_listeners[message_json.session_id].timer);
																			communication.message_listeners[message_json.session_id].timer = null;
																		}
																	}
																}
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
			communication.xmpp_connection.ping.ping(BOSH_DOMAIN, null, null, 30*1000);
		}
		
		communication.send_message = function (message, timeout, listener_func)
		{
			if (listener_func)
			{
				var cur_length = communication.message_listeners.length;
				
				communication.message_listeners[cur_length] = {};
				communication.message_listeners[cur_length].func = listener_func;
				if (timeout)
				{
					communication.message_listeners[cur_length].timer = window.setInterval(function(){
						var error_message = {ret_code: 1000};
						listener_func(error_message);
						
						window.clearInterval(communication.message_listeners[cur_length].timer);
						communication.message_listeners[cur_length].timer = null;
					},
					timeout);
				}
				
				message["session_id"] = cur_length;
				
				var m_msg = $msg({to: communication.xmpp_account + '@' + BOSH_DOMAIN + '/' + REMOTE_RESOURCE, 
							  	 from: communication.xmpp_jid, 
							  	 type: 'chat'})
       						 .c('body', null, JSON.stringify(message));
			}
			else
				var m_msg = $msg({to: communication.xmpp_account + '@' + BOSH_DOMAIN + '/' + REMOTE_RESOURCE,
							  	 from: communication.xmpp_jid, 
							  	 type: 'chat'})
       						 .c('body', null, JSON.stringify(message));
			
			communication.xmpp_connection.send(m_msg.tree());
		}
		
		return communication;
	}
};