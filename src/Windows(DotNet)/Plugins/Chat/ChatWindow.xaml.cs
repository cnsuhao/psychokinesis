using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;
using System.ComponentModel.Composition;
using FirstFloor.ModernUI.Windows.Navigation;
using Newtonsoft.Json.Linq;
using Psychokinesis.Interface;

namespace Psychokinesis
{
    namespace Plugins
    {
        namespace Chat
        {
            /// <summary>
            /// UserControl1.xaml 的交互逻辑
            /// </summary>
            [Export(typeof(IPlugin))]
            [ExportMetadata("DisplayName", "文本")]
            [ExportMetadata("MuiUri", "/Chat")]
            public partial class ChatWindow : UserControl, IPlugin, IMessage
            {
                private string identification = "Chat";
                private ICommunication messager;
                private IDisposable messagerObserver;

                public ChatWindow()
                {
                    InitializeComponent();
                }

                // Mui IContent接口
                // The content is no longer the active content.
                public void OnNavigatedFrom(NavigationEventArgs e)
                {
                }

                // The content becomes the active content in a frame. This is a good time to initialize your content.
                public void OnNavigatedTo(NavigationEventArgs e)
                {
                }

                // You navigate to a link uri containing a fragment
                public void OnFragmentNavigation(FragmentNavigationEventArgs e)
                {
                }

                // The content is about to become inactive.You can use the OnNavigatingFrom method to cancel navigation by setting the NavigatingCancelEventArgs.Cancel to true.
                public void OnNavigatingFrom(NavigatingCancelEventArgs e)
                {
                }

                // Psychokinesis插件接口
                // 网络可用
                public void NetworkAvailable(ICommunication cc)
                {
                    messager = cc;
                    messagerObserver = messager.Subscribe(this);
                    DeviceListGrid.DataContext = messager.GetOnlineDevices();
                }

                // 网络不可用
                public void NetworkUnavailable()
                {
                    messagerObserver.Dispose();
                    messager = null;
                }

                // 其他终端上线
                public void DeviceOnline(Device dev)
                {
                }

                // 其他终端下线
                public void DeviceOffline(Device dev)
                {
                }

                // Psychokinesis通信接口
                public virtual String GetIdentification()
                {
                    return identification;
                }

                public virtual void OnCompleted()
                {}

                public virtual void OnError(Exception e)
                {}

                public virtual void OnNext(Message value)
                {
                    ChatText.AppendText("收到" + value.SourceName + "的消息：" + value.Content["message"] + "\n");
                }

                private void SendBtn_Click(object sender, RoutedEventArgs e)
                {
                    if (DeviceListGrid.SelectedItem == null)
                    {
                        DeviceListGrid.SelectAll();
                    }

                    if (DeviceListGrid.SelectedItem != null)
                    {
                        JObject msg = new JObject();
                        msg["message"] = SendText.Text;
                        Message m = new Message { Content = msg, DestinationName = ((Device)DeviceListGrid.SelectedItem).DeviceName };
                        messager.SendMessage(identification, m);

                        ChatText.AppendText("发给" + m.DestinationName + "的消息：" + SendText.Text + "\n");
                        SendText.Clear();
                    }
                }
            }
        }
    }
    
}