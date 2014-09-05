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

namespace Psychokinesis
{
    namespace Main
    {
        /// <summary>
        /// MainWindow.xaml 的交互逻辑
        /// </summary>
        public partial class MainWindow : ModernWindow
        {
            public MainWindow()
            {
                InitializeComponent();

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
        }
    } 
}
