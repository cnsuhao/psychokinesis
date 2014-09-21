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
using System.Collections.ObjectModel;

namespace Psychokinesis
{
    namespace Interface
    {
        /** 通信接口
         *
         * 插件可通过NetworkAvailable事件获得此接口实例；
         * 插件可通过接口的Subscribe方法订阅消息事件。
         */
        public interface ICommunication : IObservable<Message>
        {
            /** 获取在线终端的信息
             *
             * 返回在线终端列表
             */
            ObservableCollection<Device> GetOnlineDevices();

            /** 向其他终端发送消息
             *
             * 需要指定通信实体。
             */
            void SendMessage(String identification, Message msg);
        }
    }
    
}