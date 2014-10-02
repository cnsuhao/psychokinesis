var url_params = {};

window.location.href.replace(/[?&]+([^=&]+)=([^&]*)/gi,    
	function(m, key, value) {
		url_params[key] = value;
	}
);


function encrypt(ori, key)
{				
	var keyHex = CryptoJS.enc.Utf8.parse(key);
	var ivHex = CryptoJS.enc.Utf8.parse(key);
	var encrypted = CryptoJS.DES.encrypt(ori, keyHex, { iv: ivHex });
				
	return encrypted.toString();
}
			
function decrypt(cryto, key)
{
	var keyHex = CryptoJS.enc.Utf8.parse(key);
	var ivHex = CryptoJS.enc.Utf8.parse(key);
	var decrypted = CryptoJS.DES.decrypt(cryto, keyHex, { iv: ivHex });
				
	return decrypted.toString(CryptoJS.enc.Utf8);
}
