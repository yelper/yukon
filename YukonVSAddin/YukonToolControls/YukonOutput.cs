using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;

namespace YukonToolControls
{
    public class YukonOutput
    {
        private ObservableCollection<UseCaseNotification> useCaseNotifs;

        public YukonOutput()
        {
            useCaseNotifs = new ObservableCollection<UseCaseNotification>();
        }

        public ObservableCollection<UseCaseNotification> UseCaseNotifications
        {
            get
            {
                return useCaseNotifs;
            }
        }

        public void ReadOutput(string path)
        {
        }
    }

    public class UseCaseNotification
    {
        private string useCase;
        private string method;
        private string codeFile;
        private int codeLine;
        private string callGraphPath;

        public UseCaseNotification(string useCase, string method,
            string codeFile, int codeLine, string callGraphPath)
        {
            this.useCase = useCase;
            this.method = method;
            this.codeFile = codeFile;
            this.codeLine = codeLine;
            this.callGraphPath = callGraphPath;
        }

        public string UseCase
        {
            get { return useCase; }
        }

        public string Method
        {
            get { return method; }
        }

        public string CodeFile
        {
            get { return codeFile; }
        }

        public int CodeLine
        {
            get { return codeLine; }
        }

        public string[] CallGraphPath
        {
            get
            {
                string[] sep = { "->" };
                return callGraphPath.Split(sep, StringSplitOptions.RemoveEmptyEntries);
            }
        }

        public override string ToString()
        {
            return "Use case '" + useCase + "' may be affected by change in method " + method +
                " in " + codeFile + ", " + codeLine + ".";
        }
    }
}
