/* ==============================================================================
 * 作者：张扬
 * 创建时间：2014/9/7 22:33:12
 * 简介：加密/解密方法
 * 静态初始化实现的单例模式。
 * ==============================================================================*/
using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Security.Cryptography;

namespace Psychokinesis.Main.Util
{
    class Crypto
    {
        public string MD5String(string ori)
        {
            byte[] s = md5.ComputeHash(Encoding.UTF8.GetBytes(ori));
            return String.Concat(s.Select(b => b.ToString("X2")).ToArray()).ToLower();
        }

        public string DESEncryptString(string key, string toEncrypt)
        {
            string encrypted = null;

            using (DESCryptoServiceProvider des = new DESCryptoServiceProvider())
            {
                des.Key = UTF8Encoding.UTF8.GetBytes(key);
                des.IV = UTF8Encoding.UTF8.GetBytes(key);

                using (MemoryStream ms = new MemoryStream())
                {
                    using (CryptoStream cs = new CryptoStream(ms, des.CreateEncryptor(), CryptoStreamMode.Write))
                    {
                        using (StreamWriter sw = new StreamWriter(cs))
                        {
                            sw.Write(toEncrypt);
                        }
                    }

                    encrypted = Convert.ToBase64String(ms.ToArray());
                }
            }

            return encrypted;
        }

        public string DESDecryptString(string key, string toDecrypt)
        {
            string decrypted = null;

            using (DESCryptoServiceProvider des = new DESCryptoServiceProvider())
            {
                des.Key = UTF8Encoding.UTF8.GetBytes(key);
                des.IV = UTF8Encoding.UTF8.GetBytes(key);

                byte[] inputByteArray = Convert.FromBase64String(toDecrypt);

                using (MemoryStream ms = new MemoryStream())
                {
                    using (CryptoStream cs = new CryptoStream(ms, des.CreateDecryptor(des.Key, des.IV), CryptoStreamMode.Write))
                    {
                        cs.Write(inputByteArray, 0, inputByteArray.Length);
                        cs.FlushFinalBlock();
                        cs.Close();
                    }

                    decrypted = Encoding.UTF8.GetString(ms.ToArray());
                    ms.Close();
                }
            }
            return decrypted;
        }
        
        private MD5 md5 = MD5.Create();
        private Crypto() 
        { }

        // 实现单例模式
        private static readonly Crypto instance = new Crypto();
        public static Crypto Instance
        {
            get { return instance; }
        }
    }
}
