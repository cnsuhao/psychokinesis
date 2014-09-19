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
using Psychokinesis.Main.Control;
using Psychokinesis.Interface;

namespace Psychokinesis.Main
{
    /// <summary>
    /// SplashWindow.xaml 的交互逻辑
    /// </summary>
    public partial class SplashWindow : Window, IMessage
    {
        // 声明委托
        delegate void ShowDelegate(string txt);
        delegate void HideDelegate();
        delegate void ErrorExitDelegate(string txt);

        // 用于在线程中更新界面的委托
        ShowDelegate showDelegate;
        HideDelegate hideDelegate;
        ErrorExitDelegate errorExitDelegate;

        IDisposable messagerObserver;

        static Dictionary<MessengerState, String> stateMsg = new Dictionary<MessengerState, string>()
        {
            {MessengerState.Active, "获取登录信息......"},
            {MessengerState.GetLoginInformation, "登录中......"}
        };
        static Dictionary<MessengerState, String> stateErrorMsg = new Dictionary<MessengerState, string>()
        {
            {MessengerState.Active, "无法从服务器获取登录信息，请稍后再试！"},       // 错误是发生在当前状态转到下一个状态的过程中
            {MessengerState.GetLoginInformation, "登录失败，请稍后再试！"},
            {MessengerState.ConnectionError, "登录失败，请稍后再试！"}
        };

        public SplashWindow()
        {
            InitializeComponent();

            showDelegate = new ShowDelegate(this.ShowPrompt);
            hideDelegate = new HideDelegate(this.HidePrompt);
            errorExitDelegate = new ErrorExitDelegate(this.ErrorExit);
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            messagerObserver = Messenger.Instance.Subscribe(this);
            Messenger.Instance.Login();
        }

        private void Window_Unloaded(object sender, RoutedEventArgs e)
        {
            messagerObserver.Dispose();
        }


        public virtual void OnCompleted()
        {
            MessengerState state = Messenger.Instance.State;

            if (state > MessengerState.Active)
            {
                this.Dispatcher.Invoke(hideDelegate);
                Thread.Sleep(500);                    // 等待提示信息消失动画
            }

            if (state == MessengerState.Online)
                this.Dispatcher.Invoke(DispatcherPriority.Normal, (Action)delegate() { Close(); });
            else
            {
                this.Dispatcher.Invoke(showDelegate, stateMsg[Messenger.Instance.State]);
                Thread.Sleep(1500);                    // 等待提示信息显示动画
            }
        }

        public virtual void OnError(Exception e)
        {
            this.Dispatcher.BeginInvoke(errorExitDelegate, stateErrorMsg[Messenger.Instance.State]);
        }

        public virtual void OnNext(Message value)
        {}

        public virtual String GetIdentification()
        {
            return "";       // 此窗口不会接收任何消息
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
            Messenger.Instance.Logout();
            MessageBox.Show(err, "错误", MessageBoxButton.OK, MessageBoxImage.Error);
            Application.Current.Shutdown();
        }
    }
}
