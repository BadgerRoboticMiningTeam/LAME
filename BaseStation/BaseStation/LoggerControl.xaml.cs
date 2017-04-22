using System;
using System.Collections.Generic;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Media;

namespace BaseStation
{
    /// <summary>
    /// Interaction logic for LoggerControl.xaml
    /// </summary>
    public partial class LoggerControl : UserControl
    {
        Logger log;
        Paragraph paragraph;

        static readonly IDictionary<LoggerLevel, Brush> levelColorMap = new Dictionary<LoggerLevel, Brush> {
            { LoggerLevel.Info, Brushes.Blue },
            { LoggerLevel.Warning, Brushes.Yellow },
            { LoggerLevel.Error, Brushes.Red }
        };

        public LoggerControl()
        {
            InitializeComponent();

            paragraph = new Paragraph();
            LoggerTextBox.Document = new FlowDocument(paragraph);

            log = Logger.GetInstance(DateTime.Now.ToString("MMddyyyy_HHmmss") + ".log");
            log.LogUpdated += LoggerUpdated;
            DataContext = log;

            log.Write(LoggerLevel.Info, "Base Station GUI Logger initialized.");
        }

        void LoggerUpdated(LogItem item)
        {
            Dispatcher.BeginInvoke(new Action(() =>
            {
                // [{time} | {level}] {data}\n
                TextRange tr1 = new TextRange(LoggerTextBox.Document.ContentEnd, LoggerTextBox.Document.ContentEnd);
                tr1.Text = "[" + item.Timestamp + " | ";
                tr1.ApplyPropertyValue(TextElement.ForegroundProperty, Brushes.Black);

                TextRange tr2 = new TextRange(LoggerTextBox.Document.ContentEnd, LoggerTextBox.Document.ContentEnd);
                tr2.Text = Logger.levelMap[item.Level];
                tr2.ApplyPropertyValue(TextElement.ForegroundProperty, levelColorMap[item.Level]);

                TextRange tr3 = new TextRange(LoggerTextBox.Document.ContentEnd, LoggerTextBox.Document.ContentEnd);
                tr3.Text = "] " + item.Text + "\r";
                tr3.ApplyPropertyValue(TextElement.ForegroundProperty, Brushes.Black);

                LoggerTextBox.ScrollToEnd();
            }));
        }
    }
}
