using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
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
    public class StringObject
    {
        private string str;
        public StringObject(string str) { this.str = str; }
        public string StringValue { get { return str; } set { str = value; } }
        public override string ToString() { return str; }
    }

    /// <summary>
    /// Interaction logic for YukonWindow.xaml
    /// </summary>
    public partial class YukonWindow : UserControl
    {
        private YukonOutput output = new YukonOutput();
        private YukonInput input = new YukonInput();
        private bool loadYukonInput = false;
        private bool rerunYukon = false;

        public YukonWindow()
        {
            InitializeComponent();

            output = new YukonOutput();
            input = new YukonInput();

            // TODO: remove this
            output.UseCaseNotifications.Add(new UseCaseNotification("Create New File", "void FileIO::openFile(string)", "FileIO.cpp", 78, "MainWindow::btnOpen_Click(Object,EventArgs)->FileManager::openFile(string)->FileIO::openFile(string)"));
            output.UseCaseNotifications.Add(new UseCaseNotification("Create New File", "void FileIO::btnOpen_Click(Object,EventArgs)", "MainWindow.cpp", 219, "MainWindow::btnOpen_Click(Object,EventArgs)"));
            //
            /*// TODO: move this elsewhere
            ListBox lst = (ListBox)FindName("lstNotifications");
            foreach (UseCaseNotification not in output.UseCaseNotifications)
                lst.Items.Add(not.CodeChange);
            //*/

            ListBox lst = (ListBox)FindName("lstNotifications");
            lst.DataContext = output.UseCaseNotifications;
        }

        private void tbcYukon_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            string path = input.ProjectDirectory + "yukon/input.xml";
            if (loadYukonInput)
            {
                try
                {
                    input.ReadXML(path);
                }
                catch (Exception)
                {
                }

                PopulateUseCaseList();
                loadYukonInput = false;
            }

            TabItem tbiuc = ((TabItem)FindName("tbiUseCases"));
            if (tbiuc.IsSelected)
                RefreshControlStates();

            TabItem tbislu = ((TabItem)FindName("tbiSinceLastUpdate"));
            if (!tbislu.IsSelected || !rerunYukon)
                return;

            try
            {
                input.WriteXML(path);
            }
            catch (Exception)
            {
                MessageBox.Show("Error writing yukon XML input. Path " + path + " may be write-protected.",
                    "Error", MessageBoxButton.OK, MessageBoxImage.Error);
            }

            /*Process yukon = new Process();
            yukon.StartInfo.FileName = "yukon";
            yukon.StartInfo.Arguments = "yukon/input.xml";
            yukon.StartInfo.UseShellExecute = false;
            yukon.StartInfo.RedirectStandardOutput = true;
            try
            {
                yukon.Start();
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error running yukon command line tool: " + ex.Message, "Error",
                    MessageBoxButton.OK, MessageBoxImage.Error);
                return;
            }

            try
            {
                StreamWriter sw = new StreamWriter("yukon/output.txt");
                sw.Write(yukon.StandardOutput.ReadToEnd());
                sw.Close();
            }
            catch (Exception)
            {
                MessageBox.Show("Error writing yukon output to yukon/output.txt. The path may be write-protected.",
                    "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                return;
            }
            try
            {
                output.ReadOutput("yukon/output.txt");
            }
            catch (Exception)
            {
                MessageBox.Show("Error reading yukon output from yukon/output.txt.",
                    "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                return;
            }

            rerunYukon = false;*/
        }

        private void lstNotifications_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            ListBox lst = (ListBox)sender;
            TreeView trv = (TreeView)FindName("trvCallGraphPath");
            if (lst.SelectedIndex < 0)
                return;

            // Display corresponding call graph path
            trv.Items.Clear();
            string[] cgpath = output.UseCaseNotifications[lst.SelectedIndex].CallGraphPath;
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

        private void lstUseCases_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            ListBox lst1 = (ListBox)FindName("lstUseCases");
            if (lst1.SelectedIndex < 0)
                return;

            ListBox lst2 = (ListBox)FindName("lstUseCaseMethods");
            UseCaseDefinition ucd = input.UseCaseDefinitions[lst1.SelectedIndex];
            ((TextBox)FindName("txtUseCaseName")).Text = ucd.Name;
            lst2.Items.Clear();
            foreach (string method in ucd.Methods)
                lst2.Items.Add(new StringObject(method));

            RefreshControlStates();
        }

        private void btnCreateNew_Click(object sender, RoutedEventArgs e)
        {
            ListBox lst = (ListBox)FindName("lstUseCases");
            input.UseCaseDefinitions.Add(new UseCaseDefinition("MyUseCase"));
            lst.Items.Add(new StringObject("MyUseCase"));
            lst.SelectedIndex = lst.Items.Count-1;

            rerunYukon = true;
            RefreshControlStates();
        }

        private void btnDelete_Click(object sender, RoutedEventArgs e)
        {
            ListBox lst = (ListBox)FindName("lstUseCases");
            if (lst.SelectedIndex < 0)
                return;

            input.UseCaseDefinitions.RemoveAt(lst.SelectedIndex);
            lst.Items.RemoveAt(lst.SelectedIndex);
            lst.SelectedIndex = -1;

            rerunYukon = true;
            RefreshControlStates();
        }

        private void txtUseCaseName_TextChanged(object sender, TextChangedEventArgs e)
        {
            ListBox lst = (ListBox)FindName("lstUseCases");
            if (lst.SelectedIndex < 0)
                return;

            string new_name = ((TextBox)e.Source).Text;
            input.UseCaseDefinitions[lst.SelectedIndex].Name = new_name;
            ((StringObject)lst.Items[lst.SelectedIndex]).StringValue = new_name;
            lst.Items.Refresh();

            rerunYukon = true;
        }

        private void lstUseCaseMethods_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            ListBox lst1 = (ListBox)FindName("lstUseCases");
            if (lst1.SelectedIndex < 0)
                return;

            ListBox lst2 = (ListBox)FindName("lstUseCaseMethods");
            if (lst2.SelectedIndex < 0)
                return;

            UseCaseDefinition ucd = input.UseCaseDefinitions[lst1.SelectedIndex];
            ((TextBox)FindName("txtMethodSig")).Text = ucd.Methods[lst2.SelectedIndex];
            
            RefreshControlStates();
        }

        private void btnAddMethod_Click(object sender, RoutedEventArgs e)
        {
            ListBox lst1 = (ListBox)FindName("lstUseCases");
            if (lst1.SelectedIndex < 0)
                return;

            ListBox lst2 = (ListBox)FindName("lstUseCaseMethods");
            input.UseCaseDefinitions[lst1.SelectedIndex].Methods.Add("void myMethod(T)");
            lst2.Items.Add(new StringObject("void myMethod(T)"));
            lst2.SelectedIndex = lst2.Items.Count-1;

            rerunYukon = true;
            RefreshControlStates();
        }

        private void btnRemoveMethod_Click(object sender, RoutedEventArgs e)
        {
            ListBox lst1 = (ListBox)FindName("lstUseCases");
            if (lst1.SelectedIndex < 0)
                return;

            ListBox lst2 = (ListBox)FindName("lstUseCaseMethods");
            if (lst2.SelectedIndex < 0)
                return;

            input.UseCaseDefinitions[lst1.SelectedIndex].Methods.RemoveAt(lst2.SelectedIndex);
            lst2.Items.RemoveAt(lst2.SelectedIndex);
            lst2.SelectedIndex = -1;

            rerunYukon = true;
            RefreshControlStates();
        }

        private void txtMethodSig_TextChanged(object sender, TextChangedEventArgs e)
        {
            ListBox lst1 = (ListBox)FindName("lstUseCases");
            if (lst1.SelectedIndex < 0)
                return;

            ListBox lst2 = (ListBox)FindName("lstUseCaseMethods");
            if (lst2.SelectedIndex < 0)
                return;

            string new_sig = ((TextBox)e.Source).Text;
            input.UseCaseDefinitions[lst1.SelectedIndex].Methods[lst2.SelectedIndex] = new_sig;
            ((StringObject)lst2.Items[lst2.SelectedIndex]).StringValue = new_sig;
            lst2.Items.Refresh();

            rerunYukon = true;
        }

        private void txtProjDir_TextChanged(object sender, TextChangedEventArgs e)
        {
            input.ProjectDirectory = ((TextBox)e.Source).Text;
            char lc = input.ProjectDirectory[input.ProjectDirectory.Length-1];
            if (lc != '\\' && lc != '/')
                input.ProjectDirectory += "/";

            loadYukonInput = true;
            RefreshControlStates();
        }

        private void PopulateUseCaseList()
        {
            ListBox lst = (ListBox)FindName("lstUseCases");
            foreach (UseCaseDefinition ucd in input.UseCaseDefinitions)
                lst.Items.Add(new StringObject(ucd.Name));
        }

        private void RefreshControlStates()
        {
            ListBox lst1 = (ListBox)FindName("lstUseCases");
            ListBox lst2 = (ListBox)FindName("lstUseCaseMethods");
            if (lst1.SelectedIndex < 0)
            {
                ((Button)FindName("btnDelete")).IsEnabled = false;
                ((TextBox)FindName("txtUseCaseName")).IsEnabled = false;
                ((Button)FindName("btnAddMethod")).IsEnabled = false;
                lst2.SelectedIndex = -1;
                lst2.IsEnabled = false;
            }
            else
            {
                ((Button)FindName("btnDelete")).IsEnabled = true;
                ((TextBox)FindName("txtUseCaseName")).IsEnabled = true;
                ((Button)FindName("btnAddMethod")).IsEnabled = true;

                lst2.IsEnabled = true;
                if (lst2.SelectedIndex >= lst2.Items.Count )
                    lst2.SelectedIndex = -1;
            }

            if (lst2.SelectedIndex < 0)
            {
                ((TextBox)FindName("txtMethodSig")).IsEnabled = false;
                ((Button)FindName("btnRemoveMethod")).IsEnabled = false;
            }
            else
            {
                ((TextBox)FindName("txtMethodSig")).IsEnabled = true;
                ((Button)FindName("btnRemoveMethod")).IsEnabled = true;
            }
        }
    }
}
