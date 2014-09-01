/* ==============================================================================
 * 作者：张扬
 * 创建时间：2014/8/27 20:52:46
 * 简介：插件接口
 * 基于此接口实现具体的插件。
 * ==============================================================================*/
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using FirstFloor.ModernUI.Windows;

namespace Psychokinesis
{
    namespace Interface
    {
        /** 插件接口
         *
         * 继承ModernUI的IContent接口，因此应该让插件的窗口类继承它。
         */
        public interface IPlugin : IContent
        {
            /** 网络可用
             *
             * 网络可以使用事件，之后便可通过ICommunication使用网络进行通信。
             */
            void NetworkAvailable(ICommunication cc);

            /** 网络不可用
             *
             * 网络中断事件，之后不可再使用ICommunication，否则相关调用会抛出异常。
             */
            void NetworkUnavailable();

            /** 其他终端上线
             *
             * 上线终端的标识符
             */
            void DeviceOnline(Device dev);

            /** 其他终端下线
             *
             * 下线终端的标识符
             */
            void DeviceOffline(Device dev);

            /** 接收其他终端发来的消息
             *
             * 可获得消息的来源及内容
             */
            void MessageReceived(Message msg);
        }

        /** 插件属性
         *
         * 定义插件时使用特性声明这些属性，以标识此插件。
         */
        public interface IPluginMetadata
        {
            // Mui的标识属性，Mui框架依据此属性查找此插件
            string MuiUri { get; }
            // 插件的界面标题
            string DisplayName { get; }
        }
    } 
}
