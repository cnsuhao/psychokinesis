/* ==============================================================================
 * 作者：张扬
 * 创建时间：2014/8/27 20:53:23
 * 简介：通信接口
 * 用于与其他终端通信。
 * ==============================================================================*/
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Psychokinesis
{
    namespace Interface
    {
        /** 通信接口
         *
         * 插件可通过NetworkAvailable事件获得此接口实例。
         */
        public interface ICommunication
        {
            /** 获取在线终端的信息
             *
             * 返回在线终端列表
             */
            Device[] GetOnlineDevices();

            /** 向其他终端发送消息
             *
             * 同步调用，如有异步需要请使用SendMessageDelegate委托。
             */
            bool SendMessage(Message msg);
        }

        /** 发送消息的委托
         *
         * 异步发送消息
         */
        public delegate void SendMessageDelegate(Message msg);
    }
    
}