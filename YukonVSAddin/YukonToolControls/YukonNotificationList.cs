using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;

namespace YukonToolControls
{
    public class YukonNotificationList : ObservableCollection<YukonNotification>
    {
    }

    public class YukonNotification
        {
            private string codeChange;
            private string callGraphPath;

            public YukonNotification(string codeChange, string callGraphPath)
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
