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
using System.Collections;
using System.Collections.ObjectModel;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using Psychokinesis.Interface;
using Psychokinesis.Main.Util;
using agsXMPP;
using agsXMPP.protocol.extensions.ping;


namespace Psychokinesis.Main.Control
{
    class Messenger : ICommunication
    {
        private Thread workThread;
        private bool isRunning = false;
        private Semaphore workSemaphore = new Semaphore(0, 2);        // 界面和xmpp响应事件可能同时触发
        private MessengerState state = MessengerState.NotActive;      // 仅允许在workThread中修改

        private XmppClientConnection xmppClient = new XmppClientConnection("chat.psychokinesis.me", 5222);
        private string serialNumber = Properties.Settings.Default.SerialNumber;

        private Hashtable observers = new Hashtable();

        private ObservableCollection<Device> devices = new ObservableCollection<Device>();

        public MessengerState State { get { return state; } }
        public String SerialNumber { get { return serialNumber; } }
        public String Account { get { return xmppClient.Username; } }
        public String Password { get { return xmppClient.Password; } }

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

                if (State == MessengerState.NotActive ||
                    State == MessengerState.GetLoginInformation ||
                    State == MessengerState.Online)
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
                    // 程序启动时获取一次登录信息
                    if (xmppClient.Username == "")
                    {
                        try
                        {
                            if (serialNumber == "")
                            {
                                serialNumber = Crypto.Instance.MD5String(
                                    string.Join("", SystemInfo.Instance.ProcessorIds.ToArray()) +
                                    string.Join("", SystemInfo.Instance.PhysicalMediaSerialNumbers.ToArray()) +
                                    string.Join("", SystemInfo.Instance.NetworkAdapterMacs.ToArray())
                                    ).Substring(8, 16);

                                Properties.Settings.Default.SerialNumber = serialNumber;
                                Properties.Settings.Default.Save();
                            }

                            string accountInfo = HttpClient.Get("http://psychokinesis.me/nodejs/access-communication?serialnumber=" + serialNumber);

                            JObject o = (JObject)JsonConvert.DeserializeObject(accountInfo);
                            string account = (string)o["account"];
                            string password = (string)o["password"];
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

                            xmppClient.Username = account;
                            xmppClient.Password = password;
                        }
                        catch (Exception e)
                        {
                            NotifyError(e);
                            break;
                        }

                        state = MessengerState.GetLoginInformation;
                        NotifyCompleted();
                    }

                    // 登录
                    xmppClient.Resource = "psychokinesis-pc";
                    xmppClient.Open();

                    workSemaphore.WaitOne();
                    if (isRunning == false)
                        break;

                    if (state != MessengerState.Online)
                    {
                        NotifyError(new Exception());
                        break;
                    }

                    NotifyCompleted();

                    System.Windows.Application.Current.Dispatcher.BeginInvoke(System.Windows.Threading.DispatcherPriority.Normal, (Action)delegate()
                    {
                        try
                        {
                            App.PluginsManager.NetworkAvailable(this);
                        }
                        catch (Exception)
                        { }

                    });

                    // 发送心跳包
                    while (isRunning &&
                            state == MessengerState.Online)
                    {
                        PingIq p = new PingIq(xmppClient.Server, xmppClient.MyJID);
                        p.Type = agsXMPP.protocol.client.IqType.get;
                        xmppClient.Send(p);

                        workSemaphore.WaitOne(30000);       // 30s
                    }

                    if (isRunning)
                    {
                        NotifyError(new Exception());

                        System.Windows.Application.Current.Dispatcher.BeginInvoke(System.Windows.Threading.DispatcherPriority.Normal, (Action)delegate()
                        {
                            try
                            {
                                App.PluginsManager.NetworkUnavailable();
                            }
                            catch (Exception)
                            { }

                        });
                    }
                }while(false);

                if (state == MessengerState.Online)
                {
                    xmppClient.Close();
                }

