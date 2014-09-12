
var config = module.exports;

config.db  = {
	url: 'mongodb://USER:PASS@HOST:PORT/DBNAME',
	host: 'mongo.duapp.com',
	port: '8908',
	name: 'DBNAME',
	user: 'USER',
	pass: 'PASS'

}

var menu = {
	"button":[
	{	
		"type":"click",
		"name":"取消绑定",
		"key":"V1001_UNBIND"
	},
	{
		"type":"click",
		"name":"绑定账号名",
		"key":"V1001_GET_ACCOUNT"
	},
	{
		"name":"菜单",
		"sub_button":[
		{	
			"type":"view",
			"name":"搜索",
			"url":"http://www.soso.com/"
		},
		{
			"type":"view",
			"name":"视频",
			"url":"http://v.qq.com/"
		},
		{
			"type":"click",
			"name":"赞一下我们",
			"key":"V1001_GOOD"
		}]
	}]
}


config.wechat = {
	appID: 'appid',
	appsecret: 'appsecret',
	Token: 'token',
	menu: menu
}

config.app = {
	port: '18080'
}