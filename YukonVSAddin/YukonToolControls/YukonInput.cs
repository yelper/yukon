using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Xml;

namespace YukonToolControls
{
    public class YukonInput
    {
        private string projDir;
        private List<UseCaseDefinition> useCaseDefs;

        public YukonInput()
        {
            projDir = "";
            useCaseDefs = new List<UseCaseDefinition>();
        }

        public string ProjectDirectory
        {
            get
            {
                return projDir;
            }
            set
            {
                projDir = value;
            }
        }

        public List<UseCaseDefinition> UseCaseDefinitions
        {
            get
            {
                return useCaseDefs;
            }
        }

        public void ReadXML(string path)
        {
            XmlReader xmlr = new XmlTextReader(path);
            xmlr.ReadStartElement("yukonConfig");
            projDir = xmlr.ReadElementString();
            if (!xmlr.IsEmptyElement)
            {
                xmlr.ReadStartElement("useCases");
                while (xmlr.IsStartElement())
                {
                    xmlr.ReadStartElement("useCase");
                    UseCaseDefinition ucd = new UseCaseDefinition(xmlr.ReadElementString());
                    if (!xmlr.IsEmptyElement)
                    {
                        xmlr.ReadStartElement("methods");
                        while (xmlr.IsStartElement())
                            ucd.Methods.Add(xmlr.ReadElementString());
                        xmlr.ReadEndElement();
                    }
                    else
                        xmlr.Read();
                    useCaseDefs.Add(ucd);
                    xmlr.ReadEndElement();
                }
                xmlr.ReadEndElement();
            }
            else
                xmlr.Read();
            xmlr.ReadEndElement();
            xmlr.Close();
        }

        public void WriteXML(string path)
        {
            XmlTextWriter xmlw = new XmlTextWriter(path, Encoding.Unicode);
            xmlw.Indentation = 2;
            xmlw.Formatting = Formatting.Indented;
            xmlw.WriteStartDocument();
            xmlw.WriteStartElement("yukonConfig");
            xmlw.WriteElementString("projectDirectory", projDir);
            xmlw.WriteStartElement("useCases");
            foreach (UseCaseDefinition ucd in useCaseDefs)
            {
                xmlw.WriteStartElement("useCase");
                xmlw.WriteElementString("name", ucd.Name);
                xmlw.WriteStartElement("methods");
                foreach (string method in ucd.Methods)
                    xmlw.WriteElementString("method", method);
                xmlw.WriteEndElement();
                xmlw.WriteEndElement();
            }
            xmlw.WriteEndElement();
            xmlw.WriteEndElement();
            xmlw.WriteEndDocument();
            xmlw.Close();
        }
    }

    public class UseCaseDefinition
    {
        private string name;
        private List<string> methods;

        public UseCaseDefinition(string name)
        {
            this.name = name;
            this.methods = new List<string>();
        }

        public string Name
        {
            get
            {
                return name;
            }
            set
            {
                name = value;
            }
        }

        public List<string> Methods
        {
            get
            {
                return methods;
            }
        }

        public override string ToString()
        {
            return name;
        }
    }
}
