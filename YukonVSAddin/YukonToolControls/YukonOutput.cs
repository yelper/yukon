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
        private string codeChange;
        private string callGraphPath;

        public UseCaseNotification(string codeChange, string callGraphPath)
        {
            this.codeChange = codeChange;
            this.callGraphPath = callGraphPath;
        }

        public string CodeChange
        {
            get { return codeChange; }
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
            return codeChange + "\n" + callGraphPath;
        }
    }
}
