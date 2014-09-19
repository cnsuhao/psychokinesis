/* ==============================================================================
 * 作者：张扬
 * 创建时间：2014/8/27 21:41:11
 * 简介：消息内容
 * 用于承载在不同终端间传输的消息内容。
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
        /** 消息内容协议
         *
         * 包含需要序列化的数据。
         */
        [DataContract]
        public class Message
        {
            /// 消息源终端标识符
            [DataMember]
            public string SourceName { get; set; }

            /// 消息目标终端标识符
            [DataMember]
            public string DestinationName { get; set; }

            /// 消息内容
            [DataMember]
            public object Content { get; set; }
        }
    }
    
}