using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;
using System.ComponentModel.Composition;
using System.Drawing;
using System.Drawing.Imaging;
using FirstFloor.ModernUI.Windows.Navigation;
using Gma.QrCodeNet.Encoding;
using Gma.QrCodeNet.Encoding.Windows.Render;
using Psychokinesis.Interface;
using Psychokinesis.Main.Control;

namespace Psychokinesis
{
    namespace Main
    {
        /// <summary>
        /// WelcomeWindow.xaml 的交互逻辑
        /// </summary>
        [Export(typeof(IPlugin))]
        [ExportMetadata("DisplayName", "欢迎使用")]
        [ExportMetadata("MuiUri", "/Welcome")]
        public partial class WelcomeWindow : UserControl, IPlugin
        {
            public WelcomeWindow()
            {
                InitializeComponent();

                QrEncoder qrEncoder = new QrEncoder(ErrorCorrectionLevel.H);
                QrCode qrCode = qrEncoder.Encode(Messenger.Instance.SerialNumber);
                GraphicsRenderer renderer = new GraphicsRenderer(new FixedModuleSize(5, QuietZoneModules.Two), Brushes.Black, Brushes.White);
                MemoryStream stream = new MemoryStream();
                renderer.WriteToStream(qrCode.Matrix, ImageFormat.Png, stream);
                PngBitmapDecoder decoder = new PngBitmapDecoder(stream, BitmapCreateOptions.PreservePixelFormat, BitmapCacheOption.Default);
                qrCodeImg.Source = decoder.Frames[0];
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
            }

            // 网络不可用
            public void NetworkUnavailable()
            {
            }

            // 其他终端上线
            public void DeviceOnline(Device dev)
            {
            }

            // 其他终端下线
            public void DeviceOffline(Device dev)
            {
            }
        }
    }
    
}