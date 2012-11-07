using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace YukonToolControls
{
    /// <summary>
    /// Interaction logic for YukonWindow.xaml
    /// </summary>
    public partial class YukonWindow : UserControl
    {
        private YukonNotificationList notificationList = new YukonNotificationList();

        public YukonWindow()
        {
            InitializeComponent();

            // TODO: remove this
            notificationList.Add(new YukonNotification("Use case 'Create New File' may be affected by change in method FileIO::openFile()", "MainWindow::btnOpen_Click()->FileManager::openFile()->FileIO::openFile()"));
            notificationList.Add(new YukonNotification("Use case 'Save Current File' may be affected by change in method FileIO::openFile()", "MainWindow::btnSave_Click()->FileManager::saveFile()->FileIO::openFile()"));
            notificationList.Add(new YukonNotification("Use case 'Exit Application' may be affected by change in method MainWindow::btnExit_Click()", "MainWindow::btnExit_Click()"));
            notificationList.Add(new YukonNotification("Use case 'Exit Application' may be affected by change in method FileIO::openFile()", "MainWindow::btnExit_Click()->FileManager::saveFile()->FileIO::openFile()"));
            notificationList.Add(new YukonNotification("ERROR: Use case 'Save All Files' is missing method MainWindow::btnSaveAll_Click()", ""));
            //
            // TODO: move this elsewhere
            ListBox lst = (ListBox)FindName("lstNotifications");
            foreach (YukonNotification not in notificationList)
                lst.Items.Add(not.CodeChange);
            //
        }

        private void lstNotifications_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            ListBox lst = (ListBox)sender;
            TreeView trv = (TreeView)FindName("trvCallGraphPath");

            if (lst.SelectedIndex < 0)
                return;

            trv.Items.Clear();
            string[] cgpath = notificationList[lst.SelectedIndex].CallGraphPath;
            TreeViewItem lcn = null;
            for (int cgni = 0; cgni < cgpath.Length; ++cgni)
            {
                TreeViewItem trvitem = new TreeViewItem();
                trvitem.Header = cgpath[cgni];
                trvitem.IsExpanded = true;
                
                if (cgni == 0)
                    trv.Items.Add(trvitem);
                else
                    lcn.Items.Add(trvitem);

                lcn = trvitem;
            }
        }

        private void trvCallGraphPath_SelectedItemChanged(object sender, RoutedPropertyChangedEventArgs<object> e)
        {

        }
    }
}
