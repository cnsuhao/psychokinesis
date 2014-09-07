/* ==============================================================================
 * 作者：张扬
 * 创建时间：2014/9/6 13:20:16
 * 简介：与服务器通信的实现
 * 静态初始化实现的单例模式。
 * ==============================================================================*/
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Threading;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using Psychokinesis.Interface;
using Psychokinesis.Main.Util;


namespace Psychokinesis.Main.Control
{
    class Messenger : ICommunication
    {
        private Thread workThread;
        private bool isRunning = false;
        private Semaphore workSemaphore = new Semaphore(0, 1);
        private MessengerState state = MessengerState.NotActive;      // 仅允许在workThread中修改
        public MessengerState State { get { return state; } }

        private List<IObserver<Message>> observers;

        public void Login()
        {
            if (State != MessengerState.NotActive)
                return;

            workSemaphore.Release();

            if (workThread.IsAlive == false)
            {
                isRunning = true;
                workThread.Start();
            }
        }

        // 在messenger的监听事件中直接调用此方法会导致卡死，因为此函数需要等待工作线程结束
        public void Logout()
        {
            if (workThread.IsAlive == true)
            {
                isRunning = false;

                if (State == MessengerState.NotActive)
                    workSemaphore.Release();

                workThread.Join();
            }
        }

        private void Run()
        {
            while (isRunning)
            {
                workSemaphore.WaitOne();
                if (isRunning == false)
                    break;

                state = MessengerState.Active;
                NotifyCompleted();

                do
                {
                    // 获取登录信息
                    string account, password;
                    try
                    {
                        string serialNumber = Crypto.Instance.MD5String(
                            string.Join("", SystemInfo.Instance.ProcessorIds.ToArray()) +
                            string.Join("", SystemInfo.Instance.PhysicalMediaSerialNumbers.ToArray()) +
                            string.Join("", SystemInfo.Instance.NetworkAdapterMacs.ToArray())
                            ).Substring(8, 16);

                        var postForm = new Dictionary<string, string>()
                        {
                            {"serialnumber", serialNumber}
                        };
                        string accountInfo = HttpClient.Post("http://psychokinesis.me/nodejs/access-communication",
                                                             postForm);

                        JObject o = (JObject)JsonConvert.DeserializeObject(accountInfo);
                        account = (string)o["account"];
                        password = (string)o["password"];
                        if (account == null || password == null)
                        {
                            string errorDesc = (string)o["desc"];

                            Exception e;
                            if (errorDesc != null)
                                e = new Exception(errorDesc);
                            else
                                e = new Exception();
                            NotifyError(e);
                            break;
                        }
                    }
                    catch (Exception e)
                    {
                        NotifyError(e);
                        break;
                    }

                    state = MessengerState.GetLoginInformation;
                    NotifyCompleted();

                    // TODO 登录

                    state = MessengerState.Online;
                    NotifyCompleted();

                    // TODO 发送心跳包
                }while(false);

                if (state == MessengerState.Online)
                {
                    // TODO 关闭与服务器的连接
                }

                state = MessengerState.NotActive;
            }
        }


        public Device[] GetOnlineDevices()
        {
            throw new NotImplementedException();
        }

        public bool SendMessage(Message msg)
        {
            throw new NotImplementedException();
        }


        private Messenger() 
        {
            workThread = new Thread(Run);

            observers = new List<IObserver<Message>>();
        }

        // 实现观察者模式
        public IDisposable Subscribe(IObserver<Message> observer)
        {
            lock (observers)
            {
                if (!observers.Contains(observer))
                    observers.Add(observer);
            }
            return new Unsubscriber(observers, observer);
        }

        private class Unsubscriber : IDisposable
        {
            private List<IObserver<Message>> _observers;
            private IObserver<Message> _observer;

            public Unsubscriber(List<IObserver<Message>> observers, IObserver<Message> observer)
            {
                this._observers = observers;
                this._observer = observer;
            }

            public void Dispose()
            {
                lock (_observer)
                {
                    if (_observers.Contains(_observer))
                        _observers.Remove(_observer);
                }
            }
        }

        private void NotifyCompleted()
        {
            lock (observers)
            {
                foreach (var observer in observers)
                    observer.OnCompleted();
            }
        }

        private void NotifyError(Exception error)
        {
            lock (observers)
            {
                foreach (var observer in observers)
                    observer.OnError(error);
            }
        }

        private void NotifyNext(Message msg)
        {
            lock (observers)
            {
                foreach (var observer in observers)
                    observer.OnNext(msg);
            }
        }

        // 实现单例模式
        private static readonly Messenger instance = new Messenger();
        public static Messenger Instance
        {
            get { return instance; }
        }
    }

    enum MessengerState
    {
        NotActive,                // 工作线程未启动至获得信号量之前
        Active,                   // 获得信号量之后到获取登录信息之前
        GetLoginInformation,
        Online
    }
}
