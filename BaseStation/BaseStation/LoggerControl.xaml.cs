using System;
using System.Collections.Generic;
using System.IO;
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
    /// Interaction logic for Logger.xaml
    /// </summary>
    public partial class LoggerControl : UserControl
    {
        private Logger log;

        public LoggerControl()
        {
            InitializeComponent();
            log = Logger.GetInstance(DateTime.Now.ToString("MMddyyyy_HHmmss") + ".log");
            DataContext = log;

            log.Write(LoggerLevel.Info, "Base Station GUI Logger initialized.");
        }
    }
}
