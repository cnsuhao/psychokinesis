var wechat = require('wechat');
var bodyParser = require('body-parser');
var express = require('express');
var config = require('./config');
var database = require('./db.js');


var API = wechat.API;
var api = new API(config.wechat.appID, config.wechat.appsecret);
var app = express();


api.createMenu(config.wechat.menu, function (err, result){});

app.use(bodyParser.json());
app.use(bodyParser.urlencoded());
app.use(express.static(__dirname+'/static'));

//监听获取二维码请求
app.use('/yuki', function(req, res){

	console.log('Request DATA');
	console.log(req.body);
	var opr = req.body.opr;
	switch (opr) {
		case 'getqrcode':

			var account = req.body['account'];
			var password = req.body['password'];
			//TODO check account

			//create qrcode Expire after 900s     100000 > dur * 60 * 10
			var sid = Math.floor((new Date().getTime() / 10) % 100000) + 1;
			//	res.write('sid: ' + sid);
			var dur = 900;
			api.createTmpQRCode(sid, dur, function (err, ress){     
			//	res.end(api.showQRCodeURL(ress.ticket));
				if (err) {
					res.send({success:false, msg: 'create code failed'});
					return;
				}
				database.saveQrcode(account, sid);
				res.send({success: true, account: account, codeurl: api.showQRCodeURL(ress.ticket)});
			});
			break;
		case 'echo':
			var msg = req.body;
			msg.success = true;
			res.send(req.body);
			break;
		default:
			res.send({success:false, msg: 'unknown opr'});
			break;
	}
	//write sid-account to checktable

//	res.write('<p>Hello World</p>');
});

//自定义菜单点击事件
function onClick(req, res, next) {
	var message = req.weixin;
	console.log(message.EventKey);
	switch (message.EventKey) {
		case 'V1001_UNBIND':
		database.unbindAccount(message.FromUserName);
		res.reply('unbind account');
		break;
		case 'V1001_GET_ACCOUNT':
		database.getBindAccount(message.FromUserName, function(err, ac){
			if (err || ac == null){
				err && console.log(err);
				res.reply('404 not founded');
				return;
			}
			var name = ac.name;
			if (name != null)
				res.reply(name);
			else
				res.reply('404 not founded');
		});
		break;
		default:
	}
}

//扫描二维码绑定账号
function onScan(req, res, next) {

	var message = req.weixin;
	var sid = message.EventKey.replace('qrscene_','');
	var str = 'ff';
	console.log("sid: " + sid);
	database.findQrcodebysid(sid, function(err,docs){
		console.log(docs);
		var len = docs.length;
		console.log('len'+ len);
		if (docs.length == 1){
			str = docs[0].name;
		} else {
			res.reply('qrcode error');
		}

		var wxuser = message.FromUserName;
		console.log("name: " + str + ',' + wxuser);

		//band weixinaccount-account to DB
		database.bindAccount(str, wxuser, function(err){
			if (err){
				res.reply('bindAccount failed');
			} else {
				res.reply(str + ',' + wxuser);           
			}
		});
	});

}

app.use(wechat(config.wechat.Token, function (req, res, next) {
// 微信输入信息都在req.weixin上
var message = req.weixin;

switch (message.MsgType){
	case 'event':
	switch (message.Event) {
		case 'subscribe':
		case 'SCAN':
		onScan(req, res, next);
		break;
		case 'CLICK':
		onClick(req, res, next);
		break;
		default:
		reply('event: ' + message.Event);
		break;
	}
	console.log("event: " + message.Event);
	break;
	case 'location':
	case 'text':
	case 'voice':
	case 'url':
	case 'image':
	res.reply({
		content: message.MsgType,
		type: 'text'});
	break;

	default:
	break;
}

}));

app.listen(config.app.port);
