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
using System.Windows.Media.Animation;
using System.Threading;
using System.Windows.Threading;

namespace Psychokinesis.Main
{
    /// <summary>
    /// SplashWindow.xaml 的交互逻辑
    /// </summary>
    public partial class SplashWindow : Window
    {
        // 声明委托
        delegate void ShowDelegate(string txt);
        delegate void HideDelegate();
        delegate void ErrorExitDelegate(string txt);

        // 程序初始化线程
        Thread initializingThread;

        // 用于在线程中更新界面的委托
        ShowDelegate showDelegate;
        HideDelegate hideDelegate;
        ErrorExitDelegate errorExitDelegate;


        public SplashWindow()
        {
            InitializeComponent();

            showDelegate = new ShowDelegate(this.ShowPrompt);
            hideDelegate = new HideDelegate(this.HidePrompt);
            errorExitDelegate = new ErrorExitDelegate(this.ErrorExit);
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            initializingThread = new Thread(Initializing);
            initializingThread.Start();
        }


        private void Initializing()
        {
            this.Dispatcher.Invoke(showDelegate, "连接网络......");
            Thread.Sleep(1500);                    // 等待提示信息显示动画

            if (Connecting() == false)
                this.Dispatcher.Invoke(errorExitDelegate, "无法连接服务器，请稍后再试！");

            this.Dispatcher.Invoke(hideDelegate);
            Thread.Sleep(500);                     // 等待提示信息消失动画

            // 初始化结束，关闭窗口
            this.Dispatcher.Invoke(DispatcherPriority.Normal, (Action)delegate() { Close(); });
        }

        private bool Connecting()
        {
            return false;
        }


        private void ShowPrompt(string txt)
        {
            txtLoading.Text = txt;

            Storyboard Showboard = this.Resources["ShowStoryBoard"] as Storyboard;
            Showboard.Begin();
        }

        private void ShowStoryBoard_Completed(object sender, EventArgs e)
        {
            Storyboard board = this.Resources["FadeStoryBoard"] as Storyboard;
            board.Begin(this, true);
        }

        private void HidePrompt()
        {
            Storyboard board = this.Resources["FadeStoryBoard"] as Storyboard;
            board.Stop();

            Storyboard Hideboard = this.Resources["HideStoryBoard"] as Storyboard;
            Hideboard.Begin();
        }

        private void ErrorExit(string err)
        {
            MessageBox.Show(err, "错误", MessageBoxButton.OK, MessageBoxImage.Error);
            Application.Current.Shutdown();
        }
    }
}
