using System;
using Newtonsoft.Json;
using Gtk;
using System.Collections.Generic;
using System.IO;

namespace SpritesheetManager
{
    public static class SaveData
    {
        public static List<MetaData> Coredata = new List<MetaData>();
        public static void LoadData()
        {
            FileChooserDialog fileDialog = new FileChooserDialog(
                "Choose a file",
                MainClass.win,
                FileChooserAction.Open,
                "Cancel", ResponseType.Cancel,
                "Open", ResponseType.Accept
            );

            FileFilter filter = new FileFilter();
            filter.Name = "meta image data";
            filter.AddPattern("*.meta");
            fileDialog.AddFilter(filter);

            if (fileDialog.Run() == (int)ResponseType.Accept)
            {
                Coredata = JsonConvert.DeserializeObject<List<MetaData>>(File.ReadAllText(fileDialog.Filename));
            }

            fileDialog.Destroy();

            MainWindow.highesint = Coredata.Count;

            return;
        }

        public static void SaveDataNow()
        {
            FileChooserDialog fileDialog = new FileChooserDialog(
                "Choose a file",
                MainClass.win,
                FileChooserAction.Save,
                "Cancel", ResponseType.Cancel,
                "Save", ResponseType.Accept
            );
                
            FileFilter filter = new FileFilter();
            filter.Name = "meta image data";
            filter.AddPattern("*.meta");
            fileDialog.AddFilter(filter);

            if (fileDialog.Run() == (int)ResponseType.Accept)
            {
                if (!fileDialog.Filename.Contains(".meta"))
                {
                    File.WriteAllText(fileDialog.Filename, JsonConvert.SerializeObject(Coredata, Formatting.Indented));
                }
                else
                {
                    File.WriteAllText(fileDialog.Filename + ".meta", JsonConvert.SerializeObject(Coredata, Formatting.Indented));
                }
            }

            fileDialog.Destroy();
            return;
        }
    }
}
