/* ==============================================================================
 * 作者：张扬
 * 创建时间：2014/9/7 21:55:19
 * 简介：硬件基本信息
 * double-check locking实现的单例模式，支持线程安全的延迟初始化。
 * ==============================================================================*/
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Management;


namespace Psychokinesis.Main.Util
{
    class SystemInfo
    {
        public readonly List<string> ProcessorIds = new List<string>();
        public readonly List<string> PhysicalMediaSerialNumbers = new List<string>();
        public readonly List<string> NetworkAdapterMacs = new List<string>();

        private SystemInfo() 
        {
            try
            {
                ManagementClass mc = new ManagementClass("Win32_Processor");
                ManagementObjectCollection moc = mc.GetInstances();
                foreach (ManagementObject mo in moc)
                {
                    ProcessorIds.Add(mo.Properties["ProcessorId"].Value.ToString());
                }
            }
            catch
            { }

            try
            {
                ManagementClass mc = new ManagementClass("Win32_PhysicalMedia");
                ManagementObjectCollection moc = mc.GetInstances();
                foreach (ManagementObject mo in moc)
                {
                    PhysicalMediaSerialNumbers.Add(mo.Properties["SerialNumber"].Value.ToString());
                }
            }
            catch
            { }

            try
            {
                ManagementClass mc = new ManagementClass("Win32_NetworkAdapterConfiguration");
                ManagementObjectCollection moc = mc.GetInstances();
                foreach (ManagementObject mo in moc)
                {
                    if ((bool)mo["IPEnabled"] == true)
                        NetworkAdapterMacs.Add(mo["MacAddress"].ToString());
                }
            }
            catch
            { }
        }


        // 实现单例模式
        private static volatile SystemInfo instance;
        private static object syncRoot = new Object();
        public static SystemInfo Instance
        {
            get
            {
                if (instance == null)
                {
                    lock (syncRoot)
                    {
                        if (instance == null)
                            instance = new SystemInfo();
                    }
                }
                return instance;
            }
        } 
    }
}
