using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Runtime.CompilerServices;
using System.Text;

namespace BaseStation
{
    enum LoggerLevel : int
    {
        Info,
        Warning,
        Error
    }

    struct LogItem
    {
        public LoggerLevel Level;
        public string Timestamp;
        public string Text;
    }

    class Logger : IDisposable
    {
        static Logger instance;
        public static readonly IDictionary<LoggerLevel, string> levelMap = new Dictionary<LoggerLevel, string> {
            { LoggerLevel.Info, "INFO" },
            { LoggerLevel.Warning, "WARN" },
            { LoggerLevel.Error, "ERROR" }
        };

        StreamWriter writer;
        StringBuilder text;

        public delegate void UpdateLoggerEventHandler(LogItem item);
        public event UpdateLoggerEventHandler LogUpdated;

        /// <summary>
        /// Gets or creates a Logger instance.
        /// </summary>
        /// <param name="filePath">The log file path, or null for an in-memory log.</param>
        /// <returns>The created or existing Logger instance.</returns>
        public static Logger GetInstance(string filePath = null)
        {
            if (instance == null)
                instance = new Logger(filePath);

            return instance;
        }

        private Logger(string filePath)
        {
            text = new StringBuilder();

            if (filePath != null)
                writer = new StreamWriter(filePath);
        }

        [MethodImpl(MethodImplOptions.Synchronized)]
        public void Write(LoggerLevel level, string log_text)
        {
            var currentTime = DateTime.Now.ToString("HH:mm:ss");
            string converted_text = string.Format("[{0} | {1}] {2}\n", DateTime.Now.ToString("HH:mm:ss"), levelMap[level], log_text);
            text.Append(converted_text);

            if (writer != null)
            {
                writer.Write(converted_text);
                writer.Flush();
            }

            var item = new LogItem();
            item.Level = level;
            item.Text = log_text;
            item.Timestamp = currentTime;

            LogUpdated?.Invoke(item);
        }

        public string Text
        {
            get { return text.ToString(); }
        }

        #region IDisposable Support
        private bool disposedValue = false; // To detect redundant calls

        protected virtual void Dispose(bool disposing)
        {
            if (!disposedValue)
            {
                if (disposing && writer != null)
                {
                    writer.Flush();
                    writer.Dispose();
                }

                disposedValue = true;
            }
        }

        public void Dispose()
        {
            Dispose(true);
        }
        #endregion
    }
}
