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
    namespace Main
    {
        [Export]
        class PluginManager
            : DefaultContentLoader
        {
            [ImportMany(AllowRecomposition = true)]
            private Lazy<IPlugin, IPluginMetadata>[] plugins { get; set; }

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
        }
    } 
}