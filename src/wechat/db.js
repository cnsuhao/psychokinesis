

var mongoose = require('mongoose');
var config = require('./config');
var Schema = mongoose.Schema;


var qrcodesschema = new Schema({ name: String, sid: Number });
var accbindschema = new Schema({name:String, wxopenid:String});

var db  = exports.Db = mongoose.createConnection(config.db.url);

db.on('error', function (err) {
       //logger.error("connect error :" + err);
       //监听BAE mongodb异常后关闭闲置连接
       console.log('connect err: ' + err);
       db.close();
   });
//监听db close event并重新连接
db.on('close', function () {
	console.log("connect close retry connect ");
	var option = {
		db: { native_parser: true },
		server: { poolSize: 5 },
		user: config.db.user,
		pass: config.db.pass
	};
	db.open(config.db.host, config.db.name, config.db.port, option);
});

var qrcodes = db.model('qrcodes', qrcodesschema);
var accbind = db.model('accbind', accbindschema);

exports.saveQrcode = function(account, sid) {

	qrcodes.find({ sid:sid}, function(err,docs){}).remove().exec();

	var doc1 = new qrcodes({name:account, sid:sid});
	doc1.save(function(err){
		if (err) // ...
			console.log('meow');
	});

}

//通过临时二维码序号获取将绑定的账号
exports.findQrcodebysid = function(sid, callback) {

	return qrcodes.find({ sid:sid}, callback);
}

//通过微信账号查找绑定账号
exports.getBindAccount = function(oid, callback) {
	accbind.findOne({wxopenid:oid}, callback);
}

//解除绑定
exports.unbindAccount = function(oid) {
	accbind.find({wxopenid:oid}, function(err, docs){}).remove().exec();	
}

//绑定
exports.bindAccount = function(account, oid, callback) {
	accbind.find({wxopenid:oid}, function(err, docs){}).remove().exec();
	var ac = new accbind({name: account, wxopenid: oid});
	ac.save(callback);
}