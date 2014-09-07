/* ==============================================================================
 * 作者：张扬
 * 创建时间：2014/9/7 22:33:12
 * 简介：加密方法
 * 静态初始化实现的单例模式。
 * ==============================================================================*/
using System;
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
