using System;
using System.Collections.Generic;
using System.IO;
using System.Text;

namespace BaseStation
{
    enum LoggerLevel
    {
        Info,
        Warning,
        Error
    }

    class Logger : IDisposable
    {
        static Logger instance;
        static readonly Dictionary<LoggerLevel, string> levelMap = new Dictionary<LoggerLevel, string> {
            { LoggerLevel.Info, "INFO" },
            { LoggerLevel.Warning, "WARN" },
            { LoggerLevel.Error, "ERROR" }
        };

        StreamWriter writer;
        StringBuilder text;

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

            if (filePath == null)
                writer = new StreamWriter(filePath);
        }

        public void Write(LoggerLevel level, string log_text)
        {
            string converted_text = string.Format("[{0} | {1}] {2}\n", DateTime.Now.ToString("HH:mm:ss") , levelMap[level], log_text);
            text.Append(converted_text);

            if (writer != null)
            {
                writer.Write(converted_text);
                writer.Flush();
            }
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
