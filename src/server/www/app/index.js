var APP_PATH = '/app/index.html';
var communication = null;

$(document).ready(function(){
	// 从URL参数获取序列号，失败则自动生成一个序列号
	if (url_params.hasOwnProperty("serial_number"))
	{
		var serial_number = url_params["serial_number"];
		
		// 切换页面后会带有#参数，需要去除
		serial_number = serial_number.split("#")[0];
	}
	else
		var serial_number = CryptoJS.MD5(Math.random().toString()).toString().substr(8, 16);
	
	if (!communication)
		communication = Communication.create();
		
	communication.connect(serial_number, on_connect);
});

function ui_ready()
{
	var url = 'http://' + window.location.hostname + APP_PATH + '?serial_number=' + communication.xmpp_account;
	var qrcode = new QRCode(document.getElementById("qr_code"), {
		width : 200,
		height : 200
	});
	
	qrcode.makeCode(url);
	$("#promot").text("Scan the qr code with the device that you want to be synchronized.");
	
	$("#synchronize_text").bind("input propertychange", function(){
		if (document.getElementById("auto_synchronize").checked)
		{
			var enc = encrypt(document.getElementById("synchronize_text").value, communication.xmpp_password);
			communication.send_message(enc);
		}
	});
	
	$("#synchronization_btn").bind("tap", function(){
		var enc = encrypt(document.getElementById("synchronize_text").value, communication.xmpp_password);
		communication.send_message(enc);
	});
	
	if (url_params.hasOwnProperty("serial_number"))
		$.ui.loadContent("page_synchronization", false, false, "slide");
}


function refresh_app()
{
	var url = "index.html";
	if (communication && communication.xmpp_account)
		url = url + "?serial_number=" + communication.xmpp_account;
		
	window.location = url;
}

function on_connect(status)
{
	if (status == Strophe.Status.CONNECTED)
	{
		communication.listen_presence(on_presence);
		communication.listen_message(on_message);
		communication.presence();
		
		$.ui.launch();
	}
	else if(status == Strophe.Status.AUTHFAIL ||
			status == Strophe.Status.ERROR ||
			status == Strophe.Status.CONNFAIL ||
			status == Strophe.Status.DISCONNECTED ||
			status == 998 || status == 999)
	{
		alert("An error occurred(" + status + "). Auto Refresh will happen after 20 seconds.");
		setTimeout("refresh_app()", 15000);      // 15秒后再尝试
	}
}

function on_presence(status, account, res)
{
	if (status == "available")
	{
		$.ui.loadContent("page_synchronization", false, false, "slide");
	}
}

function on_message(from, message)
{
	try
	{
		var dmsg = decrypt(message, communication.xmpp_password);
	}
	catch(err)
	{
		// alert(err.message);
	}
	
	document.getElementById("synchronize_text").value = dmsg;
}
