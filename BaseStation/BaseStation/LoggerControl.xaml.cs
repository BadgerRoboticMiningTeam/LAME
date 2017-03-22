using System;
using System.Collections.Generic;
using System.ComponentModel;
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

namespace BaseStation
{
    /// <summary>
    /// Interaction logic for LoggerControl.xaml
    /// </summary>
    public partial class LoggerControl : UserControl
    {
        private Logger log;

        public LoggerControl()
        {
            InitializeComponent();
            log = Logger.GetInstance(DateTime.Now.ToString("MMddyyyy_HHmmss") + ".log");
            log.PropertyChanged += LoggerUpdated;
            DataContext = log;

            log.Write(LoggerLevel.Info, "Base Station GUI Logger initialized.");
        }

        void LoggerUpdated(object sender, PropertyChangedEventArgs e)
        {
            Dispatcher.BeginInvoke(new Action(() =>
            {
                // not very efficient, but it'll do
                LoggerTextBox.Text = log.Text;
                LoggerTextBox.Focus();
                LoggerTextBox.CaretIndex = LoggerTextBox.Text.Length;
                LoggerTextBox.ScrollToEnd();
            }));
        }
    }
}
