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
using System.Windows.Navigation;
using System.Windows.Shapes;
using FirstFloor.ModernUI.Windows.Controls;
using FirstFloor.ModernUI.Presentation;
using System.Windows.Threading;
using Psychokinesis.Main.Control;
using Psychokinesis.Interface;

namespace Psychokinesis
{
    namespace Main
    {
        /// <summary>
        /// MainWindow.xaml 的交互逻辑
        /// </summary>
        public partial class MainWindow : ModernWindow, IObserver<Message>
        {
            private IDisposable messagerObserver;
            private Geometry logo = Geometry.Parse("F1 M 53,49C 55.2091,49 57,50.7909 57,53C 57,55.2091 55.2091,57 53,57C 50.7909,57 49,55.2091 49,53C 49,50.7909 50.7909,49 53,49 Z M 57,24C 38.7746,24 24,38.7746 24,57L 19,57C 19,36.0132 36.0132,19 57,19L 57,24 Z M 57,34C 44.2974,34 34,44.2975 34,57L 29,57C 29,41.536 41.536,29 57,29L 57,34 Z M 57,44C 49.8203,44 44,49.8203 44,57L 39,57C 39,47.0589 47.0589,39 57,39L 57,44 Z");
            private DispatcherTimer flashLogoTimer = new DispatcherTimer();
            private DispatcherTimer reconnectTimer = new DispatcherTimer();

            public MainWindow()
            {
                InitializeComponent();
                this.LogoData = logo;

                // 根据插件初始化界面
                foreach (var pc in App.pluginCatalog)
                {
                    foreach (var p in pc.ExportDefinitions)
                    {
                        var linkGroup = new LinkGroup { DisplayName = (string)p.Metadata["DisplayName"] };
                        linkGroup.Links.Add(new Link { Source = new Uri((string)p.Metadata["MuiUri"], UriKind.Relative) });
                        MenuLinkGroups.Add(linkGroup);
                    }
                }

                // 插件目录有变化事件
                App.pluginCatalog.Changed += pluginCatalog_Changed;

                // 耗时的初始化操作过渡界面
                new SplashWindow().ShowDialog();
            }

            private void ModernWindow_Loaded(object sender, RoutedEventArgs e)
            {
                // 监听messager，连接中断时自动重连
                messagerObserver = Messenger.Instance.Subscribe(this);
            }

            private void ModernWindow_Closing(object sender, System.ComponentModel.CancelEventArgs e)
            {

            }

            private void ModernWindow_Unloaded(object sender, RoutedEventArgs e)
            {
                reconnectTimer.Stop();
                messagerObserver.Dispose();
            }

            private void ModernWindow_Closed(object sender, EventArgs e)
            {
                Messenger.Instance.Logout();
            }


            public virtual void OnCompleted()
            {
                if (Messenger.Instance.State == MessengerState.Online)
                {
                    this.Dispatcher.BeginInvoke(DispatcherPriority.Normal, (Action)delegate()
                    {
                        StopFlashLogo();
                        this.LogoData = logo;
                    });
                }
            }

            public virtual void OnError(Exception e)
            {
                if (Messenger.Instance.State == MessengerState.ConnectionError)
                {
                    this.Dispatcher.BeginInvoke(DispatcherPriority.Normal, (Action)delegate()
                    {
                        StopFlashLogo();
                        this.LogoData = null;

                        reconnectTimer.Interval = new TimeSpan(0, 0, 10);
                        reconnectTimer.Tick += reconnectTimer_Tick;
                        reconnectTimer.Start();
                    });
                }
            }

            public virtual void OnNext(Message value)
            { }


            private void reconnectTimer_Tick(object sender, EventArgs e)
            {
                Messenger.Instance.Login();
                StartFlashLogo();
                reconnectTimer.Stop();
            }

            // 插件目录有变化事件
            void pluginCatalog_Changed(object sender, System.ComponentModel.Composition.Hosting.ComposablePartCatalogChangeEventArgs e)
            {
                foreach (var pc in e.AddedDefinitions)
                {
                    foreach (var p in pc.ExportDefinitions)
                    {
                        var linkGroup = new LinkGroup { DisplayName = (string)p.Metadata["DisplayName"] };
                        linkGroup.Links.Add(new Link { Source = new Uri((string)p.Metadata["MuiUri"], UriKind.Relative) });
                        MenuLinkGroups.Add(linkGroup);
                    }
                }

                foreach (var pc in e.RemovedDefinitions)
                {
                    foreach (var p in pc.ExportDefinitions)
                    {
                        var nd = (from l in MenuLinkGroups
                                  where l.Links.FirstOrDefault().Source.OriginalString == (string)p.Metadata["MuiUri"]
                                  select l).FirstOrDefault();
                        if (nd == null)
                            throw new ArgumentException((string)p.Metadata["MuiUri"] + "isn't found!");

                        MenuLinkGroups.Remove(nd);
                    }
                }
            }

            // 闪烁logo
            private void StartFlashLogo()
            {
                flashLogoTimer.Interval = new TimeSpan(0, 0, 1);
                flashLogoTimer.Tick += flashLogoTimer_Tick;
                flashLogoTimer.Start();
            }

            // 停止闪烁logo
            private void StopFlashLogo()
            {
                flashLogoTimer.Stop();
            }

            private void flashLogoTimer_Tick(object sender, EventArgs e)
            {
                if (this.LogoData == null)
                {
                    this.LogoData = logo;
                }
                else
                {
                    this.LogoData = null;
                }
            }
        }
    } 
}
