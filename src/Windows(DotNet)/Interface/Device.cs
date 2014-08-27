/* ==============================================================================
 * 作者：张扬
 * 创建时间：2014/8/27 22:05:08
 * 简介：终端信息
 * 用于承载终端设备的基本信息。
 * ==============================================================================*/
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.Serialization;

namespace Psychokinesis
{
    namespace Interface
    {
        /** 终端信息协议
         *
         * 包含需要序列化的数据。
         */
        [DataContract]
        public class Device
        {
            /// 终端标识符
            [DataMember]
            public string DeviceName { get; set; }

            /// 终端名称
            [DataMember]
            public string DeviceNickName { get; set; }
        }
    }
    
}