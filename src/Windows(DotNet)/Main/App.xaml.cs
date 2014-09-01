using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Threading.Tasks;
using System.Windows;
using System.IO;
using System.Reflection;
using System.ComponentModel.Composition;           // 构成托管扩展框架 (MEF) 的核心的类，相关特性的定义
using System.ComponentModel.Composition.Hosting;   // 构成托管扩展框架 (MEF) 的核心的类，相关容器、目录的定义
using FirstFloor.ModernUI.Windows;

namespace Psychokinesis
{
    namespace Main
    {
        /// <summary>
        /// App.xaml 的交互逻辑
        /// </summary>
        public partial class App : Application
        {
            public static DirectoryCatalog pluginCatalog;                                                               // 目录索引容器
            public static CompositionContainer pluginContainer;                                                         // 插件容器


            protected override void OnStartup(StartupEventArgs e)
            {
                base.OnStartup(e);

                /*
                 * 程序启动时执行
                 */
                // 开启映像复制，实现动态加载/卸载插件
                AppDomain.CurrentDomain.SetShadowCopyFiles();

                // 插件存储目录不存在则创建
                var pluginPath = Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location) + "\\Plugin";
                if (!Directory.Exists(pluginPath))
                    Directory.CreateDirectory(pluginPath);

                // AggregateCatalog可以组合多个目录一块儿导入
                var catalog = new AggregateCatalog();
                catalog.Catalogs.Add(new AssemblyCatalog(Assembly.GetExecutingAssembly()));      // 导入程序自身导出的接口
                pluginCatalog = new DirectoryCatalog(pluginPath);                                // 导入指定目录里的接口
                catalog.Catalogs.Add(pluginCatalog);

                // 创建一个包含指定目录集合里插件的容器
                pluginContainer = new CompositionContainer(catalog);

                // retrieve the MefContentLoader export and assign to global resources (so {DynamicResource MefContentLoader} can be resolved)
                var pluginLoader = pluginContainer.GetExport<PluginManager>().Value;

                this.Resources.Add("PluginContentLoader", pluginLoader);
            }
        }
    }
}