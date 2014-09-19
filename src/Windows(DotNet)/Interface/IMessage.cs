/* ==============================================================================
 * 作者：张扬
 * 创建时间：2014/9/19 21:33:46
 * 简介：消息接口
 * 基于此接口实现需要获取其他终端发来消息的对象。
 * ==============================================================================*/
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Psychokinesis.Interface
{
    /** 消息接口
     *
     * 使用观察者模式实现
     */
    public interface IMessage : IObserver<Message>
    {
        /** 获取标识
         *
         * 通信实体的名字，用于标识此实体，只允许同名的实体在不同终端间通信。
         */
        String GetIdentification();
    }
}
