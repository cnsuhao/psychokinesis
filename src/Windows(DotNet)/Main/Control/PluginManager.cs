/* ==============================================================================
 * 作者：张扬
 * 创建时间：2014/9/1 22:18:09
 * 简介：管理插件
 * LoadContent方法会在插件界面需要显示时导入插件。
 * ==============================================================================*/
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.ComponentModel.Composition;
using System.ComponentModel.Composition.Hosting;
using FirstFloor.ModernUI.Windows;
using Psychokinesis.Interface;

namespace Psychokinesis
{
    namespace Main.Control
    {
        [Export]
        public class PluginManager
            : DefaultContentLoader
        {
            [ImportMany(AllowRecomposition = true)]
            private Lazy<IPlugin, IPluginMetadata>[] plugins { get; set; }           // 只可在界面线程中访问插件的接口

            protected override object LoadContent(Uri uri)
            {
                // lookup the content based on the content uri in the content metadata
                var content = (from c in plugins
                               where c.Metadata.MuiUri == uri.OriginalString
                               select c.Value).FirstOrDefault();

                if (content == null)
                {
                    throw new ArgumentException("Invalid source uri: " + uri);
                }

                return content;
            }

            public void NetworkAvailable(ICommunication cc)
            {
                foreach (var p in plugins)
                {
                    p.Value.NetworkAvailable(cc);
                }
            }

            public void NetworkUnavailable()
            {
                foreach (var p in plugins)
                {
                    p.Value.NetworkUnavailable();
                }
            }

            public void DeviceOnline(Device d)
            {
                foreach (var p in plugins)
                {
                    p.Value.DeviceOnline(d);
                }
            }

            public void DeviceOffline(Device d)
            {
                foreach (var p in plugins)
                {
                    p.Value.DeviceOffline(d);
                }
            }
        }
    } 
}