var BOSH_DOMAIN = '127.0.0.1';
var BOSH_RESOURCE = 'psychokinesis-mobile';
var REMOTE_RESOURCE = 'psychokinesis-pc';
var BOSH_SERVICE = '/http-bind/';

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
					communication.xmpp_connection = new Strophe.Connection(BOSH_SERVICE);
	
			communication.xmpp_account = account;
			communication.xmpp_jid = account + '@' + BOSH_DOMAIN + '/' + BOSH_RESOURCE;
			communication.xmpp_password = password;
	
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
														 // TODO 接收PC客户端离线消息
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
															if (msg.getAttribute('type') != 'chat')
																return true;
																
															var from = Strophe.getBareJidFromJid(msg.getAttribute('from'));
															
															if (window.navigator.userAgent.indexOf('MSIE') > 0 ||
																!!window.navigator.userAgent.match(/Trident.*/)) {
        														var els = msg.getElementsByTagName("*");
																for (var i=0, numEls=els.length, el; i<numEls; i++){
																	if (els[i].localName == 'body')
																		var message = els[i].childNodes[0].data;
																}
															}
															else
																var message = $(msg).find('body').html();
															
															on_func(from, message);
															
															if (msg.getAttribute('from') != communication.xmpp_jid)    // 防止自己发的消息被处理
															{
																var message_json = jQuery.parseJSON(message);
																if (message_json.session_id < communication.message_listeners.length)
																	communication.message_listeners[message_json.session_id](message_json);
															}
														}
														catch(err)
														{}
														
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
		
		communication.send_message = function (message, listener_func)
		{
			if (listener_func)
			{
				communication.message_listeners[communication.message_listeners.length] = listener_func;
				message["session_id"] = communication.message_listeners.length - 1;
				
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