                state = MessengerState.NotActive;
            }
        }


        public ObservableCollection<Device> GetOnlineDevices()
        {
            return devices;
        }

        public void SendMessage(String identification, Message msg)
        {
            JObject m = new JObject();
            m["identification"] = identification;
            m["content"] = msg.Content;
            string str_m = JsonConvert.SerializeObject(m);

            if (this.Password.Length >= 8)
                str_m = Crypto.Instance.DESEncryptString(this.Password.Substring(0, 8), str_m);

            agsXMPP.protocol.client.Message mm = new agsXMPP.protocol.client.Message(xmppClient.MyJID.Bare + '/' + msg.DestinationName, agsXMPP.protocol.client.MessageType.chat, str_m);
            xmppClient.Send(mm);
        }


        private void xmppClient_OnPresence(object sender, agsXMPP.protocol.client.Presence pres)
        {
            if (pres.From.ToString() != xmppClient.MyJID.ToString())
            {
                int pos = pres.From.Resource.IndexOf("-");
                string nick = pos == -1 ? pres.From.Resource : pres.From.Resource.Substring(pos + 1);

                Device d = new Device { DeviceName = pres.From.Resource, DeviceNickName = nick, JoinTime = DateTime.Now };
                if (pres.Type == agsXMPP.protocol.client.PresenceType.available)
                {
                    System.Windows.Application.Current.Dispatcher.BeginInvoke(System.Windows.Threading.DispatcherPriority.Normal, (Action)delegate()
                    {
                        devices.Add(d);
                        App.PluginsManager.DeviceOnline(d);
                    });
                }
                else if (pres.Type == agsXMPP.protocol.client.PresenceType.unavailable)
                {
                    System.Windows.Application.Current.Dispatcher.BeginInvoke(System.Windows.Threading.DispatcherPriority.Normal, (Action)delegate()
                    {
                        devices.Remove(
                            devices.Where(x => x.DeviceName == d.DeviceName).FirstOrDefault()
                        );
                        App.PluginsManager.DeviceOffline(d);
                    });
                }
            }
        }

        private void xmppClient_OnMessage(object sender, agsXMPP.protocol.client.Message msg)
        {
            string identification;
            Message mm = new Message { SourceName = msg.From.Resource };

            try
            {
                string str_m = msg.Body;
                if (this.Password.Length >= 8)
                    str_m = Crypto.Instance.DESDecryptString(this.Password.Substring(0, 8), msg.Body);

                JObject m = (JObject)JsonConvert.DeserializeObject(str_m);
                identification = (string)m["identification"];
                mm.Content = m["content"] as JObject;
                if (identification == null ||
                    mm.Content == null)
                    return;
            }
            catch (Exception)
            {
                return;
            }

            lock (observers)
            {
                if (observers.Contains(identification))
                {
                    System.Windows.Application.Current.Dispatcher.BeginInvoke(System.Windows.Threading.DispatcherPriority.Normal, (Action)delegate()
                        {
                            try
                            {
                                ((IMessage)observers[identification]).OnNext(mm);
                            }
                            catch (Exception)
                            { }
                    });
                }
            }
        }


        private Messenger() 
        {
            workThread = new Thread(Run);

            xmppClient.AutoResolveConnectServer = false;
            xmppClient.OnLogin += xmppClient_OnLogin;
            xmppClient.OnClose += xmppClient_OnClose;
            xmppClient.OnError += xmppClient_OnError;
            xmppClient.OnAuthError += xmppClient_OnAuthError;
            xmppClient.OnSocketError += xmppClient_OnSocketError;
            xmppClient.OnPresence += xmppClient_OnPresence;
            xmppClient.OnMessage += xmppClient_OnMessage;
        }

        private void xmppClient_OnSocketError(object sender, Exception ex)
        {
            state = MessengerState.ConnectionError;
            workSemaphore.Release();
        }

        private void xmppClient_OnAuthError(object sender, agsXMPP.Xml.Dom.Element e)
        {
            state = MessengerState.ConnectionError;
            workSemaphore.Release();
        }

        private void xmppClient_OnError(object sender, Exception ex)
        {
            state = MessengerState.ConnectionError;
            workSemaphore.Release();
        }

        private void xmppClient_OnClose(object sender)
        {
            if (isRunning)
            {
                state = MessengerState.ConnectionError;          // 经测试连接异常中断会触发此事件
                workSemaphore.Release();
            }
        }

        private void xmppClient_OnLogin(object sender)
        {
            state = MessengerState.Online;
            workSemaphore.Release();
        }

        // 实现观察者模式
        public IDisposable Subscribe(IObserver<Message> observer)
        {
            IMessage m = observer as IMessage;
            if (m == null)
                throw new EntryPointNotFoundException("observer isn't IMessage");

            lock (observers)
            {
                if (!observers.Contains(m.GetIdentification()))
                    observers.Add(m.GetIdentification(), m);
            }
            return new Unsubscriber(observers, m);
        }

        private class Unsubscriber : IDisposable
        {
            private Hashtable _observers;
            private IMessage _observer;

            public Unsubscriber(Hashtable observers, IMessage observer)
            {
                this._observers = observers;
                this._observer = observer;
            }

            public void Dispose()
            {
                lock (_observer)
                {
                    if (_observers.Contains(_observer.GetIdentification()))
                        _observers.Remove(_observer.GetIdentification());
                }
            }
        }

        private void NotifyCompleted()
        {
            lock (observers)
            {
                foreach (DictionaryEntry observer in observers)
                    ((IMessage)observer.Value).OnCompleted();
            }
        }

        private void NotifyError(Exception error)
        {
            lock (observers)
            {
                foreach (DictionaryEntry observer in observers)
                    ((IMessage)observer.Value).OnError(error);
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
        Online,
        ConnectionError
    }
}
